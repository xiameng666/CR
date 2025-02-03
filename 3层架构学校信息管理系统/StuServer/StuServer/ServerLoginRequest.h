#pragma once
#include "IServerRequest.h"

// �������˵�¼����
class ServerLoginRequest : public IServerRequest {
private:
    LoginRequest* request_;   // ���

public:

    ServerLoginRequest() {
        request_ = new LoginRequest();
    }

    ~ServerLoginRequest() {
        delete request_;
    }

    // ת����������Ҫ�õķ���
    bool Deserialize(const char* buffer, int len) {
        return request_->Deserialize(buffer, len);
    }

    // ʵ�ַ������˽ӿ�
    MYSQL_RES* ExecR(StudentDB* db) override { return nullptr; }//��¼�������

    bool ExecCUD(StudentDB* db) override {
        return db->AdminLogin(request_->GetUsername().c_str(),
            request_->GetPassword().c_str());
    }
};