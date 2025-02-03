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

    // IRequest接口实现
    int GetType() const override { return msg_type_; }
    int GetSize() const override { return sizeof(int) * 4 + name_len_; }

    bool isValidate() const override {
        return true;  // 基类默认返回true，让派生类决定具体验证规则
    }

    bool BindUIData(const CString& str1, const CString& str2, const CString& str3) override {
        if (!str1.IsEmpty()) course_id_ = _ttoi(str1);
        if (!str2.IsEmpty()) course_name_ = std::string(CStringA(str2));
        if (!str3.IsEmpty()) credit_ = _ttoi(str3);
        name_len_ = course_name_.length();
        return true;
    }

    bool Serialize(char* buffer, int bufferSize) const override{
        printf("[序列化] 类型:%d, 课程ID:%d, 课程名称:%s, 学分:%d\n",
        msg_type_, course_id_, course_name_.c_str(), credit_);

    if (bufferSize < GetSize()) {
        printf("[序列化] 失败 - 缓冲区太小: 需要%d, 实际%d\n", 
            GetSize(), bufferSize);
        return false;
    }
    
    char* p = buffer;
    // 写入消息类型
    memcpy(p, &msg_type_, sizeof(int));
    p += sizeof(int);
    // 写入课程ID
    memcpy(p, &course_id_, sizeof(int));
    p += sizeof(int);
    // 写入学分
    memcpy(p, &credit_, sizeof(int));
    p += sizeof(int);
    // 写入名称长度
    memcpy(p, &name_len_, sizeof(int));
    p += sizeof(int);
    // 写入名称内容
    memcpy(p, course_name_.c_str(), name_len_);
    
    printf("[序列化] 成功 - 总大小:%d\n", GetSize());
    return true;
    }

    bool Deserialize(const char* buffer, int len) override{
    printf("[反序列化] 开始解析消息，长度:%d\n", len);

    if (len < sizeof(int) * 4) {  // 4个int字段
        printf("[反序列化] 失败 - 数据长度不足\n");
        return false;
    }
    
    const char* p = buffer;
    // 读取消息类型
    memcpy(&msg_type_, p, sizeof(int));
    p += sizeof(int);
    // 读取课程ID
    memcpy(&course_id_, p, sizeof(int));
    p += sizeof(int);
    // 读取学分
    memcpy(&credit_, p, sizeof(int));
    p += sizeof(int);
    // 读取名称长度
    memcpy(&name_len_, p, sizeof(int));
    p += sizeof(int);
    
    if (len < GetSize()) {
        printf("[反序列化] 失败 - 数据长度不足，需要%d\n", GetSize());
        return false;
    }

    // 读取名称内容
    course_name_.assign(p, name_len_);
    
    printf("[反序列化] 成功 - 类型:%d, 课程ID:%d, 课程名称:%s, 学分:%d\n",
        msg_type_, course_id_, course_name_.c_str(), credit_);
    return true;

    }
};

// 派生类
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