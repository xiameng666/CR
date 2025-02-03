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
        // 每次都创建新的请求对象
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

    // 登录请求
    bool SendLoginRequest(const CString& username, const CString& password) {
        TRACE("[请求] 开始处理登录\n");

        LoginRequest req;
        if (!req.BindUIData(username, password, _T(""))) {
            TRACE("[请求] 登录数据绑定失败\n");
            return false;
        }

        if (!SendRequestImp(&req)) return false;
        return true;
    }

    //心跳包
    bool SendHeartbeat() {
        TRACE("[请求] 开始发送心跳\n");

        HeartbeatRequest req;
        if (!SendRequestImp(&req)) return false;

        return true;
    }
private:
    // 统一的发送实现
    bool SendRequestImp(IRequest* req) {
        // 验证数据
        if (!req->isValidate()) {
            TRACE("[发送请求] 数据验证失败\n");
            return false;
        }

        char buf[1024] = { 0 };
        int len = req->GetSize();

        TRACE("[发送请求] 类型:%d, 大小:%d\n", req->GetType(), len);

        if (!req->Serialize(buf, sizeof(buf))) {
            TRACE("[发送请求] 序列化失败\n");
            return false;
        }

        if (UdpConnect::GetInstance()->SendData((uint8_t*)buf, len) < 0) {
            TRACE("[发送请求] 发送失败\n");
            return false;
        }

        TRACE("[发送请求] 成功\n");
        return true;
    }
};