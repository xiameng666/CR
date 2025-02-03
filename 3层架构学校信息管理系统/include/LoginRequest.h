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

    // Request�ӿ�ʵ��
    int GetType() const override { return msg_type_; }

    int GetSize() const override {
        return sizeof(int) * 3 + username_len_ + password_len_;  // msgType + �������� + ����
    }

    //C->Sʱ�������л������ݰ���
    bool Serialize(char* buffer, int bufferSize) const override {
        printf("[���л�] ����:%d, �û���:%s\n",
            msg_type_, username_.c_str());

        if (bufferSize < GetSize()) {
            printf("[���л�] ʧ�� - ������̫С: ��Ҫ%d, ʵ��%d\n",
                GetSize(), bufferSize);
            return false;
        }

        char* p = buffer;
        // д����Ϣ����
        memcpy(p, &msg_type_, sizeof(int));
        p += sizeof(int);

        // д���û������Ⱥ�����
        memcpy(p, &username_len_, sizeof(int));
        p += sizeof(int);
        memcpy(p, username_.c_str(), username_len_);
        p += username_len_;

        // д�����볤�Ⱥ�����
        memcpy(p, &password_len_, sizeof(int));
        p += sizeof(int);
        memcpy(p, password_.c_str(), password_len_);

        printf("[���л�] �ɹ� - �ܴ�С:%d\n", GetSize());
        return true;
    }

    //S�յ����ݰ��巴���л�
    bool Deserialize(const char* buffer, int len) override {
        printf("[�����л�] ��ʼ������¼��Ϣ������:%d\n", len);

        if (len < sizeof(int) * 3) {  // ��Ϣ���� + ��������
            printf("[�����л�] ʧ�� - ���ݳ��Ȳ���\n");
            return false;
        }

        const char* p = buffer;
        // ��ȡ��Ϣ����
        memcpy(&msg_type_, p, sizeof(int));
        p += sizeof(int);

        // ��ȡ�û������Ⱥ�����
        memcpy(&username_len_, p, sizeof(int));
        p += sizeof(int);
        username_.assign(p, username_len_);
        p += username_len_;

        // ��ȡ���볤�Ⱥ�����
        memcpy(&password_len_, p, sizeof(int));
        p += sizeof(int);
        password_.assign(p, password_len_);

        printf("[�����л�] �ɹ� - ����:%d, �û���:%s\n",
            msg_type_, username_.c_str());
        return true;
    }

    // ��֤��������
    bool isValidate() const {
        return !username_.empty() &&
            !password_.empty() &&
            username_.length() <= 50 &&
            password_.length() <= 50;
    }

  
};