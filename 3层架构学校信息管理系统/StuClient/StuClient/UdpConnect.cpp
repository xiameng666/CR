#include "pch.h"
#include "UdpConnect.h"
#include "../../include/MyUdp.h"
#include "../../include/MyUdp.cpp"
#include <thread>


//����ģʽ
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

    //���ӳ�ʼ��
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return -1;
    }

    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0) {
        return -1;
    }

    //��ͬ��(����)socket����Ϊ�첽(������)socket
    u_long val = 1;
    ioctlsocket(s, FIONBIO, &val);

    sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    //addr.sin_addr.s_addr = inet_addr("127.0.0.1");  //inet_ntoa    
    addr.sin_addr.s_addr = inet_addr("192.168.124.2");  //inet_ntoa

    srand((size_t)time(NULL));
    uint32_t clientId = rand();  // �����������ID
    m_pcb = MyUdpCreatePcb(clientId, s, *(sockaddr*)&addr);//���ﱣ����socket�ͷ�������ַ
    MyUdpSetOutput(m_pcb, output);

    // ���������߳�
    m_running = true;
    m_thread = std::thread(&UdpConnect::RecvThread, this);

    // ��ʼ����  ����SYN��
    if (!StartHandshake())
    {
        TRACE("�ͻ���:����������Ϣ1ʧ��");
        return -1;
    }

    // �ȴ��������
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_condEst.wait_for(lock,
            std::chrono::seconds(5),  // ��ӳ�ʱ
            [this]() { return IsConnected(); }))
        {
            TRACE("�ͻ���:����������ֻ���ʱ\n");
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
        // ��������
        int len = recvfrom(m_pcb->s, (char*)buf, sizeof(buf), 0,
            (sockaddr*)&addr, &addrLen);

        if (len > 0) {
            if (!IsConnected()) {
                // ��������
                MYUDP_SEG* seg = (MYUDP_SEG*)buf;
                if (seg->cmd == CMD_SYNACK) {
                    // ����ESTAB
                    MYUDP_SEG estab = { 0 };
                    estab.cmd = CMD_ESTAB;
                    estab.connId = m_pcb->connId;
                    estab.seq = m_pcb->sendSeq;

                    sendto(m_pcb->s, (char*)&estab, sizeof(estab), 0,
                        &m_pcb->addr, sizeof(m_pcb->addr));

                    m_connected = true;
                    m_pcb->state = 1;
                    m_condEst.notify_all();//֪ͨinit() �����ѽ���
                    TRACE("[connect]�յ�CMD_SYNACK������CMD_ESTAB\n");
                }
            }
            else {
                // ����ҵ������
                MyUdpInput(m_pcb, buf, len);

                // �����������
                int bytes = MyUdpRecv(m_pcb, buf, sizeof(buf));
                if (bytes > 0) {
                    // �������ݲ�֪ͨ���߳�
                    uint8_t* msgData = new uint8_t[bytes];
                    memcpy(msgData, buf, bytes);

                    ::PostMessage(m_hWnd, WM_SOCKET, (WPARAM)msgData, bytes);
                }
            }
        }

        // ����PCB״̬
        MyUdpUpdate(m_pcb, clock());
        Sleep(1);
    }
}

void UdpConnect::Close()
{
    m_running = false;  // ֹͣ�߳�
    if (m_thread.joinable()) {
        m_thread.join();     // �ȴ��߳̽���
    }


    if (m_pcb) {
        closesocket(m_pcb->s);  // �ر�PCB�е�socket
        MyUdpReleasePcb(m_pcb);
    }

    WSACleanup();
}

/*
����MyUdpSend���пɿ�����
Ӧ�ò� -> SendData -> MyUdpSend -> ��Ƭ -> output -> ���緢��
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

//ֱ�ӵ���ϵͳAPI����ԭʼ����
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

    //C->S ����SYN
    return sendto(m_pcb->s, (char*)&syn, sizeof(syn), 0,
        &m_pcb->addr, sizeof(m_pcb->addr)) > 0;
    TRACE("[connect]C->S ����SYN\n");
}
