#pragma once
#include"IDatabase.h"


class StudentDB : public IDatabase {
private:
    static StudentDB* m_instance;

    StudentDB() = default;
    ~StudentDB() = default;
    StudentDB(const StudentDB&) = delete;
    StudentDB& operator=(const StudentDB&) = delete;
public:
    static StudentDB* GetInstance();
    static void ReleaseInstance();
    virtual bool Init();
    bool InitDBandTable();
    bool InitTestData();

public://sql��ѯ
    bool AdminLogin(const char* username, const char* password);

    // �༶����
    bool AddClass(int classId, const char* className);
    bool DeleteClass(int classId);
    bool ModifyClass(int classId, const char* className);
    MYSQL_RES* QueryClass(const char* className = nullptr);

    // ѧ������
    bool AddStudent(int studentId, const char* name, int classId);
    bool DeleteStudent(int studentId);
    bool ModifyStudent(int studentId, const char* name, int classId);
    MYSQL_RES* QueryStudent(int studentId, const char* name, int classId);


    // �γ̲���
    bool AddCourse(int courseId, const char* name, int credit);
    bool DeleteCourse(int courseId);
    bool ModifyCourse(int courseId, const char* name, int credit);
    MYSQL_RES* QueryCourse(const char* name = nullptr);

    // ѡ�β���
    bool SelectCourse(int studentId, int courseId);
    bool DropCourse(int studentId, int courseId);
    MYSQL_RES* QuerySelection(int studentId, int courseId);
    /*MYSQL_RES* QuerySelection(int studentId);*/

    //��Ҫ��sql�������ݼ�ת��Ϊ������ �������ݺͳ���д�ز���
    bool ParseSqlResult(MYSQL_RES* result, char* outBuffer, int& dataSize);

};