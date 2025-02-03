#pragma once
#include "IServerRequest.h"

// 服务器端登录请求
class ServerLoginRequest : public IServerRequest {
private:
    LoginRequest* request_;   // 组合

public:

    ServerLoginRequest() {
        request_ = new LoginRequest();
    }

    ~ServerLoginRequest() {
        delete request_;
    }

    // 转发服务器需要用的方法
    bool Deserialize(const char* buffer, int len) {
        return request_->Deserialize(buffer, len);
    }

    // 实现服务器端接口
    MYSQL_RES* ExecR(StudentDB* db) override { return nullptr; }//登录不用这个

    bool ExecCUD(StudentDB* db) override {
        return db->AdminLogin(request_->GetUsername().c_str(),
            request_->GetPassword().c_str());
    }
};