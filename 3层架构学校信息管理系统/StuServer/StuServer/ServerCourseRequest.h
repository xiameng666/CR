// ServerCourseRequest.h
#pragma once
#include "IServerRequest.h"

class ServerCourseRequest : public IServerRequest {
private:
    CourseRequest* request_;

public:
    ServerCourseRequest(int msgType) {
        request_ = new CourseRequest(msgType);
    }

    ~ServerCourseRequest() {
        delete request_;
    }

    bool Deserialize(const char* buffer, int len) override {
        return request_->Deserialize(buffer, len);
    }

    MYSQL_RES* ExecR(StudentDB* db) override {
        if (request_->GetType() == MSG_QUERY_COURSE) {
            return db->QueryCourse(request_->GetCourseName().c_str());
        }
        return nullptr;
    }

    bool ExecCUD(StudentDB* db) override {
        switch (request_->GetType()) {
        case MSG_ADD_COURSE:
            return db->AddCourse(
                request_->GetCourseId(),
                request_->GetCourseName().c_str(),
                request_->GetCredit()
            );
        case MSG_DEL_COURSE:
            return db->DeleteCourse(request_->GetCourseId());
        case MSG_MOD_COURSE:
            return db->ModifyCourse(
                request_->GetCourseId(),
                request_->GetCourseName().c_str(),
                request_->GetCredit()
            );
        default:
            return false;
        }
    }
};