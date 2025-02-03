#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <malloc.h>
#include <stdarg.h>
#include <map>
#include <stdint.h>
#include <time.h>
#include <list>
#include <stdio.h>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include<mutex>
#pragma comment(lib, "Ws2_32.lib")

#define MTU   1400
#define MSS   1300  //ÿƬ������ֽ���
#define WND_SIZE   128  //128��

#define CMD_PSH     0
#define CMD_ACK     1

#define CMD_SYN     2
#define CMD_SYNACK  3
#define CMD_ESTAB   4



//Э��ͷ һ��Ƭ
struct MYUDP_SEG {
  uint32_t cmd;       //����
  uint32_t connId;    //id
  uint32_t seq;       //�����
  uint32_t frame;     //Ƭ��
  uint32_t una;       //unaȷ�Ϻ�
  uint32_t ts;        //����ʱ��
  uint32_t rto;       //�ش�ʱ��
  uint32_t rtc;       //�ش�����
  uint32_t len;       //���ݳ���
  //uint8_t  data[1];   //����
};


//���ƿ�
struct MYUDP_PCB {
  uint32_t connId;    //id
  uint32_t mss;      //Ƭ��������С
  uint32_t sendSeq;  //��һ��Ҫ���͵ı��
  uint32_t prevUna;  //
  uint32_t recvSeq;  //��һ��Ҫ���յı��
  uint32_t ts;       //��ǰʱ��
  uint32_t rto;      //�ش�ʱ��
  uint32_t rtc;      //�ش�����
  uint32_t state;    //״̬
  uint32_t sendWnd;  //���ʹ��ڴ�С
  uint32_t recvWnd;  //���մ��ڴ�С
  int s;
  sockaddr addr;
  std::list<MYUDP_SEG*> sendQueue; //���Ͷ���
  std::list<MYUDP_SEG*> sendBuf;   //���ͻ�����
  std::list<MYUDP_SEG*> recvQueue; //���ն���
  std::list<MYUDP_SEG*> recvBuf;   //���ջ�����
  std::list<uint32_t>   ackQueue;   //ȷ�϶���

  //��������ص�����
  int (*output)(MYUDP_PCB* pcb, uint8_t* buf, uint32_t len);
  int (*input)(MYUDP_PCB* pcb, uint8_t* buf, uint32_t len);
};

MYUDP_PCB* MyUdpFindPcb(uint32_t connId);


MYUDP_PCB* MyUdpCreatePcb(uint32_t connId, int s, sockaddr addr);

void MyUdpReleasePcb(MYUDP_PCB* pcb);

int MyUdpSetOutput(MYUDP_PCB* pcb, int (*output)(MYUDP_PCB* pcb, uint8_t* buf, uint32_t len));

int MyUdpSetInput(MYUDP_PCB* pcb, int (*input)(MYUDP_PCB* pcb, uint8_t* buf, uint32_t len));

int MyUdpSend(MYUDP_PCB* pcb, uint8_t* buf, uint32_t len);

int MyUdpRecv(MYUDP_PCB* pcb, uint8_t* buf, uint32_t len);

int MyUdpUpdate(MYUDP_PCB* pcb, uint32_t current);

int MyUdpFlush(MYUDP_PCB* pcb);

MYUDP_SEG* MyUdpCreateSegment(MYUDP_PCB* pcb, uint32_t len);

int MyUdpLog(const char* fmt, ...);

int MyUdpInput(MYUDP_PCB* pcb, uint8_t* buf, uint32_t len);

int MyUdpLogSeg(const char* fmt, MYUDP_PCB* pcb, MYUDP_SEG *seg);

extern std::map<uint32_t, MYUDP_PCB*> g_Pcbs;
extern std::mutex g_PcbMutex;