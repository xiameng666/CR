// ServerHeartbeatRequest.h
#pragma once
#include "IServerRequest.h"

// ����������������
class ServerHeartbeatRequest : public IServerRequest {
private:
    HeartbeatRequest* request_;  // ���

public:
    ServerHeartbeatRequest(int msgType) {
        request_ = new HeartbeatRequest();
    }

    ~ServerHeartbeatRequest() {
        delete request_;
    }

    bool Deserialize(const char* buffer, int len) {
        return request_->Deserialize(buffer, len);
    }

    MYSQL_RES* ExecR(StudentDB* db) override {
        // ����������Ҫ��ѯ���ݿ�
        return nullptr;
    }

    bool ExecCUD(StudentDB* db) override {
        // ����������Ҫ�޸����ݿ�
        // ֻҪ�����л��ɹ��ͷ���true
        return true;
    }
};