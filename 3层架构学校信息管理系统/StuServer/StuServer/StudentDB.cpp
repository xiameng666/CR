#define _CRT_SECURE_NO_WARNINGS
#include "StudentDB.h"
#include <proto.h>

StudentDB* StudentDB::m_instance = nullptr;//��һ��StudentDB::GetInstance();ʱ�Զ�init�������ݿ�

StudentDB* StudentDB::GetInstance() {
    if (!m_instance) {
        m_instance = new StudentDB();
        if (!m_instance->Init()) {
            delete m_instance;
            m_instance = nullptr;
        }
    }
    return m_instance;
}

void StudentDB::ReleaseInstance() {
    if (m_instance) {
        delete m_instance;
        m_instance = nullptr;
    }
}

bool StudentDB::Init() {
    TRACE("[DB]��ʼ��ʼ�����ݿ�����\n");
    unsigned int ssl_mode = SSL_MODE_DISABLED;
    mysql_options(m_mysql, MYSQL_OPT_SSL_MODE, &ssl_mode);

    // �������״̬
    if (mysql_ping(m_mysql) == 0) {
        TRACE("[DB]ERROR: MySQL�Ѿ�����\n");
        return false;
    }

    if (!mysql_real_connect(m_mysql, "localhost", "root", "123456", nullptr, 3306, NULL, 0)) {
        char szBuf[1024];
        sprintf_s(szBuf, sizeof(szBuf), "mysql_real_connect Error:%s\n", mysql_error(m_mysql));
        printf(szBuf);

        //CString strError;
        //strError.Format(_T("�������ݿ�ʧ�ܣ�%s"), CString(mysql_error(m_mysql)));
        //AfxMessageBox(strError, MB_ICONERROR);

        return false;
    }

    TRACE("[DB]���ӳɹ��������ַ���\n");
    mysql_set_character_set(m_mysql, "gbk");

    if (!InitDBandTable()) {
        TRACE("[DB]ERROR: !InitDBandTable\n");
        return false;
    }

    if (!InitTestData()) {
        TRACE("[DB]ERROR: !InitTestData\n");
        return false;
    }

    return true;
}

bool StudentDB::InitDBandTable() {
    // ������ʹ�����ݿ�
    if (!query_void("CREATE DATABASE IF NOT EXISTS student_db") ||
        !query_void("USE student_db")) {
        return false;
    }

    // ��������Ա��
    if (!query_void("CREATE TABLE IF NOT EXISTS admin ("
        "admin_id INT PRIMARY KEY AUTO_INCREMENT,"
        "username VARCHAR(50) NOT NULL UNIQUE,"
        "password VARCHAR(50) NOT NULL"
        ")")) {
        return false;
    }

    // �����༶��
    if (!query_void("CREATE TABLE IF NOT EXISTS class ("
        "class_id INT PRIMARY KEY,"
        "class_name VARCHAR(50) NOT NULL"
        ")")) {
        return false;
    }

    // ����ѧ����  ɾ���༶ʱ����ѧ��class_idΪNULL
    if (!query_void("CREATE TABLE IF NOT EXISTS student ("
        "student_id INT PRIMARY KEY,"
        "name VARCHAR(50) NOT NULL,"
        "class_id INT,"
        "FOREIGN KEY (class_id) REFERENCES class(class_id)"
        "ON DELETE SET NULL"
        ")")) {
        return false;
    }

    // �����γ̱�
    if (!query_void("CREATE TABLE IF NOT EXISTS course ("
        "course_id INT PRIMARY KEY,"
        "course_name VARCHAR(50) NOT NULL,"
        "credit INT NOT NULL"
        ")")) {
        return false;
    }

    // ����ѡ�α�  ����ɾ��  ɾ��ѧ��/�γ̣��Զ�ɾ����ص�ѡ�μ�¼
    if (!query_void("CREATE TABLE IF NOT EXISTS selection ("
        "student_id INT,"
        "course_id INT,"
        "PRIMARY KEY (student_id, course_id),"
        "FOREIGN KEY (student_id) REFERENCES student(student_id)"
        "ON DELETE CASCADE,"
        "FOREIGN KEY (course_id) REFERENCES course(course_id)"
        "ON DELETE CASCADE"
        ")")) {
        return false;
    }

    return true;
}

