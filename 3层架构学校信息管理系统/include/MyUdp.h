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
#define MSS   1300  //每片的最大字节数
#define WND_SIZE   128  //128包

#define CMD_PSH     0
#define CMD_ACK     1

#define CMD_SYN     2
#define CMD_SYNACK  3
#define CMD_ESTAB   4



//协议头 一个片
struct MYUDP_SEG {
  uint32_t cmd;       //命令
  uint32_t connId;    //id
  uint32_t seq;       //包序号
  uint32_t frame;     //片号
  uint32_t una;       //una确认号
  uint32_t ts;        //发送时间
  uint32_t rto;       //重传时间
  uint32_t rtc;       //重传次数
  uint32_t len;       //数据长度
  //uint8_t  data[1];   //数据
};


//控制块
struct MYUDP_PCB {
  uint32_t connId;    //id
  uint32_t mss;      //片数据最大大小
  uint32_t sendSeq;  //下一个要发送的编号
  uint32_t prevUna;  //
  uint32_t recvSeq;  //下一个要接收的编号
  uint32_t ts;       //当前时间
  uint32_t rto;      //重传时间
  uint32_t rtc;      //重传次数
  uint32_t state;    //状态
  uint32_t sendWnd;  //发送窗口大小
  uint32_t recvWnd;  //接收窗口大小
  int s;
  sockaddr addr;
  std::list<MYUDP_SEG*> sendQueue; //发送队列
  std::list<MYUDP_SEG*> sendBuf;   //发送缓冲区
  std::list<MYUDP_SEG*> recvQueue; //接收队列
  std::list<MYUDP_SEG*> recvBuf;   //接收缓冲区
  std::list<uint32_t>   ackQueue;   //确认队列

  //输入输出回调函数
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