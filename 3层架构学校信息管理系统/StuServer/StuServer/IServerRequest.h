#pragma once
#include "StudentDB.h"
#include "IRequest.h"

#include "LoginRequest.h"
#include "ClassRequest.h"
#include "CourseRequest.h"
#include "SelectionRequest.h"
#include "StudentRequest.h"
#include "HeartbeatRequest.h"

//��Ҫ�������ʵ��   ת������ʵ���Deserialize����

// ������������ӿ� �����ฺ��ִ��CRUD
class IServerRequest {
public:
    virtual ~IServerRequest() = default;

    //��
    virtual MYSQL_RES* ExecR(StudentDB* db) = 0;

    //��ɾ��
    virtual bool ExecCUD(StudentDB* db) = 0;

    virtual bool Deserialize(const char* buffer, int len) = 0;
};

