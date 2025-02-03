#pragma once
#include "proto.h"
#include <afx.h>

// 请求消息基类
class IRequest {
public:
    virtual ~IRequest() = default;
    
    // 获取消息类型
    virtual int GetType() const = 0;
    
    // 获取消息大小
    virtual int GetSize() const = 0;
    
    //验证数据有效性
    virtual bool isValidate() const = 0;

    // 序列化消息到buffer
    virtual bool Serialize(char* buffer, int bufferSize) const = 0;
    
    // 从buffer反序列化消息
    virtual bool Deserialize(const char* buffer, int len) = 0;

    //拿控件数据到实体成员变量
    virtual bool BindUIData(const CString& str1, const CString& str2, const CString& str3) = 0;

    // 统一的发送请求  由于服务器和客户端共用这个接口，发送需要包含udpconnnect而服务器没有这个文件，所以移到Mgr中实现
    // bool SendRequest() {    }
    
};

