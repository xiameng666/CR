// ServerSelectionRequest.h
#pragma once
#include "IServerRequest.h"

class ServerSelectionRequest : public IServerRequest {
private:
    SelectionRequest* request_;

public:
    ServerSelectionRequest(int msgType) {
        request_ = new SelectionRequest(msgType);
    }

    ~ServerSelectionRequest() {
        delete request_;
    }

    bool Deserialize(const char* buffer, int len) override {
        return request_->Deserialize(buffer, len);
    }

    MYSQL_RES* ExecR(StudentDB* db) override {
        if (request_->GetType() == MSG_QUERY_SELECTION) {
            return db->QuerySelection(request_->GetStudentId(),request_->GetCourseId());
        }
        return nullptr;
    }

    bool ExecCUD(StudentDB* db) override {
        switch (request_->GetType()) {
        case MSG_SELECT_COURSE:
            return db->SelectCourse(
                request_->GetStudentId(),
                request_->GetCourseId()
            );
        case MSG_DROP_COURSE:
            return db->DropCourse(
                request_->GetStudentId(),
                request_->GetCourseId()
            );
        default:
            return false;
        }
    }
};