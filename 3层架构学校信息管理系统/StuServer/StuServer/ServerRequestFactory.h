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
    // ��̬����������������Ϣ���ʹ�����Ӧ��������� TODO

    static IServerRequest* CreateRequest(int msgType) {
        switch (msgType) {
            // ��¼����
        case MSG_ADMIN_LOGIN:
            return new ServerLoginRequest();

            // ѧ���������
        case MSG_QUERY_STUDENT:
        case MSG_ADD_STUDENT:
        case MSG_DEL_STUDENT:
        case MSG_MOD_STUDENT:
            return new ServerStudentRequest(msgType);

            // �༶�������
        case MSG_QUERY_CLASS:
        case MSG_ADD_CLASS:
        case MSG_DEL_CLASS:
        case MSG_MOD_CLASS:
            return new ServerClassRequest(msgType);

            // �γ��������
        case MSG_QUERY_COURSE:
        case MSG_ADD_COURSE:
        case MSG_DEL_COURSE:
        case MSG_MOD_COURSE:
            return new ServerCourseRequest(msgType);

            // ѡ���������
        case MSG_QUERY_SELECTION:
        case MSG_SELECT_COURSE:
        case MSG_DROP_COURSE:
            return new ServerSelectionRequest(msgType);
            
        case MSG_HEARTBEAT:
            return new ServerHeartbeatRequest(msgType);
        default:
            TRACE("[����] δ֪����Ϣ����:%d\n", msgType);
            return nullptr;
        }
    }
};