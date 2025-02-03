#define _CRT_SECURE_NO_WARNINGS
#include "MyUdp.h"

std::map<uint32_t, MYUDP_PCB*> g_Pcbs;//connID对应PCB
std::mutex g_PcbMutex;//g_Pcbs线程安全

const char* g_cmdString[] = {
  "CMD_PSH",
  "CMD_ACK"

  "CMD_SYN"
  "CMD_SYNACK"
  "CMD_ESTAB"
};

MYUDP_PCB* MyUdpCreatePcb(uint32_t connId, int s, sockaddr addr) {
  MYUDP_PCB* pcb = new MYUDP_PCB();
  pcb->connId = connId;
  pcb->s = s;
  pcb->addr = addr;
  pcb->mss = MSS;
  pcb->sendSeq = 0;
  pcb->prevUna = 0;
  pcb->ts = 0;
  pcb->recvSeq = 0;
  pcb->rto = 300;
  pcb->rtc = 6;
  pcb->state = 0;
  pcb->sendWnd = 10;
  pcb->recvWnd = 10;
  {
      std::lock_guard<std::mutex> lock(g_PcbMutex);
      g_Pcbs.insert({ connId , pcb });
  }
  MyUdpLog("[MyUdpCreatePcb] pcb:%p\n", pcb);
  return pcb;
}

void MyUdpReleasePcb(MYUDP_PCB* pcb) {//都结束了就不管同步了
    auto it = g_Pcbs.find(pcb->connId);
    if (it != g_Pcbs.end()) {
        g_Pcbs.erase(it);
    }

    delete pcb;
}

int MyUdpSend(MYUDP_PCB* pcb, uint8_t*buf, uint32_t len) {

  if (pcb->sendQueue.size() >= 128)
    return -1;


  //分片
  int count = len / pcb->mss;
  count = (len % pcb->mss == 0) ? count : count + 1;

  uint32_t size = 0;
  uint32_t mss = 0;
  for (int i = 0; i < count; i++) {
    //创建一个片
    MYUDP_SEG* seg = NULL;
    if (len >= pcb->mss) {
      mss = pcb->mss;
      len -= pcb->mss;
    }
    else {
      mss = len;
    }

    //放入发送队列
    seg = MyUdpCreateSegment(pcb, mss);
    seg->frame = count - i - 1;
    seg->len = mss;
    memcpy(seg + 1, buf + size, mss);
    size += mss;
    pcb->sendQueue.push_back(seg);
    MyUdpLogSeg("[MyUdpSend] sendQueue", pcb, seg);
  }

  return size;
}

int MyUdpPeekSize(MYUDP_PCB* pcb) {
  int size = 0;
  if (pcb->recvQueue.empty())
    return 0;

  //遍历接收队列
  bool flag = false;
  for (auto seg : pcb->recvQueue) {
    size += seg->len;

    if (seg->frame == 0) {
      flag = true;
      break;
    }
  }

  return flag ? size : -1;
}

int MyUdpRecv(MYUDP_PCB* pcb, uint8_t* buf, uint32_t len) {
  //接收数据(同步还是异步)
  uint32_t peekSize = MyUdpPeekSize(pcb);
  int copySize = 0;
  if (peekSize <= 0)
    return -1;  //数据不完整
  else if (len < peekSize) {
    return  -2;  //缓冲区大小不够
  }

  //拷贝数据            疑问：处理数据的逻辑在哪里
  for (auto it = pcb->recvQueue.begin(); it != pcb->recvQueue.end();) {
    MYUDP_SEG* seg = *it;
    memcpy(buf + copySize, seg + 1, seg->len);
    copySize += seg->len;

    it = pcb->recvQueue.erase(it);
    if (seg->frame == 0) {
      free(seg);
      break;
    }
    else {
      free(seg);
    }
  }

  //将接收缓冲区的数据按顺序移动到接收队列(排序)
  if (!pcb->recvBuf.empty()) {
    MYUDP_SEG* seg = NULL;
    for (auto p : pcb->recvBuf) {
      if (p->seq == pcb->recvSeq) {
        seg = p;
        break;
      }
    }
    if (seg != NULL) {
      pcb->recvBuf.remove(seg);
      pcb->recvQueue.push_back(seg);
      pcb->recvSeq++;
      MyUdpLogSeg("[MyUdpRecv] recvQueue", pcb, seg);
    }
  }

  return copySize;
}


