#pragma once
#include "IServerRequest.h"

// 服务器端学生请求
class ServerStudentRequest : public IServerRequest {
private:
    StudentRequest* request_;  // 组合

public:
    ServerStudentRequest(int msgType) {
        request_ = new StudentRequest(msgType);
    }

    ~ServerStudentRequest() {
        delete request_;
    }

    bool Deserialize(const char* buffer, int len) {
        return request_->Deserialize(buffer, len);
    }

    MYSQL_RES* ExecR(StudentDB* db) override {
        if (request_->GetType() == MSG_QUERY_STUDENT) {
            return db->QueryStudent(
                request_->GetStudentId(),
                request_->GetName().c_str(),
                request_->GetClassId()
            );
        }
        return nullptr;
    }

    bool ExecCUD(StudentDB* db) override {
        switch (request_->GetType()) {
        case MSG_ADD_STUDENT:
            return db->AddStudent(
                request_->GetStudentId(),
                request_->GetName().c_str(),
                request_->GetClassId()
            );
        case MSG_DEL_STUDENT:
            return db->DeleteStudent(request_->GetStudentId());
        case MSG_MOD_STUDENT:
            return db->ModifyStudent(
                request_->GetStudentId(),
                request_->GetName().c_str(),
                request_->GetClassId()
            );
        default:
            return false;
        }
    }
};