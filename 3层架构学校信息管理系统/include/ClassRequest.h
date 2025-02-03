// ClassRequest.h
#pragma once
#include "IRequest.h"
#include <string>

class ClassRequest : public IRequest {
protected:
    int msg_type_;
    int class_id_;
    int name_len_;
    std::string class_name_;

public:
    ClassRequest(int type) : 
        msg_type_(type), 
        class_id_(-1), 
        name_len_(0) {}

    void SetClassId(int id) { class_id_ = id; }
    void SetClassName(const std::string& name) {
        class_name_ = name;
        name_len_ = class_name_.length();
    }

    int GetClassId() const { return class_id_; }
    const std::string& GetClassName() const { return class_name_; }

    // IRequest�ӿ�ʵ��
    int GetType() const override { return msg_type_; }
    int GetSize() const override { return sizeof(int) * 3 + name_len_; }

    bool isValidate() const override {
        return true;  // ����Ĭ�Ϸ���true�������������������֤����
    }

    bool BindUIData(const CString& str1, const CString& str2, const CString& str3) override {
        if (!str1.IsEmpty()) class_id_ = _ttoi(str1);
        if (!str2.IsEmpty()) class_name_ = std::string(CStringA(str2));
        name_len_ = class_name_.length();
        return true;  // str3��ʹ��
    }

    bool Serialize(char* buffer, int bufferSize) const override{
        printf("[���л�] ����:%d, �༶ID:%d, �༶����:%s\n",
        msg_type_, class_id_, class_name_.c_str());

    if (bufferSize < GetSize()) {
        printf("[���л�] ʧ�� - ������̫С: ��Ҫ%d, ʵ��%d\n", 
            GetSize(), bufferSize);
        return false;
    }
    
    char* p = buffer;
    // д����Ϣ����
    memcpy(p, &msg_type_, sizeof(int));
    p += sizeof(int);
    // д��༶ID
    memcpy(p, &class_id_, sizeof(int));
    p += sizeof(int);
    // д�����Ƴ���
    memcpy(p, &name_len_, sizeof(int));
    p += sizeof(int);
    // д����������
    memcpy(p, class_name_.c_str(), name_len_);
    
    printf("[���л�] �ɹ� - �ܴ�С:%d\n", GetSize());
    return true;
    }

    bool Deserialize(const char* buffer, int len) override{
    printf("[�����л�] ��ʼ������Ϣ������:%d\n", len);

    if (len < sizeof(int) * 3) {  // 3��int�ֶ�
        printf("[�����л�] ʧ�� - ���ݳ��Ȳ���\n");
        return false;
    }
    
    const char* p = buffer;
    // ��ȡ��Ϣ����
    memcpy(&msg_type_, p, sizeof(int));
    p += sizeof(int);
    // ��ȡ�༶ID
    memcpy(&class_id_, p, sizeof(int));
    p += sizeof(int);
    // ��ȡ���Ƴ���
    memcpy(&name_len_, p, sizeof(int));
    p += sizeof(int);
    
    if (len < GetSize()) {
        printf("[�����л�] ʧ�� - ���ݳ��Ȳ��㣬��Ҫ%d\n", GetSize());
        return false;
    }

    // ��ȡ��������
    class_name_.assign(p, name_len_);
    
    printf("[�����л�] �ɹ� - ����:%d, �༶ID:%d, �༶����:%s\n",
        msg_type_, class_id_, class_name_.c_str());
    return true;
    }

    
};

// ������
class AddClassRequest : public ClassRequest {
public:
    AddClassRequest() : ClassRequest(MSG_ADD_CLASS) {}
    bool isValidate() const {
        return class_id_ > 0 && !class_name_.empty();
    }
};

class DeleteClassRequest : public ClassRequest {
public:
    DeleteClassRequest() : ClassRequest(MSG_DEL_CLASS) {}
    bool isValidate() const { return class_id_ > 0; }
};

class ModifyClassRequest : public ClassRequest {
public:
    ModifyClassRequest() : ClassRequest(MSG_MOD_CLASS) {}
    bool isValidate() const {
        return class_id_ > 0 && !class_name_.empty();
    }
};

class QueryClassRequest : public ClassRequest {
public:
    QueryClassRequest() : ClassRequest(MSG_QUERY_CLASS) {}
    bool isValidate() const { return true; }
    bool HasClassName() const { return !class_name_.empty(); }
};