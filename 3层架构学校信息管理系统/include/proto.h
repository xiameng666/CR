#pragma once
#include <vector>
#include <memory>


#define WM_SOCKET (WM_USER+100)

// 存储 查询结果的数据结构
struct QueryResult{
    int rows;       // 多少行
    int cols;       // 多少列
    char data[];   // 柔性数组，存储格式: [长度][字符串][长度][字符串]...
};

static_assert(sizeof(QueryResult) == 8,
    "QueryResult size must be 8 bytes");

// 请求消息结构 C->S
struct RequestMsg {
    int msgType;
    char data[];    // 根据msgType确定数据格式
};

// 响应消息结构 S->C
struct ResponseMsg {
    int msgType;    
    int rspType;
    int result;     // 0成功，非0失败，-1连接数据库失败
    char errMsg[100];
    QueryResult queryResult;  // 查询结果（如果是查询操作）
};

enum ResponseType {
    RSP_HEARTBEAT = 0,

    RSP_LOGIN = 1,        // 登录响应（需要切换窗口）
    RSP_NO_RESULT = 2,    // 无结果集响应（增删改操作，只需显示成功/失败）
    RSP_HAS_RESULT = 3    // 有结果集响应（查询操作，需要更新列表）

};

// 业务命令类型及对应参数格式
enum MSG_TYPE {
    
    // 班级操作
    MSG_ADD_CLASS = 1,      // int classId, char name[50]
    MSG_DEL_CLASS = 2,      // int classId
    MSG_MOD_CLASS = 3,      // int classId, char name[50]
    MSG_QUERY_CLASS = 4,    // char name[50] (可选，空表示查询所有)

    // 学生操作
    MSG_ADD_STUDENT = 11,   // int studentId, char name[50], int classId
    MSG_DEL_STUDENT = 12,   // int studentId
    MSG_MOD_STUDENT = 13,   // int studentId, char name[50], int classId
    MSG_QUERY_STUDENT = 14, // char name[50], int classId (都可选)

    // 课程操作
    MSG_ADD_COURSE = 21,    // int courseId, char name[50], int credit
    MSG_DEL_COURSE = 22,    // int courseId
    MSG_MOD_COURSE = 23,    // int courseId, char name[50], int credit
    MSG_QUERY_COURSE = 24,  // char name[50] (可选)

    // 选课操作
    MSG_SELECT_COURSE = 31, // int studentId, int courseId
    MSG_DROP_COURSE = 32,   // int studentId, int courseId
    MSG_QUERY_SELECTION = 33,// int studentId (查询学生选课) 或 int courseId (查询课程学生)

    //登录
    MSG_ADMIN_LOGIN = 40,    // 管理员登录: char username[50], char password[50]

    //心跳包
    MSG_HEARTBEAT = 50
};

