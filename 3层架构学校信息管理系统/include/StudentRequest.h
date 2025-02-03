#pragma once 

#include "IRequest.h"
#include <string>

// ѧ���������
class StudentRequest : public IRequest {
protected:
    int msg_type_;
    int student_id_;
    int class_id_;
    int name_len_;
    std::string name_;

public:
    StudentRequest(int type) : 
        msg_type_(type), 
        student_id_(-1), 
        class_id_(-1), 
        name_len_(0) {}

    // ������Ϣ����
    void SetStudentId(int id) { student_id_ = id; }
    void SetClassId(int id) { class_id_ = id; }
    void SetName(const std::string& name) { 
        name_ = name; 
        name_len_ = name_.length();
    }

    // get ����
    int GetStudentId() const { return student_id_; }
    int GetClassId() const { return class_id_; }
    const std::string& GetName() const { return name_; }
    int GetNameLength() const { return name_len_; }

    bool BindUIData(const CString& str1, const CString& str2, const CString& str3) override {
        if (!str1.IsEmpty()) student_id_ = _ttoi(str1);
        if (!str2.IsEmpty()) name_ = std::string(CStringA(str2));
        if (!str3.IsEmpty()) class_id_ = _ttoi(str3);
        name_len_ = name_.length();

        TRACE("[��] ѧ��:%d, ����:%s, �༶:%d\n",
            student_id_, name_.c_str(), class_id_);
        return true;
    }

    // ʵ�ֻ���ӿ�
    int GetType() const override { return msg_type_; }
    
    int GetSize() const override {
        return sizeof(int) * 4 + name_len_;
    }

    bool isValidate() const override {
        return true;  // ����Ĭ�Ϸ���true�������������������֤����
    }

    bool Serialize(char* buffer, int bufferSize) const override {
        printf("[���л�] ����:%d, ѧ��:%d, �༶:%d, ����:%s\n",
            msg_type_, student_id_, class_id_, name_.c_str());

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
        memcpy(p, &class_id_, sizeof(int));
        p += sizeof(int);
        memcpy(p, &name_len_, sizeof(int));
        p += sizeof(int);
        memcpy(p, name_.c_str(), name_len_);
        
        printf("[���л�] �ɹ� - �ܴ�С:%d\n", GetSize());
        return true;
    }

    bool Deserialize(const char* buffer, int len) override {
        printf("[�����л�] ��ʼ������Ϣ������:%d\n", len);

        if (len < sizeof(int) * 4) {  // 4��int�ֶ�
            printf("[�����л�] ʧ�� - ���ݳ��Ȳ���\n");
            return false;
        }
        
        const char* p = buffer;
        memcpy(&msg_type_, p, sizeof(int));
        p += sizeof(int);
        memcpy(&student_id_, p, sizeof(int));
        p += sizeof(int);
        memcpy(&class_id_, p, sizeof(int));
        p += sizeof(int);
        memcpy(&name_len_, p, sizeof(int));
        p += sizeof(int);
        
        if (len < GetSize()) {
            printf("[�����л�] ʧ�� - ���ݳ��Ȳ��㣬��Ҫ%d\n", GetSize());
            return false;
        }

        name_.assign(p, name_len_);
        
        printf("[�����л�] �ɹ� - ����:%d, ѧ��:%d, �༶:%d, ����:%s\n",
            msg_type_, student_id_, class_id_, name_.c_str());
        return true;
    }

};

// ���ѧ������
class AddStudentRequest : public StudentRequest {
public:
    AddStudentRequest() : StudentRequest(MSG_ADD_STUDENT) {}

    // ��֤��������
    bool isValidate() const {
        return student_id_ > 0 && 
               class_id_ > 0 && 
               !name_.empty() && 
               name_.length() <= 50;
    }
};

// ɾ��ѧ������
class DeleteStudentRequest : public StudentRequest {
public:
    DeleteStudentRequest() : StudentRequest(MSG_DEL_STUDENT) {}

    bool isValidate() const {
        return student_id_ > 0;  // ֻ��Ҫѧ��
    }
};

// �޸�ѧ������
class ModifyStudentRequest : public StudentRequest {
public:
    ModifyStudentRequest() : StudentRequest(MSG_MOD_STUDENT) {}

    bool isValidate() const {
        return student_id_ > 0 &&  // ������ѧ��
               (class_id_ > 0 || !name_.empty());  // �����޸�һ��
    }
};

// ��ѯѧ������
class QueryStudentRequest : public StudentRequest {
public:
    QueryStudentRequest() : StudentRequest(MSG_QUERY_STUDENT) {}

    bool isValidate() const {
        return true;  // ���в������ǿ�ѡ��
    }

    // �Ƿ�ѧ�Ų�ѯ
    bool HasStudentId() const { return student_id_ > 0; }
    
    // �Ƿ񰴰༶��ѯ
    bool HasClassId() const { return class_id_ > 0; }
    
    // �Ƿ�������ѯ
    bool HasName() const { return !name_.empty(); }
};