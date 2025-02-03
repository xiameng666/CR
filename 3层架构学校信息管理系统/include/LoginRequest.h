// LoginRequest.h
#pragma once
#include "IRequest.h"
#include "proto.h"
#include <string>

class LoginRequest : public IRequest {
protected:
    int msg_type_;
    int username_len_;
    int password_len_;
    std::string username_;
    std::string password_;

public:
    LoginRequest() :
        msg_type_(MSG_ADMIN_LOGIN),
        username_len_(0),
        password_len_(0) {}

    // Setters
    void SetUsername(const std::string& username) {
        username_ = username;
        username_len_ = username_.length();
    }

    void SetPassword(const std::string& password) {
        password_ = password;
        password_len_ = password_.length();
    }

    // Getters
    const std::string& GetUsername() const { return username_; }
    const std::string& GetPassword() const { return password_; }

    bool BindUIData(const CString& username, const CString& password, const CString& unused) override {
        if (username.IsEmpty() || password.IsEmpty()) {
            return false;
        }

        username_ = std::string(CStringA(username));
        password_ = std::string(CStringA(password));
        username_len_ = username_.length();
        password_len_ = password_.length();

        return true;
    }

    // Request接口实现
    int GetType() const override { return msg_type_; }

    int GetSize() const override {
        return sizeof(int) * 3 + username_len_ + password_len_;  // msgType + 两个长度 + 数据
    }

    //C->S时发送序列化的数据包体
    bool Serialize(char* buffer, int bufferSize) const override {
        printf("[序列化] 类型:%d, 用户名:%s\n",
            msg_type_, username_.c_str());

        if (bufferSize < GetSize()) {
            printf("[序列化] 失败 - 缓冲区太小: 需要%d, 实际%d\n",
                GetSize(), bufferSize);
            return false;
        }

        char* p = buffer;
        // 写入消息类型
        memcpy(p, &msg_type_, sizeof(int));
        p += sizeof(int);

        // 写入用户名长度和内容
        memcpy(p, &username_len_, sizeof(int));
        p += sizeof(int);
        memcpy(p, username_.c_str(), username_len_);
        p += username_len_;

        // 写入密码长度和内容
        memcpy(p, &password_len_, sizeof(int));
        p += sizeof(int);
        memcpy(p, password_.c_str(), password_len_);

        printf("[序列化] 成功 - 总大小:%d\n", GetSize());
        return true;
    }

    //S收到数据包体反序列化
    bool Deserialize(const char* buffer, int len) override {
        printf("[反序列化] 开始解析登录消息，长度:%d\n", len);

        if (len < sizeof(int) * 3) {  // 消息类型 + 两个长度
            printf("[反序列化] 失败 - 数据长度不足\n");
            return false;
        }

        const char* p = buffer;
        // 读取消息类型
        memcpy(&msg_type_, p, sizeof(int));
        p += sizeof(int);

        // 读取用户名长度和内容
        memcpy(&username_len_, p, sizeof(int));
        p += sizeof(int);
        username_.assign(p, username_len_);
        p += username_len_;

        // 读取密码长度和内容
        memcpy(&password_len_, p, sizeof(int));
        p += sizeof(int);
        password_.assign(p, password_len_);

        printf("[反序列化] 成功 - 类型:%d, 用户名:%s\n",
            msg_type_, username_.c_str());
        return true;
    }

    // 验证请求数据
    bool isValidate() const {
        return !username_.empty() &&
            !password_.empty() &&
            username_.length() <= 50 &&
            password_.length() <= 50;
    }

  
};