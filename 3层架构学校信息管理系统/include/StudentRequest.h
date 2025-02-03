#pragma once 

#include "IRequest.h"
#include <string>

// 学生相关请求
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

    // 设置消息内容
    void SetStudentId(int id) { student_id_ = id; }
    void SetClassId(int id) { class_id_ = id; }
    void SetName(const std::string& name) { 
        name_ = name; 
        name_len_ = name_.length();
    }

    // get 方法
    int GetStudentId() const { return student_id_; }
    int GetClassId() const { return class_id_; }
    const std::string& GetName() const { return name_; }
    int GetNameLength() const { return name_len_; }

    bool BindUIData(const CString& str1, const CString& str2, const CString& str3) override {
        if (!str1.IsEmpty()) student_id_ = _ttoi(str1);
        if (!str2.IsEmpty()) name_ = std::string(CStringA(str2));
        if (!str3.IsEmpty()) class_id_ = _ttoi(str3);
        name_len_ = name_.length();

        TRACE("[绑定] 学号:%d, 姓名:%s, 班级:%d\n",
            student_id_, name_.c_str(), class_id_);
        return true;
    }

    // 实现基类接口
    int GetType() const override { return msg_type_; }
    
    int GetSize() const override {
        return sizeof(int) * 4 + name_len_;
    }

    bool isValidate() const override {
        return true;  // 基类默认返回true，让派生类决定具体验证规则
    }

    bool Serialize(char* buffer, int bufferSize) const override {
        printf("[序列化] 类型:%d, 学号:%d, 班级:%d, 姓名:%s\n",
            msg_type_, student_id_, class_id_, name_.c_str());

        if (bufferSize < GetSize()) {
            printf("[序列化] 失败 - 缓冲区太小: 需要%d, 实际%d\n", 
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
        
        printf("[序列化] 成功 - 总大小:%d\n", GetSize());
        return true;
    }

    bool Deserialize(const char* buffer, int len) override {
        printf("[反序列化] 开始解析消息，长度:%d\n", len);

        if (len < sizeof(int) * 4) {  // 4个int字段
            printf("[反序列化] 失败 - 数据长度不足\n");
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
            printf("[反序列化] 失败 - 数据长度不足，需要%d\n", GetSize());
            return false;
        }

        name_.assign(p, name_len_);
        
        printf("[反序列化] 成功 - 类型:%d, 学号:%d, 班级:%d, 姓名:%s\n",
            msg_type_, student_id_, class_id_, name_.c_str());
        return true;
    }

};

// 添加学生请求
class AddStudentRequest : public StudentRequest {
public:
    AddStudentRequest() : StudentRequest(MSG_ADD_STUDENT) {}

    // 验证请求数据
    bool isValidate() const {
        return student_id_ > 0 && 
               class_id_ > 0 && 
               !name_.empty() && 
               name_.length() <= 50;
    }
};

// 删除学生请求
class DeleteStudentRequest : public StudentRequest {
public:
    DeleteStudentRequest() : StudentRequest(MSG_DEL_STUDENT) {}

    bool isValidate() const {
        return student_id_ > 0;  // 只需要学号
    }
};

// 修改学生请求
class ModifyStudentRequest : public StudentRequest {
public:
    ModifyStudentRequest() : StudentRequest(MSG_MOD_STUDENT) {}

    bool isValidate() const {
        return student_id_ > 0 &&  // 必须有学号
               (class_id_ > 0 || !name_.empty());  // 至少修改一项
    }
};

// 查询学生请求
class QueryStudentRequest : public StudentRequest {
public:
    QueryStudentRequest() : StudentRequest(MSG_QUERY_STUDENT) {}

    bool isValidate() const {
        return true;  // 所有参数都是可选的
    }

    // 是否按学号查询
    bool HasStudentId() const { return student_id_ > 0; }
    
    // 是否按班级查询
    bool HasClassId() const { return class_id_ > 0; }
    
    // 是否按姓名查询
    bool HasName() const { return !name_.empty(); }
};