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
    printf("����ͻ���%u��������Ϣ����:%d\n", connId, msgType);

    // 1. ��ȡ���ݿ�����
    auto db = StudentDB::GetInstance();
    if (!db) {
        SendResponse(msgType, -1, "���ݿ�����ʧ��");
        return 0;
    }

    // ʹ�ü򵥹��������������
    // �ڲ�ͬ�������Exec������ͨ��switch(msgType)ִ�в�ͬ��db����
    IServerRequest* serverRequest = ServerRequestFactory::CreateRequest(msgType);

    /*IServerRequest* serverRequest = nullptr;*/
    if (!serverRequest) {
        SendResponse(msgType, -1, "δ֪����Ϣ����");
        return 0;
    }

    //�������ݰ���buffer �����л�������
    if (!serverRequest->Deserialize((char*)buffer.data(), buffer.size())) {
        delete serverRequest;
        SendResponse(msgType, -1, "��Ч������");
        return 0;
    }

    if (MYSQL_RES* result = serverRequest->ExecR(db)) {
        // ����ǲ�ѯ�����Ҳ�ѯ�ɹ���result�ǿ�
        char tempBuffer[8192] = { 0 };
        int dataSize = 0;
        if (db->ParseSqlResult(result, tempBuffer, dataSize)) {
            SendQueryResponse(msgType, tempBuffer, dataSize);
        }
        mysql_free_result(result);
    }
    else {
        // ���������������
        // 1. �ǲ�ѯ������ExecR����nullptr��
        // 2. ��ѯ������ʧ���ˣ�Ҳ����nullptr��
        bool success = serverRequest->ExecCUD(db);
        SendResponse(msgType, success ? 0 : -1,
            success ? "�����ɹ�" : "����ʧ��");
    }

    delete serverRequest;

    return 0;
}

void MessageTask::SendQueryResponse(int msgType, const char* resultBuffer, int dataSize)
{
    // 1. ��������Ӧ��С
    int totalSize = sizeof(ResponseMsg) - sizeof(QueryResult) + dataSize;

    // 2. ������Ӧ������   ������� TODO
    std::unique_ptr<uint8_t[]> rspBuffer(new uint8_t[totalSize]);
    ResponseMsg* rsp = (ResponseMsg*)rspBuffer.get();

    // 3. ������Ӧͷ
    rsp->msgType = msgType;
    rsp->rspType = RSP_HAS_RESULT;
    rsp->result = 0;  // �ɹ�
    strcpy(rsp->errMsg, "��ѯ�ɹ�");

    // 4. ���Ʋ�ѯ�������
    memcpy(&rsp->queryResult, resultBuffer, dataSize);

    // 5. ������Ӧ
    MYUDP_PCB* pcb = MyUdpFindPcb(connId);
    if (pcb) {
        MyUdpSend(pcb, rspBuffer.get(), totalSize);
        printf("[��Ӧ] �ͻ���%u - ��Ϣ����:%d, ��Ӧ����:%d, ���:%d, ��Ϣ:%s, ���ݴ�С:%d\n",
            connId, msgType, RSP_HAS_RESULT, rsp->result, rsp->errMsg, dataSize);
    }
    else {
        printf("[����] �ͻ���%u - �Ҳ���PCB\n", connId);
    }
}

//����Ҫ������ѯ���ݼ��ķ���
void MessageTask::SendResponse(int msgType, int result, const char* msg)
{
    ResponseMsg rsp = { 0 };
    rsp.msgType = msgType;
    rsp.result = result;
    strcpy(rsp.errMsg, msg);//ȷ����/0��β

    //������Ӧ������
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
        printf("[��Ӧ] �ͻ���%u - ��Ϣ����:%d, ��Ӧ����:%d, ���:%d, ��Ϣ:%s\n",
            connId, msgType, rsp.rspType, result, msg);
    }
    else
    {
        printf("[����] �ͻ���%u - �Ҳ���PCB\n", connId);
    }

}