int MyUdpUpdate(MYUDP_PCB* pcb, uint32_t current) {
  int result = 0;
  if (current - pcb->ts > 30) {
    result = MyUdpFlush(pcb);
    pcb->ts = current;
  }

  return result;
}

MYUDP_SEG* MyUdpCreateSegment(MYUDP_PCB* pcb, uint32_t len) {
  MYUDP_SEG *seg = (MYUDP_SEG*)malloc(sizeof(MYUDP_SEG) + len);
  memset(seg, 0, sizeof(MYUDP_SEG));
  return seg;
}

int MyUdpFlush(MYUDP_PCB* pcb) {

  //发送确认包
  if (!pcb->ackQueue.empty()) {
    MYUDP_SEG seg = { 0 };
    seg.cmd = CMD_ACK;
    seg.connId = pcb->connId;
    seg.ts = pcb->ts;
    seg.una = pcb->recvSeq;

    for (auto seq : pcb->ackQueue) {
      seg.seq = seq;
      pcb->output(pcb, (uint8_t*)&seg, sizeof(seg) + seg.len);
      MyUdpLogSeg("[MyUdpFlush] output", pcb, &seg);
    }
    pcb->ackQueue.clear();
  }
 
  //将发送队列的数据移动到发送缓冲区
  //按窗口大小移动发送缓冲区  4    1+4  una+wnd
  for (uint32_t i = pcb->sendSeq; !pcb->sendQueue.empty() && i < pcb->prevUna + pcb->sendWnd; i++) {
    auto seg = pcb->sendQueue.front();
    pcb->sendQueue.pop_front();
    seg->connId = pcb->connId;
    seg->seq = pcb->sendSeq++;
    seg->una = pcb->recvSeq;
    seg->ts = pcb->ts;
    seg->rto = pcb->rto;
    seg->rtc = 0;

    pcb->sendBuf.push_back(seg);
    MyUdpLogSeg("[MyUdpFlush] sendBuf", pcb, seg);
  }

  //发送数据
  for (auto seg : pcb->sendBuf) {
    //第一次发送
    if (seg->rtc == 0) {
      seg->rtc = 1;
      seg->ts = pcb->ts;

      pcb->output(pcb, (uint8_t*)seg, seg->len + sizeof(MYUDP_SEG));
      MyUdpLogSeg("[MyUdpFlush] output", pcb, seg);
    }
    //超时重发
    else {
      //超过从发次数, 断开连接
      if (seg->rtc >= pcb->rtc) {
        pcb->state = -1;
        return -1;
      }
      else if (pcb->ts - seg->ts >= seg->rto) {
        seg->rtc++;
        seg->rto = seg->rto * 2;
        seg->ts = pcb->ts;
        pcb->output(pcb, (uint8_t*)seg, seg->len + sizeof(MYUDP_SEG));
        MyUdpLogSeg("[MyUdpFlush] rto", pcb, seg);
      }
    }
  }


  return 0;
}

int MyUdpSetOutput(MYUDP_PCB* pcb, int (*output)(MYUDP_PCB* pcb, uint8_t* buf, uint32_t len)) {
  pcb->output = output;
  return 0;
}

int MyUdpSetInput(MYUDP_PCB* pcb, int (*input)(MYUDP_PCB* pcb, uint8_t* buf, uint32_t len)) {
  pcb->input = input;
  return 0;
}

int MyUdpLog(const char* fmt, ...) {
  char buf[4096];
  va_list argptr;
  va_start(argptr, fmt);
  _vsnprintf(buf, sizeof(buf), fmt, argptr);
  printf("%s", buf);
  va_end(argptr);
  return 0;
}

