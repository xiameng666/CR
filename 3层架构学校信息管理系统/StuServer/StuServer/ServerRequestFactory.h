#pragma once
#include "proto.h"

#include "IServerRequest.h"

#include "ServerLoginRequest.h"
#include "ServerStudentRequest.h"
#include "ServerClassRequest.h"
#include "ServerCourseRequest.h"
#include "ServerSelectionRequest.h"
#include "ServerHeartbeatRequest.h"

class ServerRequestFactory {
public:
    // 静态工厂方法，根据消息类型创建对应的请求对象 TODO

    static IServerRequest* CreateRequest(int msgType) {
        switch (msgType) {
            // 登录请求
        case MSG_ADMIN_LOGIN:
            return new ServerLoginRequest();

            // 学生相关请求
        case MSG_QUERY_STUDENT:
        case MSG_ADD_STUDENT:
        case MSG_DEL_STUDENT:
        case MSG_MOD_STUDENT:
            return new ServerStudentRequest(msgType);

            // 班级相关请求
        case MSG_QUERY_CLASS:
        case MSG_ADD_CLASS:
        case MSG_DEL_CLASS:
        case MSG_MOD_CLASS:
            return new ServerClassRequest(msgType);

            // 课程相关请求
        case MSG_QUERY_COURSE:
        case MSG_ADD_COURSE:
        case MSG_DEL_COURSE:
        case MSG_MOD_COURSE:
            return new ServerCourseRequest(msgType);

            // 选课相关请求
        case MSG_QUERY_SELECTION:
        case MSG_SELECT_COURSE:
        case MSG_DROP_COURSE:
            return new ServerSelectionRequest(msgType);
            
        case MSG_HEARTBEAT:
            return new ServerHeartbeatRequest(msgType);
        default:
            TRACE("[工厂] 未知的消息类型:%d\n", msgType);
            return nullptr;
        }
    }
};