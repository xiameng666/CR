#pragma once
#include "IRequest.h"

class HeartbeatRequest : public IRequest {
private:
    int msg_type_;
    int timestamp_;  // ʱ���

public:
    HeartbeatRequest() :
        msg_type_(MSG_HEARTBEAT),
        timestamp_((int)time(nullptr)) {}

    int GetType() const override { return msg_type_; }
    int GetSize() const override { return sizeof(int) * 2; }

    bool Serialize(char* buffer, int bufferSize) const override {
        if (bufferSize < GetSize()) {
            TRACE("[����] ���л�ʧ�� - ������̫С\n");
            return false;
        }

        char* p = buffer;
        memcpy(p, &msg_type_, sizeof(int));
        p += sizeof(int);
        memcpy(p, &timestamp_, sizeof(int));

        TRACE("[����] ���л� - ����:%d, ʱ���:%d\n", msg_type_, timestamp_);
        return true;
    }

    bool Deserialize(const char* buffer, int len) override {
        if (len < GetSize()) return false;

        const char* p = buffer;
        memcpy(&msg_type_, p, sizeof(int));
        p += sizeof(int);
        memcpy(&timestamp_, p, sizeof(int));

        return true;
    }

    bool BindUIData(const CString&, const CString&, const CString&) override {
        return true;  // ����������ҪUI����
    }

    bool isValidate() const override {
        return true;
    }
};