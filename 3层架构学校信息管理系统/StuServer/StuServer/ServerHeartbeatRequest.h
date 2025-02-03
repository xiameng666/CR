// ServerHeartbeatRequest.h
#pragma once
#include "IServerRequest.h"

// 服务器端心跳请求
class ServerHeartbeatRequest : public IServerRequest {
private:
    HeartbeatRequest* request_;  // 组合

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
        // 心跳包不需要查询数据库
        return nullptr;
    }

    bool ExecCUD(StudentDB* db) override {
        // 心跳包不需要修改数据库
        // 只要反序列化成功就返回true
        return true;
    }
};