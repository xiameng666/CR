#pragma once
#include "RequestFactory.h"
#include <LoginRequest.h>

class ReqHandlerMgr {
private:
    ReqHandlerMgr() = default;
    std::map<int, std::unique_ptr<IRequest>> requests_;

public:
    static ReqHandlerMgr& Instance() {
        static ReqHandlerMgr instance;
        return instance;
    }

    bool SendRequest(int tabIndex, int operation, CDialogEx* dlg) {
        // ÿ�ζ������µ��������
        auto req = RequestFactory::CreateRequest(tabIndex, operation);
        if (!req) return false;

        auto mainDlg = (MainDlg*)dlg;
        mainDlg->UpdateData(TRUE);
        req->BindUIData(mainDlg->GetEdit1(),
            mainDlg->GetEdit2(),
            mainDlg->GetEdit3());

        if (!SendRequestImp(req.get())) return false;
        mainDlg->ClearEdit();
        return true ;
    }

    // ��¼����
    bool SendLoginRequest(const CString& username, const CString& password) {
        TRACE("[����] ��ʼ�����¼\n");

        LoginRequest req;
        if (!req.BindUIData(username, password, _T(""))) {
            TRACE("[����] ��¼���ݰ�ʧ��\n");
            return false;
        }

        if (!SendRequestImp(&req)) return false;
        return true;
    }

    //������
    bool SendHeartbeat() {
        TRACE("[����] ��ʼ��������\n");

        HeartbeatRequest req;
        if (!SendRequestImp(&req)) return false;

        return true;
    }
private:
    // ͳһ�ķ���ʵ��
    bool SendRequestImp(IRequest* req) {
        // ��֤����
        if (!req->isValidate()) {
            TRACE("[��������] ������֤ʧ��\n");
            return false;
        }

        char buf[1024] = { 0 };
        int len = req->GetSize();

        TRACE("[��������] ����:%d, ��С:%d\n", req->GetType(), len);

        if (!req->Serialize(buf, sizeof(buf))) {
            TRACE("[��������] ���л�ʧ��\n");
            return false;
        }

        if (UdpConnect::GetInstance()->SendData((uint8_t*)buf, len) < 0) {
            TRACE("[��������] ����ʧ��\n");
            return false;
        }

        TRACE("[��������] �ɹ�\n");
        return true;
    }
};