bool StudentDB::InitTestData() {
    TRACE("[DB]��ʼ��ʼ����������\n");

    // ����Ĭ�Ϲ���Ա�˺�
    const char* admin_sql =
        "INSERT IGNORE INTO admin (username, password) VALUES ('admin', '123456')";

    // ����༶����
    const char* class_sql[] = {
        "INSERT IGNORE INTO class (class_id, class_name) VALUES (1, '�����1��')",
        "INSERT IGNORE INTO class (class_id, class_name) VALUES (2, '�����2��')",
        "INSERT IGNORE INTO class (class_id, class_name) VALUES (3, '�������1��')"
    };

    // ����ѧ������
    const char* student_sql[] = {
        "INSERT IGNORE INTO student (student_id, name, class_id) VALUES (1001, '����', 1)",
        "INSERT IGNORE INTO student (student_id, name, class_id) VALUES (1002, '����', 1)",
        "INSERT IGNORE INTO student (student_id, name, class_id) VALUES (1003, '����', 2)",
        "INSERT IGNORE INTO student (student_id, name, class_id) VALUES (1004, '����', 2)",
        "INSERT IGNORE INTO student (student_id, name, class_id) VALUES (1005, '����', 3)"
    };

    // ����γ�����
    const char* course_sql[] = {
        "INSERT IGNORE INTO course (course_id, course_name, credit) VALUES (101, 'C++�������', 4)",
        "INSERT IGNORE INTO course (course_id, course_name, credit) VALUES (102, '���ݽṹ', 4)",
        "INSERT IGNORE INTO course (course_id, course_name, credit) VALUES (103, '���������', 3)",
        "INSERT IGNORE INTO course (course_id, course_name, credit) VALUES (104, '����ϵͳ', 4)",
        "INSERT IGNORE INTO course (course_id, course_name, credit) VALUES (105, '���ݿ�ԭ��', 3)"
    };

    // ����ѡ������
    const char* selection_sql[] = {
        "INSERT IGNORE INTO selection (student_id, course_id) VALUES (1001, 101)",
        "INSERT IGNORE INTO selection (student_id, course_id) VALUES (1001, 102)",
        "INSERT IGNORE INTO selection (student_id, course_id) VALUES (1002, 101)",
        "INSERT IGNORE INTO selection (student_id, course_id) VALUES (1002, 103)",
        "INSERT IGNORE INTO selection (student_id, course_id) VALUES (1003, 102)",
        "INSERT IGNORE INTO selection (student_id, course_id) VALUES (1003, 104)",
        "INSERT IGNORE INTO selection (student_id, course_id) VALUES (1004, 101)",
        "INSERT IGNORE INTO selection (student_id, course_id) VALUES (1005, 105)"
    };

    // ִ�в���

    
    if (!query_void(admin_sql)) {
        return false;
    }

    for (const char* sql : class_sql) {
        if (!query_void(sql)) {
            TRACE("[DB]����༶����ʧ��: %s\n", sql);
            return false;
        }
    }

    for (const char* sql : student_sql) {
        if (!query_void(sql)) {
            TRACE("[DB]����ѧ������ʧ��: %s\n", sql);
            return false;
        }
    }

    for (const char* sql : course_sql) {
        if (!query_void(sql)) {
            TRACE("[DB]����γ�����ʧ��: %s\n", sql);
            return false;
        }
    }

    for (const char* sql : selection_sql) {
        if (!query_void(sql)) {
            TRACE("[DB]����ѡ������ʧ��: %s\n", sql);
            return false;
        }
    }

    TRACE("[DB]�������ݳ�ʼ�����\n");
    return true;
}

/////////////////////////sql///////////////////////
bool StudentDB::AdminLogin(const char* username, const char* password) {
    char sql[256];
    sprintf_s(sql,
        "SELECT * FROM admin WHERE username='%s' AND password='%s'",
        username, password);

    MYSQL_RES* result = query_return(sql);
    if (!result) return false;

    bool success = mysql_num_rows(result) > 0;
    mysql_free_result(result);
    return success;
}

// �༶����
bool StudentDB::AddClass(int classId, const char* className) {
    char sql[256];
    sprintf_s(sql, "INSERT INTO class (class_id, class_name) VALUES (%d, '%s')",
        classId, className);
    return query_void(sql);
}

bool StudentDB::DeleteClass(int classId) {
    char sql[256];
    sprintf_s(sql, "DELETE FROM class WHERE class_id = %d", classId);
    return query_void(sql);
}

bool StudentDB::ModifyClass(int classId, const char* className) {
    char sql[256];
    sprintf_s(sql, "UPDATE class SET class_name = '%s' WHERE class_id = %d",
        className, classId);
    return query_void(sql);
}

MYSQL_RES* StudentDB::QueryClass(const char* className) {
    char sql[256];
    if (className && strlen(className) > 0) {
        sprintf_s(sql, "SELECT * FROM class WHERE class_name LIKE '%%%s%%'", className);
    }
    else {
        strcpy_s(sql, "SELECT * FROM class");
    }
    return query_return(sql);
}

// ѧ������
bool StudentDB::AddStudent(int studentId, const char* name, int classId) {
    char sql[256];
    sprintf_s(sql, "INSERT INTO student (student_id, name, class_id) VALUES (%d, '%s', %d)",
        studentId, name, classId);
    return query_void(sql);
}

