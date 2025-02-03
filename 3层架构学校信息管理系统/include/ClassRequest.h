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

    // IRequest接口实现
    int GetType() const override { return msg_type_; }
    int GetSize() const override { return sizeof(int) * 3 + name_len_; }

    bool isValidate() const override {
        return true;  // 基类默认返回true，让派生类决定具体验证规则
    }

    bool BindUIData(const CString& str1, const CString& str2, const CString& str3) override {
        if (!str1.IsEmpty()) class_id_ = _ttoi(str1);
        if (!str2.IsEmpty()) class_name_ = std::string(CStringA(str2));
        name_len_ = class_name_.length();
        return true;  // str3不使用
    }

    bool Serialize(char* buffer, int bufferSize) const override{
        printf("[序列化] 类型:%d, 班级ID:%d, 班级名称:%s\n",
        msg_type_, class_id_, class_name_.c_str());

    if (bufferSize < GetSize()) {
        printf("[序列化] 失败 - 缓冲区太小: 需要%d, 实际%d\n", 
            GetSize(), bufferSize);
        return false;
    }
    
    char* p = buffer;
    // 写入消息类型
    memcpy(p, &msg_type_, sizeof(int));
    p += sizeof(int);
    // 写入班级ID
    memcpy(p, &class_id_, sizeof(int));
    p += sizeof(int);
    // 写入名称长度
    memcpy(p, &name_len_, sizeof(int));
    p += sizeof(int);
    // 写入名称内容
    memcpy(p, class_name_.c_str(), name_len_);
    
    printf("[序列化] 成功 - 总大小:%d\n", GetSize());
    return true;
    }

    bool Deserialize(const char* buffer, int len) override{
    printf("[反序列化] 开始解析消息，长度:%d\n", len);

    if (len < sizeof(int) * 3) {  // 3个int字段
        printf("[反序列化] 失败 - 数据长度不足\n");
        return false;
    }
    
    const char* p = buffer;
    // 读取消息类型
    memcpy(&msg_type_, p, sizeof(int));
    p += sizeof(int);
    // 读取班级ID
    memcpy(&class_id_, p, sizeof(int));
    p += sizeof(int);
    // 读取名称长度
    memcpy(&name_len_, p, sizeof(int));
    p += sizeof(int);
    
    if (len < GetSize()) {
        printf("[反序列化] 失败 - 数据长度不足，需要%d\n", GetSize());
        return false;
    }

    // 读取名称内容
    class_name_.assign(p, name_len_);
    
    printf("[反序列化] 成功 - 类型:%d, 班级ID:%d, 班级名称:%s\n",
        msg_type_, class_id_, class_name_.c_str());
    return true;
    }

    
};

// 派生类
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