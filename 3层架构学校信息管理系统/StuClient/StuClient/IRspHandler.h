#pragma once
#include "pch.h"
#include "MainDlg.h"

//S->C 响应处理接口	派生类解析数据 做具体响应
class IRspHandler {
protected:
	virtual ~IRspHandler() = default;
	virtual void Parse(ResponseMsg* rsp, CDialogEx* dlg) = 0;

	friend class RspHandlerMgr;
};

//ResponseMsg.rspType->判断Response类型->ResponseHandler中new派生类->实现不同的Parse
 
//登录响应 切换界面
class LoginRspHandler : public IRspHandler {
protected:
	void Parse(ResponseMsg* rsp, CDialogEx* dlg) override {
		if (rsp->result == 0) {
			MainDlg* pMainDlg = new MainDlg();
			AfxGetApp()->m_pMainWnd = pMainDlg;

			dlg->SendMessage(WM_DESTROY);
			dlg->SendMessage(WM_CLOSE);

			pMainDlg->DoModal();
		}
		else {
			dlg->MessageBox(rsp->errMsg, "登录失败", MB_OK);
		}
	}
};

//不需要更新UI的响应  增删改
class UnUiRspHandler : public IRspHandler {
protected:
	void Parse(ResponseMsg* rsp, CDialogEx* dlg) override {
		dlg->MessageBox(rsp->errMsg,
			rsp->result == 0 ? "成功" : "失败",
			MB_OK);
	}
};

//需要更新UI的响应 查
class UiRspHandler : public IRspHandler {
protected:
	void Parse(ResponseMsg* rsp, CDialogEx* dlg) override {
		if (rsp->result == 0) {
			// 更新主窗口的列表
			((MainDlg*)dlg)->UpdateList(&rsp->queryResult);
		}
		else {
			dlg->MessageBox(rsp->errMsg, "查询失败", MB_ICONERROR);
		}
	}
};

class HeartbeatRspHandler : public IRspHandler {
protected:
	void Parse(ResponseMsg* rsp, CDialogEx* dlg) override {
		// 心跳包只需要更新连接状态，不需要弹窗
		if (rsp->result == 0) {
			MainDlg* mainDlg = ((MainDlg*)dlg);
			if (mainDlg) {
				// 收到响应，重置丢失计数
				mainDlg->SetConnect();
			}
		}
	}
};

