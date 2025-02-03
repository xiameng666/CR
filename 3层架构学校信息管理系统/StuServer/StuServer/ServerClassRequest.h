// ServerClassRequest.h
#pragma once
#include "IServerRequest.h"

class ServerClassRequest : public IServerRequest {
private:
    ClassRequest* request_;

public:
    ServerClassRequest(int msgType) {
        request_ = new ClassRequest(msgType);
    }

    ~ServerClassRequest() {
        delete request_;
    }

    bool Deserialize(const char* buffer, int len) override {
        return request_->Deserialize(buffer, len);
    }

    MYSQL_RES* ExecR(StudentDB* db) override {
        if (request_->GetType() == MSG_QUERY_CLASS) {
            return db->QueryClass(request_->GetClassName().c_str());
        }
        return nullptr;
    }

    bool ExecCUD(StudentDB* db) override {
        switch (request_->GetType()) {
        case MSG_ADD_CLASS:
            return db->AddClass(
                request_->GetClassId(),
                request_->GetClassName().c_str()
            );
        case MSG_DEL_CLASS:
            return db->DeleteClass(request_->GetClassId());
        case MSG_MOD_CLASS:
            return db->ModifyClass(
                request_->GetClassId(),
                request_->GetClassName().c_str()
            );
        default:
            return false;
        }
    }
};