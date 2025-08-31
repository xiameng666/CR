#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <vector>
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/uio.h>

#define FAILERD -1
#define MAX_LINE_LEN 256  // /proc/[PID]/maps 每行长度不超过256字节
#define MAX_PATH_LEN 256  // 模块路径最大长度

int GetModuleBase(pid_t pid, const char* module_path, uint64_t* base_addr);

using Args = std::vector<uint64_t>;

uint64_t RemoteCall(int nPid, void* pfnAPI, const Args& args)
{
    //0. 保存寄存器环境
    user_regs_struct regsSaved = {0};
    iovec vec = {&regsSaved, sizeof(regsSaved)};
    int nRet = ptrace(PTRACE_GETREGSET, nPid, 1, &vec);
    if(nRet < 0)
    {
        perror("[ERR] \tRemoteCall 寄存器获取失败");
        return FAILERD;
    }
    printf("[INFO] \tRemoteCall 0. 保存寄存器环境 \n");

    // 1. 修改寄存器
    //     * `pc`改为API的地址
    //     * `X30`改为0
    //     * `X0` ~ `X7` 填入合适参数
    user_regs_struct regsCall = regsSaved;
    regsCall.pc = (uint64_t)pfnAPI;
    regsCall.regs[30] = 0;
    for (size_t i = 0; i < args.size(); i++)
    {
        regsCall.regs[i] = args[i];
    }
    vec.iov_base = &regsCall;
    vec.iov_len = sizeof(regsCall);
    nRet = ptrace(PTRACE_SETREGSET, nPid, 1, &vec);
    if(nRet < 0)
    {
        perror("[ERR] \tRemoteCall 寄存器修改失败");
        return FAILERD;
    }
    printf("[INFO] \tRemoteCall 1. 修改寄存器成功 \n");

    // 2. 恢复目标进程运行
    nRet = ptrace(PTRACE_CONT, nPid, nullptr, nullptr);
    if(nRet < 0)
    {
        perror("[ERR] \tRemoteCall 恢复目标进程运行失败");
        return FAILERD;
    }
    printf("[INFO] \tRemoteCall 2. 恢复目标进程运行 \n");

    // 3. 等待信号（异常）
    int nStatus = 0;
    nRet = waitpid(nPid, &nStatus,  WUNTRACED);
    printf("[INFO] \tRemoteCall 3. 等待信号 ret:%d  sig:%d\n",nRet, WTERMSIG(nStatus));

    // 4. 从`X0`获取返回API的结果
    user_regs_struct regsReult = {0};
    vec.iov_base = &regsReult;
    vec.iov_len = sizeof(regsReult);
    nRet = ptrace(PTRACE_GETREGSET, nPid, 1, &vec);
    if(nRet < 0)
    {
        perror("[ERR] \tRemoteCall 寄存器获取失败");
        return FAILERD;
    }
    printf("[INFO] \tRemoteCall 4. 从`X0`获取返回API的结果成功 0x%lx 0x%lx\n", 
        regsReult.regs[0], 
        regsReult.pc);

    // 5. 恢复寄存器环境
    vec.iov_base = &regsSaved;
    vec.iov_len = sizeof(regsSaved);
    nRet = ptrace(PTRACE_SETREGSET, nPid, 1, &vec);
    if(nRet < 0)
    {
        perror("[ERR] \tRemoteCall 寄存器恢复失败");
        return FAILERD;
    }
    printf("[INFO] \tRemoteCall 5. 恢复寄存器环境 \n");

    return regsReult.regs[0];
}



