// MessageTask.h
#pragma once
#include "threadpool.h"
#include "proto.h"
#include "LoginRequest.h"
#include <vector>
#include "StudentDB.h"

class MessageTask : public Task {
private:
    std::vector<uint8_t> buffer;
    uint32_t connId;

public:
    MessageTask(uint8_t* buf, uint32_t len, uint32_t id);
    ~MessageTask() = default;

    Any run() override;

   

private:
    void SendResponse(int msgType, int result, const char* msg);
    void SendQueryResponse(int msgType, const char* resultBuffer, int dataSize);
};