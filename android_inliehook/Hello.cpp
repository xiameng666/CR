#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdatomic.h>
#include "Hook.h"

using PFN_open = int (*)(const char *path, int oflag, ...);

PFN_open g_pfnNewOpen = nullptr;
int FakedOpen(const char* szPath, int nFlag, int nMod)
{
    printf("open被调用 path:%s \n", szPath);
    return g_pfnNewOpen("aaa.txt", nFlag, nMod);
}


atomic_int n = 0;
void* Thread(void* )
{
    for (int i = 0; i < 100; i++)
    {
        atomic_store(&n, i);
    }
    printf("%d \n", atomic_load(&n));
}

int main()
{
    pthread_create(nullptr, nullptr, Thread, nullptr);
    

    //安装hook
    InstallHook((void*)(PFN_open)open64, (void*)FakedOpen, (void**)&g_pfnNewOpen);

    int nFD = open64("test.txt", O_CREAT|O_RDWR, 0777);
    if (nFD < 0)
    {
        perror("打开文件失败");
        return  0;
    }
    printf("写入文件成功 \n");
    
    char sz[] = {"hello"};
    int nRet = write(nFD, sz, sizeof(sz));
    if (nRet < 0)
    {
        perror("写入失败");
    }
    
    close(nFD);
    printf("写入文件结束 \n");
    return 0;
}