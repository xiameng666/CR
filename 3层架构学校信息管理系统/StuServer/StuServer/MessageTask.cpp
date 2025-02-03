#include <MyUdp.h>
#include "MessageTask.h"
#include "IServerRequest.h"
#include "ServerRequestFactory.h"




MessageTask::MessageTask(uint8_t* buf, uint32_t len, uint32_t id)
    :  connId(id)
{
    buffer.resize(len);
    memcpy(buffer.data(), buf, len);
}

Any MessageTask::run()
{
    int msgType = *(int*)buffer.data();
    printf("处理客户端%u的请求，消息类型:%d\n", connId, msgType);

    // 1. 获取数据库连接
    auto db = StudentDB::GetInstance();
    if (!db) {
        SendResponse(msgType, -1, "数据库连接失败");
        return 0;
    }

    // 使用简单工厂创建请求对象
    // 在不同派生类的Exec函数中通过switch(msgType)执行不同的db函数
    IServerRequest* serverRequest = ServerRequestFactory::CreateRequest(msgType);

    /*IServerRequest* serverRequest = nullptr;*/
    if (!serverRequest) {
        SendResponse(msgType, -1, "未知的消息类型");
        return 0;
    }

    //解析数据包体buffer 反序列化成请求
    if (!serverRequest->Deserialize((char*)buffer.data(), buffer.size())) {
        delete serverRequest;
        SendResponse(msgType, -1, "无效的请求");
        return 0;
    }

    if (MYSQL_RES* result = serverRequest->ExecR(db)) {
        // 如果是查询操作且查询成功，result非空
        char tempBuffer[8192] = { 0 };
        int dataSize = 0;
        if (db->ParseSqlResult(result, tempBuffer, dataSize)) {
            SendQueryResponse(msgType, tempBuffer, dataSize);
        }
        mysql_free_result(result);
    }
    else {
        // 两种情况会进入这里：
        // 1. 非查询操作（ExecR返回nullptr）
        // 2. 查询操作但失败了（也返回nullptr）
        bool success = serverRequest->ExecCUD(db);
        SendResponse(msgType, success ? 0 : -1,
            success ? "操作成功" : "操作失败");
    }

    delete serverRequest;

    return 0;
}

void MessageTask::SendQueryResponse(int msgType, const char* resultBuffer, int dataSize)
{
    // 1. 计算总响应大小
    int totalSize = sizeof(ResponseMsg) - sizeof(QueryResult) + dataSize;

    // 2. 分配响应缓冲区   这里简便点 TODO
    std::unique_ptr<uint8_t[]> rspBuffer(new uint8_t[totalSize]);
    ResponseMsg* rsp = (ResponseMsg*)rspBuffer.get();

    // 3. 设置响应头
    rsp->msgType = msgType;
    rsp->rspType = RSP_HAS_RESULT;
    rsp->result = 0;  // 成功
    strcpy(rsp->errMsg, "查询成功");

    // 4. 复制查询结果数据
    memcpy(&rsp->queryResult, resultBuffer, dataSize);

    // 5. 发送响应
    MYUDP_PCB* pcb = MyUdpFindPcb(connId);
    if (pcb) {
        MyUdpSend(pcb, rspBuffer.get(), totalSize);
        printf("[响应] 客户端%u - 消息类型:%d, 响应类型:%d, 结果:%d, 消息:%s, 数据大小:%d\n",
            connId, msgType, RSP_HAS_RESULT, rsp->result, rsp->errMsg, dataSize);
    }
    else {
        printf("[错误] 客户端%u - 找不到PCB\n", connId);
    }
}

//不需要附带查询数据集的发送
void MessageTask::SendResponse(int msgType, int result, const char* msg)
{
    ResponseMsg rsp = { 0 };
    rsp.msgType = msgType;
    rsp.result = result;
    strcpy(rsp.errMsg, msg);//确保以/0结尾

    //设置响应的类型
    if (msgType == MSG_ADMIN_LOGIN) {
        rsp.rspType = RSP_LOGIN;
    }
    else {
        rsp.rspType = RSP_NO_RESULT;
    }

    if (msgType == MSG_HEARTBEAT) {
        rsp.rspType = RSP_HEARTBEAT;
    }

    MYUDP_PCB* pcb = MyUdpFindPcb(connId);
    if (pcb)
    {
        MyUdpSend(pcb, (uint8_t*)&rsp, sizeof(ResponseMsg));
        printf("[响应] 客户端%u - 消息类型:%d, 响应类型:%d, 结果:%d, 消息:%s\n",
            connId, msgType, rsp.rspType, result, msg);
    }
    else
    {
        printf("[错误] 客户端%u - 找不到PCB\n", connId);
    }

}
