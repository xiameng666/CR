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
	HWND m_hWnd;//Ҫ֪ͨ��Ϣ�Ĵ���

    std::thread  m_thread;
    bool m_running;     //�߳����б�־

    std::condition_variable m_condEst;//���͵��������ֵ��ź��� ��init�ȴ�����ź��� �ɹ����ֲŷ���init����
    std::mutex m_mutex;//������
public:
    static UdpConnect* GetInstance();

    // ��ʼ��������
    int Init(HWND hWnd);

    void RecvThread();

    void Close();

    // ��������
    bool SendData(const void* buf, int len);

    // ״̬��ѯ
    bool IsConnected() const { return m_connected && m_pcb && m_pcb->state == 1; }
    MYUDP_PCB* GetPCB() { return m_pcb; }


    void SetMessageWindow(HWND hWnd);
    HWND GetMessageWindow() const;
private:
    static int output(MYUDP_PCB* pcb, uint8_t* buf, uint32_t len);
    bool StartHandshake();
};


