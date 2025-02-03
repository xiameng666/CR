#include "proto.h"
#include "MyUdp.h"
#include "MyUdp.cpp"
#include "threadpool.h"
#include "threadpool.cpp"
#include "StudentDB.h"
#include <LoginRequest.h>
#include "MessageTask.h"

int output(MYUDP_PCB* pcb, uint8_t* buf, uint32_t len)
{
    int result = sendto(pcb->s, (char*)buf, len, 0, &pcb->addr, sizeof(pcb->addr));
    if (result <= 0)
        return result;
    //MyUdpLog("[output] sendto buf:%p len:%d\n", buf, len);
    return result;
}

int StartServer() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return -1;
    }

    int s;
    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0) {
        return -1;
    }

    //将同步(阻塞)socket设置为异步(非阻塞)socket
    u_long val = 1;
    ioctlsocket(s, FIONBIO, &val);

    sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    /*addr.sin_addr.s_addr = inet_addr("127.0.0.1");*/  //inet_ntoa
    addr.sin_addr.s_addr = INADDR_ANY;
    sockaddr_in caddr = { 0 };
    int addrLen = sizeof(caddr);

    //绑定端口
    if (bind(s, (sockaddr*)&addr, sizeof(addr)) < 0) {
        return -1;
    }

    srand((unsigned)(time(NULL)));

    //是否支持连接
    uint32_t start = clock();
    uint8_t buf[10000];
    int count = 0;

    /*  改为通过三次握手创建PCB
    MYUDP_PCB* pcb = MyUdpCreatePcb(0x11223344, s, *(sockaddr*)&addr);
    MyUdpSetOutput(pcb, output);

    int flag = false;//控制定时发送消息的标志？
    */
    std::map<uint32_t, int> countMap;  // 为每个连接维护一个计数器

    ThreadPool threadPool;
    threadPool.setMode(PoolMode::MODE_CACHED);
    threadPool.start(1);


    //char logMsg[256];
    //sprintf_s(logMsg, "[thread]Thread pool started\n");
    //OutputDebugString(logMsg);

    printf("Server: start\n");
    while (true)
    {
        // 4. 遍历所有PCB，处理数据和更新状态
        uint32_t current = clock();

        for (auto& pair : g_Pcbs)
        {
            MYUDP_PCB* pcb = pair.second;

            //  检查是否有已排序好的完整数据可以处理 从接收队列获取已排序的数据
            int bytes = MyUdpRecv(pcb, buf, sizeof(buf));
            if (bytes > 0 && pcb->state == 1)
            {
                // 只处理已建立连接的数据   疑问：处理数据的逻辑在哪里
                printf("MyUdpRecv:从连接 %u 收到数据: %d, len:%d\n",
                    pcb->connId, *(int*)buf, bytes);

                //下面要将任务提交到线程池
                threadPool.submitTask(
                    std::make_shared<MessageTask>(buf, bytes, pcb->connId)
                );

            }

            // 定时发送数据 暂时先关闭
            /*
            if (pcb->state == 1 && current - start >= 300)
            {  // 只给已连接的PCB发送数据
                *(int*)buf = countMap[pcb->connId]++;
                int sendBytes = MyUdpSend(pcb, buf, sizeof(int));
                if (sendBytes > 0)
                {
                    printf("connId: %u 发送数据: %d, 发送字节数: %d\n",
                        pcb->connId,
                        *(int*)buf,
                        sendBytes);  // 应该是4
                }
            }
            */

            // 更新PCB状态
            MyUdpUpdate(pcb, current);
        }

        // 更新发送计时器
        if (current - start >= 300)
        {
            start = current;
        }

        //接收服务器的包   实现三次握手 将数据放入排序系统

        //1.接收新数据 
        int len = recvfrom(s, (char*)buf, sizeof(buf), 0, (sockaddr*)&addr, &addrLen);//这里存疑 是addr还是caddr
        if (len > 0)
        {

            MYUDP_SEG* seg = (MYUDP_SEG*)buf;
            MYUDP_PCB* pcb = nullptr;

            //2.处理握手和PCB创建
            if (seg->cmd == CMD_SYN)
            {
                printf("Server:收到SYN");
                // 收到客户端的SYN，创建新的PCB
                pcb = MyUdpCreatePcb(seg->connId, s, *(sockaddr*)&addr);
                MyUdpSetOutput(pcb, output);//这里只是设置回调函数？为什么只设置output
                countMap[seg->connId] = 0;

                // 回复SYNACK
                MYUDP_SEG synack = { 0 };
                synack.cmd = CMD_SYNACK;
                synack.connId = seg->connId;
                synack.seq = pcb->sendSeq;
                synack.una = pcb->recvSeq;
                /*int sendResult = MyUdpSend(pcb, (uint8_t*)&synack, sizeof(synack));*/
                int sendResult = sendto(s, (char*)&synack, sizeof(synack), 0,
                    (sockaddr*)&addr, sizeof(addr));

                if (sendResult > 0)
                {
                    printf("Server: 建立PCB成功 发送SYNACK, connId:%u\n", seg->connId);
                }
                else
                {
                    printf("Server: Failed to send SYNACK, connId:%u, error:%d\n",
                        seg->connId, WSAGetLastError());
                }
                continue;
            }
            else
            {
                //不是新连接 查找已存在的PCB
                pcb = MyUdpFindPcb(seg->connId);
                if (!pcb)
                {
                    printf("Server: 未知连接的PCB %u\n", seg->connId);
                    continue;
                }

                // 处理ESTAB
                if (seg->cmd == CMD_ESTAB)
                {
                    pcb->state = 1; // 标记连接已建立
                    printf("Server: 收到ESTAB包，三次握手完成,开始通讯 %u\n", seg->connId);
                }
            }

            /*flag = true;//这个应该是每个PCB对应一个flag？   */

            // 3. 数据包进入排序系统
            MyUdpInput(pcb, buf, len);
        }

    }

    // 清理资源
    for (auto& pair : g_Pcbs)
    {
        MyUdpReleasePcb(pair.second);
    }
    g_Pcbs.clear();

    return 0;
}



int main()
{

    StartServer();

    return 0;
}