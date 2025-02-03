#pragma once
#include "pch.h"
#include "../../include/MyUdp.h"
#include <thread>
#include <condition_variable>
#include <mutex>

class UdpConnect
{
private:
	UdpConnect();
	~UdpConnect();

	static UdpConnect* m_instance;

	MYUDP_PCB* m_pcb;
	bool m_connected;
	HWND m_hWnd;//要通知消息的窗口

    std::thread  m_thread;
    bool m_running;     //线程运行标志

    std::condition_variable m_condEst;//发送第三次握手的信号量 让init等待这个信号量 成功握手才返回init函数
    std::mutex m_mutex;//配套锁
public:
    static UdpConnect* GetInstance();

    // 初始化和连接
    int Init(HWND hWnd);

    void RecvThread();

    void Close();

    // 发送数据
    bool SendData(const void* buf, int len);

    // 状态查询
    bool IsConnected() const { return m_connected && m_pcb && m_pcb->state == 1; }
    MYUDP_PCB* GetPCB() { return m_pcb; }


    void SetMessageWindow(HWND hWnd);
    HWND GetMessageWindow() const;
private:
    static int output(MYUDP_PCB* pcb, uint8_t* buf, uint32_t len);
    bool StartHandshake();
};