int MyUdpInput(MYUDP_PCB* pcb, uint8_t* buf, uint32_t len) {
  //接收数据
  MYUDP_SEG* p = (MYUDP_SEG*)buf;

  //检测
  if (!(p->cmd == CMD_PSH || p->cmd == CMD_ACK)) {
    return -1;
  }

  MyUdpLogSeg("[MyUdpInput] input", pcb, p);


  if (pcb->recvBuf.size() >= 128)
    return -1;


  //una确认 删除una所有包   3    4
  if (p->una > pcb->prevUna) {
    for (auto it = pcb->sendBuf.begin(); it != pcb->sendBuf.end();) {
      MYUDP_SEG* seg = *it;
      if (seg->seq >= p->una)
        break;

      it = pcb->sendBuf.erase(it);
      MyUdpLogSeg("[MyUdpInput] una delete", pcb, seg);
      free(seg);
    }
    pcb->prevUna = p->una;
  }
  
  if (p->cmd == CMD_ACK) {  //0 1 2 3
    if (!pcb->sendBuf.empty() 
      && p->seq >= pcb->sendBuf.front()->seq 
      && p->seq <= pcb->sendBuf.back()->seq) {
      for (auto it = pcb->sendBuf.begin(); it != pcb->sendBuf.end(); it++) {
        MYUDP_SEG* seg = *it;
        if (seg->seq == p->seq) {
          it = pcb->sendBuf.erase(it);
          MyUdpLogSeg("[MyUdpInput] delete", pcb, seg);
          free(seg);
          break;
        }
      }
    }
  }
  else if (p->cmd == CMD_PSH) {
    if (p->seq >= pcb->recvSeq) {
      bool repeat = false;
      for (auto seg : pcb->recvBuf) {
        if (seg->seq == p->seq) {
          repeat = true;
          break;
        }
      }
      //数重复了
      if (!repeat) {
        MYUDP_SEG* seg = MyUdpCreateSegment(pcb, p->len);
        *seg = *p;
        memcpy(seg + 1, p + 1, p->len);

        //放入缓冲区
        pcb->recvBuf.push_back(seg);
        MyUdpLogSeg("[MyUdpInput] recvBuf", pcb, seg);
      }
    }

    //待发送确认包
    pcb->ackQueue.push_back(p->seq);

    //将接收缓冲区的数据按顺序移动到接收队列(排序)
    if (!pcb->recvBuf.empty()) {
      MYUDP_SEG* seg = NULL;
      for (auto p : pcb->recvBuf) {
        if (p->seq == pcb->recvSeq) {
          seg = p;
          break;
        }
      }
      if (seg != NULL) {
        pcb->recvBuf.remove(seg);
        pcb->recvQueue.push_back(seg);
        pcb->recvSeq++;
        MyUdpLogSeg("[MyUdpInput] recvQueue", pcb, seg);
      }
    }
  }

  return 0;
}


int MyUdpLogSeg(const char* fmt, MYUDP_PCB* pcb, MYUDP_SEG* seg) {
  //MyUdpLog("%-19s sendSeq:%d prevUna:%d recvSeq:%d cmd:%s seq:%d una:%d frame:%d ack:%d ts:%d len:%d rto:%d rtc:%d\n",
  //  fmt,
  //  pcb->sendSeq,
  //  pcb->prevUna,
  //  pcb->recvSeq,
  //  g_cmdString[seg->cmd],   //命令
  //  seg->seq,       //包序号
  //  seg->una,
  //  seg->frame,      //片号
  //  seg->una,       //确认号
  //  seg->ts,         //发送时间
  //  seg->len,        //数据长度
  //  seg->rto,
  //  seg->rtc
  //          );
  return 0;
}


MYUDP_PCB* MyUdpFindPcb(uint32_t connId) {
    std::lock_guard<std::mutex> lock(g_PcbMutex);

  auto it = g_Pcbs.find(connId);
  if (it != g_Pcbs.end()) {
    return it->second;
  }
  return NULL;
}