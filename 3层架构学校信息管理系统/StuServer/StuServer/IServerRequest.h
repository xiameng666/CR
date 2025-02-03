#pragma once
#include "StudentDB.h"
#include "IRequest.h"

#include "LoginRequest.h"
#include "ClassRequest.h"
#include "CourseRequest.h"
#include "SelectionRequest.h"
#include "StudentRequest.h"
#include "HeartbeatRequest.h"

//需要组合请求实体   转发请求实体的Deserialize方法

// 服务器端请求接口 派生类负责执行CRUD
class IServerRequest {
public:
    virtual ~IServerRequest() = default;

    //查
    virtual MYSQL_RES* ExecR(StudentDB* db) = 0;

    //增删改
    virtual bool ExecCUD(StudentDB* db) = 0;

    virtual bool Deserialize(const char* buffer, int len) = 0;
};

