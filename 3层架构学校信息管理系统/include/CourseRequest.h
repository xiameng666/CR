// CourseRequest.h
#pragma once
#include "IRequest.h"
#include <string>

class CourseRequest : public IRequest {
protected:
    int msg_type_;
    int course_id_;
    int credit_;
    int name_len_;
    std::string course_name_;

public:
    CourseRequest(int type) : 
        msg_type_(type), 
        course_id_(-1),
        credit_(-1),
        name_len_(0) {}

    void SetCourseId(int id) { course_id_ = id; }
    void SetCredit(int credit) { credit_ = credit; }
    void SetCourseName(const std::string& name) {
        course_name_ = name;
        name_len_ = course_name_.length();
    }

    int GetCourseId() const { return course_id_; }
    int GetCredit() const { return credit_; }
    const std::string& GetCourseName() const { return course_name_; }

    // IRequest�ӿ�ʵ��
    int GetType() const override { return msg_type_; }
    int GetSize() const override { return sizeof(int) * 4 + name_len_; }

    bool isValidate() const override {
        return true;  // ����Ĭ�Ϸ���true�������������������֤����
    }

    bool BindUIData(const CString& str1, const CString& str2, const CString& str3) override {
        if (!str1.IsEmpty()) course_id_ = _ttoi(str1);
        if (!str2.IsEmpty()) course_name_ = std::string(CStringA(str2));
        if (!str3.IsEmpty()) credit_ = _ttoi(str3);
        name_len_ = course_name_.length();
        return true;
    }

    bool Serialize(char* buffer, int bufferSize) const override{
        printf("[���л�] ����:%d, �γ�ID:%d, �γ�����:%s, ѧ��:%d\n",
        msg_type_, course_id_, course_name_.c_str(), credit_);

    if (bufferSize < GetSize()) {
        printf("[���л�] ʧ�� - ������̫С: ��Ҫ%d, ʵ��%d\n", 
            GetSize(), bufferSize);
        return false;
    }
    
    char* p = buffer;
    // д����Ϣ����
    memcpy(p, &msg_type_, sizeof(int));
    p += sizeof(int);
    // д��γ�ID
    memcpy(p, &course_id_, sizeof(int));
    p += sizeof(int);
    // д��ѧ��
    memcpy(p, &credit_, sizeof(int));
    p += sizeof(int);
    // д�����Ƴ���
    memcpy(p, &name_len_, sizeof(int));
    p += sizeof(int);
    // д����������
    memcpy(p, course_name_.c_str(), name_len_);
    
    printf("[���л�] �ɹ� - �ܴ�С:%d\n", GetSize());
    return true;
    }

    bool Deserialize(const char* buffer, int len) override{
    printf("[�����л�] ��ʼ������Ϣ������:%d\n", len);

    if (len < sizeof(int) * 4) {  // 4��int�ֶ�
        printf("[�����л�] ʧ�� - ���ݳ��Ȳ���\n");
        return false;
    }
    
    const char* p = buffer;
    // ��ȡ��Ϣ����
    memcpy(&msg_type_, p, sizeof(int));
    p += sizeof(int);
    // ��ȡ�γ�ID
    memcpy(&course_id_, p, sizeof(int));
    p += sizeof(int);
    // ��ȡѧ��
    memcpy(&credit_, p, sizeof(int));
    p += sizeof(int);
    // ��ȡ���Ƴ���
    memcpy(&name_len_, p, sizeof(int));
    p += sizeof(int);
    
    if (len < GetSize()) {
        printf("[�����л�] ʧ�� - ���ݳ��Ȳ��㣬��Ҫ%d\n", GetSize());
        return false;
    }

    // ��ȡ��������
    course_name_.assign(p, name_len_);
    
    printf("[�����л�] �ɹ� - ����:%d, �γ�ID:%d, �γ�����:%s, ѧ��:%d\n",
        msg_type_, course_id_, course_name_.c_str(), credit_);
    return true;

    }
};

// ������
class AddCourseRequest : public CourseRequest {
public:
    AddCourseRequest() : CourseRequest(MSG_ADD_COURSE) {}
    bool isValidate() const {
        return course_id_ > 0 && credit_ > 0 && !course_name_.empty();
    }
};

class DeleteCourseRequest : public CourseRequest {
public:
    DeleteCourseRequest() : CourseRequest(MSG_DEL_COURSE) {}
    bool isValidate() const { return course_id_ > 0; }
};

class ModifyCourseRequest : public CourseRequest {
public:
    ModifyCourseRequest() : CourseRequest(MSG_MOD_COURSE) {}
    bool isValidate() const {
        return course_id_ > 0 && (credit_ > 0 || !course_name_.empty());
    }
};

class QueryCourseRequest : public CourseRequest {
public:
    QueryCourseRequest() : CourseRequest(MSG_QUERY_COURSE) {}
    bool isValidate() const { return true; }
    bool HasCourseName() const { return !course_name_.empty(); }
};