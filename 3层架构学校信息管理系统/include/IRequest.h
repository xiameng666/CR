#pragma once
#include "proto.h"
#include <afx.h>

// ������Ϣ����
class IRequest {
public:
    virtual ~IRequest() = default;
    
    // ��ȡ��Ϣ����
    virtual int GetType() const = 0;
    
    // ��ȡ��Ϣ��С
    virtual int GetSize() const = 0;
    
    //��֤������Ч��
    virtual bool isValidate() const = 0;

    // ���л���Ϣ��buffer
    virtual bool Serialize(char* buffer, int bufferSize) const = 0;
    
    // ��buffer�����л���Ϣ
    virtual bool Deserialize(const char* buffer, int len) = 0;

    //�ÿؼ����ݵ�ʵ���Ա����
    virtual bool BindUIData(const CString& str1, const CString& str2, const CString& str3) = 0;

    // ͳһ�ķ�������  ���ڷ������Ϳͻ��˹�������ӿڣ�������Ҫ����udpconnnect��������û������ļ��������Ƶ�Mgr��ʵ��
    // bool SendRequest() {    }
    
};