bool StudentDB::DeleteStudent(int studentId) {
    char sql[256];
    sprintf_s(sql, "DELETE FROM student WHERE student_id = %d", studentId);
    return query_void(sql);
}

bool StudentDB::ModifyStudent(int studentId, const char* name, int classId) {
    char sql[256];
    sprintf_s(sql, "UPDATE student SET name = '%s', class_id = %d WHERE student_id = %d",
        name, classId, studentId);
    return query_void(sql);
}

MYSQL_RES* StudentDB::QueryStudent(int studentId, const char* name, int classId) {
    char sql[512] = 
        "SELECT s.student_id, s.name, s.class_id, c.class_name "  
        "FROM student s "
        "LEFT JOIN class c ON s.class_id = c.class_id " 
        "WHERE 1=1";

    // ��ѧ�Ų�ѯ
    if (studentId > 0) {
        char temp[50];
        sprintf_s(temp, " AND s.student_id = %d", studentId);
        strcat_s(sql, temp);
    }

    // ������ģ����ѯ
    if (name && strlen(name) > 0) {
        char temp[100];
        sprintf_s(temp, " AND s.name LIKE '%%%s%%'", name);
        strcat_s(sql, temp);
    }

    // ���༶��ѯ
    if (classId > 0) {
        char temp[50];
        sprintf_s(temp, " AND s.class_id = %d", classId);
        strcat_s(sql, temp);
    }

    printf("[DB] ִ��SQL: %s\n", sql);
    return query_return(sql);
}

// �γ̲���
bool StudentDB::AddCourse(int courseId, const char* name, int credit) {
    char sql[256];
    sprintf_s(sql, "INSERT INTO course (course_id, course_name, credit) VALUES (%d, '%s', %d)",
        courseId, name, credit);
    return query_void(sql);
}

bool StudentDB::DeleteCourse(int courseId) {
    char sql[256];
    sprintf_s(sql, "DELETE FROM course WHERE course_id = %d", courseId);
    return query_void(sql);
}

bool StudentDB::ModifyCourse(int courseId, const char* name, int credit) {
    char sql[256];
    sprintf_s(sql, "UPDATE course SET course_name = '%s', credit = %d WHERE course_id = %d",
        name, credit, courseId);
    return query_void(sql);
}

MYSQL_RES* StudentDB::QueryCourse(const char* name) {
    char sql[256];
    if (name && strlen(name) > 0) {
        sprintf_s(sql, "SELECT * FROM course WHERE course_name LIKE '%%%s%%'", name);
    }
    else {
        strcpy_s(sql, "SELECT * FROM course");
    }
    return query_return(sql);
}

// ѡ�β���
bool StudentDB::SelectCourse(int studentId, int courseId) {
    char sql[256];
    sprintf_s(sql, "INSERT INTO selection (student_id, course_id) VALUES (%d, %d)",
        studentId, courseId);
    return query_void(sql);
}

bool StudentDB::DropCourse(int studentId, int courseId) {
    char sql[256];
    sprintf_s(sql, "DELETE FROM selection WHERE student_id = %d AND course_id = %d",
        studentId, courseId);
    return query_void(sql);
}

MYSQL_RES* StudentDB::QuerySelection(int studentId, int courseId) {
    char sql[256];
    char temp[128] = { 0 };
    sprintf_s(sql,
        "SELECT s.student_id, s.name, c.course_id, c.course_name, c.credit "
        "FROM selection sel "
        "JOIN student s ON sel.student_id = s.student_id "
        "JOIN course c ON sel.course_id = c.course_id "
        "WHERE 1=1");

    if (studentId != -1) {
        sprintf_s(temp, " AND sel.student_id = %d", studentId);
        strcat_s(sql, temp);
    }
    if (courseId != -1) {
        sprintf_s(temp, " AND sel.course_id = %d", courseId);
        strcat_s(sql, temp);
    }

    return query_return(sql);
}

bool StudentDB::ParseSqlResult(MYSQL_RES* result, char* outBuffer, int& dataSize)
{
    if (!result) return false;

    // ��ȡ��������д��ṹ
    QueryResult* qr = (QueryResult*)outBuffer;
    qr->rows = mysql_num_rows(result);
    qr->cols = mysql_num_fields(result);

    // ��ȡ����д��λ��
    char* pData = qr->data;

    // ���д�������
    MYSQL_ROW row;
    while (row = mysql_fetch_row(result)) {
        for (int i = 0; i < qr->cols; i++) {
            // ��ȡ�ֶ�ֵ��NULLתΪ"NULL"�ַ���
            const char* str = row[i] ? row[i] : "NULL";
            int len = strlen(str);

            // д�볤�Ⱥ�����
            *(int*)pData = len;
            pData += sizeof(int);
            memcpy(pData, str, len);
            pData += len;
        }
    }

    // ���������ݴ�С
    dataSize = pData - outBuffer;
    return true;
}