int main(int argc, char** argv)
{
    //获取PID
    if (argc < 2)
    {
        printf("[ERR] 参数过少 用法:inject <pid>");
        return 0;
    }
    int nPid = atoi(argv[1]);
    printf("[INF] 获取pid %d\n", nPid);

    //附加
    int nRet = ptrace(PTRACE_ATTACH, nPid, 0, 0);
    if (nRet < 0)
    {
        perror("[ERR] 附加失败");
        return 0;
    }

    waitpid(nPid, nullptr,  WUNTRACED);

    //获取libc在目标进程中的基址
    uint64_t nLibcBase = 0;
    if(GetModuleBase(nPid, "libc.so", &nLibcBase) < 0)
    {
        printf("[ERR] 获取libc模块基址失败 \n");
        return 0;
    }

    //调用mmap
    Dl_info di = {0};
    nRet = dladdr((void*)mmap, &di);
    if (nRet <= 0)
    {
        printf("[ERR] 无法获取mmap的地址 %s \n", dlerror());
        return 0;
    }
    void* pfnMmap = (void*)(nLibcBase + ((char*)di.dli_saddr - (char*)di.dli_fbase));
    uint64_t nRemoteAddr = RemoteCall(
        nPid, 
        pfnMmap, 
        {0, 0x1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, (uint64_t)-1, 0}) ;
    if(nRemoteAddr == FAILERD)
    {
        printf("[ERR] 调用mmap 失败\n");
        return 0;
    }
    printf("[INFO] 调用mmap成功 结果:%lx\n", nRemoteAddr);

    //3. 远程写入so的路径
    char szSoPath[] = {"libtest.so"};
    iovec vecLocal = {szSoPath, sizeof(szSoPath)};
    iovec vecRemote = {(void*)nRemoteAddr, sizeof(szSoPath)};
    nRet = process_vm_writev(nPid, &vecLocal, 1, &vecRemote, 1, 0);
    if (nRet < 0)
    {
        perror("[ERR] 写入SO路径失败");
        return 0;
    }
    printf("[INFO] 远程写入so的路径成功 写入:%d\n", nRet);
    
    //4. 获取`dlopen`在目标进程中的地址 
    uint64_t nLibDlBase = 0;
    if(GetModuleBase(nPid, "libdl.so", &nLibDlBase) < 0)
    {
        printf("[ERR] 获取libdl模块基址失败 \n");
        return 0;
    }
    nRet = dladdr((void*)dlopen, &di);
    if (nRet <= 0)
    {
        printf("[ERR] 无法获取mmap的地址 %s \n", dlerror());
        return 0;
    }
    void* pfnDlopen = (void*)(nLibDlBase + ((char*)di.dli_saddr - (char*)di.dli_fbase));
    printf("[INFO] 获取dlopen地址 %lx:%lx \n", nLibDlBase, pfnDlopen);

    //5. 远程调用`dlopen`
    //     * `X0` - so路径
    //     * `X1` - RTLD_NOW
    uint64_t nHandle = RemoteCall(nPid, pfnDlopen, {nRemoteAddr, RTLD_NOW});
    if (nHandle == FAILERD)
    {
        printf("[ERR] 远程调用dlopen失败 \n");
        return 0;
    }
    printf("[INFO] 远程调用dlopen成功 返回:%lx\n", nHandle);

    //脱离
    nRet = ptrace(PTRACE_DETACH, nPid, 0, 0);
    if (nRet < 0)
    {
        perror("[ERR] 脱离失败");
        return 0;
    }
    puts("[INF] 脱离成功");

    return 0;
}


int GetModuleBase(pid_t pid, const char* module_path, uint64_t* base_addr) {
    char maps_path[MAX_PATH_LEN];
    FILE* fp = NULL;
    char line[MAX_LINE_LEN];
    char path_buf[MAX_PATH_LEN];
    uint64_t start_addr;

    // 1. 构造 /proc/[PID]/maps 的路径
    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);

    // 2. 打开 maps 文件（非当前用户进程需 root 权限，否则 fopen 失败）
    fp = fopen(maps_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "[ERR] \tGetModuleBase fopen %s 失败：%s（可能需要 root 权限）\n", maps_path, strerror(errno));
        return -1;
    }

    // 3. 逐行读取并解析 maps 文件
    while (fgets(line, sizeof(line), fp) != NULL) {

        int ret = sscanf(
            line, 
            "%lx-%*lx %*s %*x %*s %*d %s", 
            &start_addr, 
            path_buf
        );

        // 解析失败则跳过当前行（异常格式）
        if (ret != 2) {
            continue;
        }

        // 4. 匹配模块路径（精确匹配，避免部分匹配）
        if (strstr(path_buf, module_path) != nullptr) {
            *base_addr = start_addr;

            printf("[INFO] \t GetModuleBase 模块 %s 在进程 %d 中的基地址：0x%lx\n", path_buf, pid, start_addr);
            return 0;
        }
    }

    // 5. 遍历结束未找到模块
    fprintf(stderr, "[ERR] \tGetModuleBase进程 %d 中未加载模块：%s\n", pid, module_path);
    fclose(fp);
    return -1;
}
