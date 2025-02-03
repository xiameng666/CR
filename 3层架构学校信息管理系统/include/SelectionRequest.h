// SelectionRequest.h
#pragma once
#include "IRequest.h"

class SelectionRequest : public IRequest {
protected:
    int msg_type_;
    int student_id_;
    int course_id_;

public:
    SelectionRequest(int type) : 
        msg_type_(type),
        student_id_(-1),
        course_id_(-1) {}

    void SetStudentId(int id) { student_id_ = id; }
    void SetCourseId(int id) { course_id_ = id; }

    int GetStudentId() const { return student_id_; }
    int GetCourseId() const { return course_id_; }

    // IRequest�ӿ�ʵ��
    int GetType() const override { return msg_type_; }
    int GetSize() const override { return sizeof(int) * 3; }
    
    bool isValidate() const override {
        return true;  // ����Ĭ�Ϸ���true�������������������֤����
    }

    bool BindUIData(const CString& str1, const CString& str2, const CString& str3) override {
        if (!str1.IsEmpty()) student_id_ = _ttoi(str1);
        if (!str2.IsEmpty()) course_id_ = _ttoi(str2);
        return true;  // str3��ʹ��
    }

    bool Serialize(char* buffer, int bufferSize) const override {
        printf("[���л�] ����:%d, ѧ��:%d, �γ�ID:%d\n",
            msg_type_, student_id_, course_id_);

        if (bufferSize < GetSize()) {
            printf("[���л�] ʧ�� - ������̫С: ��Ҫ%d, ʵ��%d\n", 
                GetSize(), bufferSize);
            return false;
        }
        
        char* p = buffer;
        memcpy(p, &msg_type_, sizeof(int));
        p += sizeof(int);
        memcpy(p, &student_id_, sizeof(int));
        p += sizeof(int);
        memcpy(p, &course_id_, sizeof(int));
        
        printf("[���л�] �ɹ� - �ܴ�С:%d\n", GetSize());
        return true;
    }

    bool Deserialize(const char* buffer, int len) override {
        printf("[�����л�] ��ʼ������Ϣ������:%d\n", len);

        if (len < GetSize()) {
            printf("[�����л�] ʧ�� - ���ݳ��Ȳ���\n");
            return false;
        }
        
        const char* p = buffer;
        memcpy(&msg_type_, p, sizeof(int));
        p += sizeof(int);
        memcpy(&student_id_, p, sizeof(int));
        p += sizeof(int);
        memcpy(&course_id_, p, sizeof(int));
        
        printf("[�����л�] �ɹ� - ����:%d, ѧ��:%d, �γ�ID:%d\n",
            msg_type_, student_id_, course_id_);
        return true;
    }
};

// ѡ������
class SelectCourseRequest : public SelectionRequest {
public:
    SelectCourseRequest() : SelectionRequest(MSG_SELECT_COURSE) {}
    bool isValidate() const {
        return student_id_ > 0 && course_id_ > 0;
    }
};

// �˿�����
class DropCourseRequest : public SelectionRequest {
public:
    DropCourseRequest() : SelectionRequest(MSG_DROP_COURSE) {}
    bool isValidate() const {
        return student_id_ > 0 && course_id_ > 0;
    }
};

// ��ѯѡ������
class QuerySelectionRequest : public SelectionRequest {
public:
    QuerySelectionRequest() : SelectionRequest(MSG_QUERY_SELECTION) {}
    bool isValidate() const {
        return true;

     //   return student_id_ > 0;  // ֻ��Ҫѧ�ż��ɲ�ѯ��ѧ��������ѡ��
    }
};