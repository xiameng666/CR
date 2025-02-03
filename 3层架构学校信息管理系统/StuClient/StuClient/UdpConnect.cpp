#include "pch.h"
#include "UdpConnect.h"
#include "../../include/MyUdp.h"
#include "../../include/MyUdp.cpp"
#include <thread>


//单例模式
UdpConnect* UdpConnect::m_instance = nullptr;

UdpConnect::UdpConnect()
    : m_pcb(nullptr)
    , m_connected(false)
    , m_hWnd(NULL) 
    , m_running(false)
{}

UdpConnect::~UdpConnect()
{
    Close();
}

UdpConnect* UdpConnect::GetInstance()
{
    if (!m_instance) {
        m_instance = new UdpConnect();
    }
    return m_instance;
}

int UdpConnect::Init(HWND hWnd)
{
    SetMessageWindow(hWnd);

    //连接初始化
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return -1;
    }

    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0) {
        return -1;
    }

    //将同步(阻塞)socket设置为异步(非阻塞)socket
    u_long val = 1;
    ioctlsocket(s, FIONBIO, &val);

    sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    //addr.sin_addr.s_addr = inet_addr("127.0.0.1");  //inet_ntoa    
    addr.sin_addr.s_addr = inet_addr("192.168.124.2");  //inet_ntoa

    srand((size_t)time(NULL));
    uint32_t clientId = rand();  // 生成随机连接ID
    m_pcb = MyUdpCreatePcb(clientId, s, *(sockaddr*)&addr);//这里保存了socket和服务器地址
    MyUdpSetOutput(m_pcb, output);

    // 启动接收线程
    m_running = true;
    m_thread = std::thread(&UdpConnect::RecvThread, this);

    // 开始握手  发送SYN包
    if (!StartHandshake())
    {
        TRACE("客户端:发送握手消息1失败");
        return -1;
    }

    // 等待握手完成
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_condEst.wait_for(lock,
            std::chrono::seconds(5),  // 添加超时
            [this]() { return IsConnected(); }))
        {
            TRACE("客户端:五秒等你握手还超时\n");
            return -2;
        }
    }

    return 0;
}

void UdpConnect::RecvThread()
{
    uint8_t buf[10000];
    sockaddr_in addr;
    int addrLen = sizeof(addr);

    while (m_running) {
        // 接收数据
        int len = recvfrom(m_pcb->s, (char*)buf, sizeof(buf), 0,
            (sockaddr*)&addr, &addrLen);

        if (len > 0) {
            if (!IsConnected()) {
                // 处理握手
                MYUDP_SEG* seg = (MYUDP_SEG*)buf;
                if (seg->cmd == CMD_SYNACK) {
                    // 发送ESTAB
                    MYUDP_SEG estab = { 0 };
                    estab.cmd = CMD_ESTAB;
                    estab.connId = m_pcb->connId;
                    estab.seq = m_pcb->sendSeq;

                    sendto(m_pcb->s, (char*)&estab, sizeof(estab), 0,
                        &m_pcb->addr, sizeof(m_pcb->addr));

                    m_connected = true;
                    m_pcb->state = 1;
                    m_condEst.notify_all();//通知init() 连接已建立
                    TRACE("[connect]收到CMD_SYNACK，发送CMD_ESTAB\n");
                }
            }
            else {
                // 处理业务数据
                MyUdpInput(m_pcb, buf, len);

                // 检查完整数据
                int bytes = MyUdpRecv(m_pcb, buf, sizeof(buf));
                if (bytes > 0) {
                    // 复制数据并通知主线程
                    uint8_t* msgData = new uint8_t[bytes];
                    memcpy(msgData, buf, bytes);

                    ::PostMessage(m_hWnd, WM_SOCKET, (WPARAM)msgData, bytes);
                }
            }
        }

        // 更新PCB状态
        MyUdpUpdate(m_pcb, clock());
        Sleep(1);
    }
}

void UdpConnect::Close()
{
    m_running = false;  // 停止线程
    if (m_thread.joinable()) {
        m_thread.join();     // 等待线程结束
    }


    if (m_pcb) {
        closesocket(m_pcb->s);  // 关闭PCB中的socket
        MyUdpReleasePcb(m_pcb);
    }

    WSACleanup();
}

/*
调用MyUdpSend进行可靠传输
应用层 -> SendData -> MyUdpSend -> 分片 -> output -> 网络发送
*/
bool UdpConnect::SendData(const void* buf, int len)
{
    if (!IsConnected() || !buf || len <= 0) return false;

    return MyUdpSend(m_pcb, (uint8_t*)buf, len) > 0;
}

void UdpConnect::SetMessageWindow(HWND hWnd)
{
    m_hWnd = hWnd;
    //if (m_hWnd == NULL) {
    //    
    //}
    TRACE("[connect]MsgWindow :0x%08X", m_hWnd);
}

HWND UdpConnect::GetMessageWindow() const
{
    return m_hWnd;
}

//直接调用系统API发送原始数据
int UdpConnect::output(MYUDP_PCB* pcb, uint8_t* buf, uint32_t len)
{
    return sendto(pcb->s, (char*)buf, len, 0, &pcb->addr, sizeof(pcb->addr));
}

bool UdpConnect::StartHandshake()
{
    MYUDP_SEG syn = { 0 };
    syn.cmd = CMD_SYN;
    syn.connId = m_pcb->connId;
    syn.seq = m_pcb->sendSeq;

    //C->S 发送SYN
    return sendto(m_pcb->s, (char*)&syn, sizeof(syn), 0,
        &m_pcb->addr, sizeof(m_pcb->addr)) > 0;
    TRACE("[connect]C->S 发送SYN\n");
}
