#pragma once
#include <vector>
#include <memory>


#define WM_SOCKET (WM_USER+100)

// �洢 ��ѯ��������ݽṹ
struct QueryResult{
    int rows;       // ������
    int cols;       // ������
    char data[];   // �������飬�洢��ʽ: [����][�ַ���][����][�ַ���]...
};

static_assert(sizeof(QueryResult) == 8,
    "QueryResult size must be 8 bytes");

// ������Ϣ�ṹ C->S
struct RequestMsg {
    int msgType;
    char data[];    // ����msgTypeȷ�����ݸ�ʽ
};

// ��Ӧ��Ϣ�ṹ S->C
struct ResponseMsg {
    int msgType;    
    int rspType;
    int result;     // 0�ɹ�����0ʧ�ܣ�-1�������ݿ�ʧ��
    char errMsg[100];
    QueryResult queryResult;  // ��ѯ���������ǲ�ѯ������
};

enum ResponseType {
    RSP_HEARTBEAT = 0,

    RSP_LOGIN = 1,        // ��¼��Ӧ����Ҫ�л����ڣ�
    RSP_NO_RESULT = 2,    // �޽������Ӧ����ɾ�Ĳ�����ֻ����ʾ�ɹ�/ʧ�ܣ�
    RSP_HAS_RESULT = 3    // �н������Ӧ����ѯ��������Ҫ�����б�

};

// ҵ���������ͼ���Ӧ������ʽ
enum MSG_TYPE {
    
    // �༶����
    MSG_ADD_CLASS = 1,      // int classId, char name[50]
    MSG_DEL_CLASS = 2,      // int classId
    MSG_MOD_CLASS = 3,      // int classId, char name[50]
    MSG_QUERY_CLASS = 4,    // char name[50] (��ѡ���ձ�ʾ��ѯ����)

    // ѧ������
    MSG_ADD_STUDENT = 11,   // int studentId, char name[50], int classId
    MSG_DEL_STUDENT = 12,   // int studentId
    MSG_MOD_STUDENT = 13,   // int studentId, char name[50], int classId
    MSG_QUERY_STUDENT = 14, // char name[50], int classId (����ѡ)

    // �γ̲���
    MSG_ADD_COURSE = 21,    // int courseId, char name[50], int credit
    MSG_DEL_COURSE = 22,    // int courseId
    MSG_MOD_COURSE = 23,    // int courseId, char name[50], int credit
    MSG_QUERY_COURSE = 24,  // char name[50] (��ѡ)

    // ѡ�β���
    MSG_SELECT_COURSE = 31, // int studentId, int courseId
    MSG_DROP_COURSE = 32,   // int studentId, int courseId
    MSG_QUERY_SELECTION = 33,// int studentId (��ѯѧ��ѡ��) �� int courseId (��ѯ�γ�ѧ��)

    //��¼
    MSG_ADMIN_LOGIN = 40,    // ����Ա��¼: char username[50], char password[50]

    //������
    MSG_HEARTBEAT = 50
};

