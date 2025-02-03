#pragma once
#include "pch.h"
#include "MainDlg.h"

//S->C ��Ӧ����ӿ�	������������� ��������Ӧ
class IRspHandler {
protected:
	virtual ~IRspHandler() = default;
	virtual void Parse(ResponseMsg* rsp, CDialogEx* dlg) = 0;

	friend class RspHandlerMgr;
};

//ResponseMsg.rspType->�ж�Response����->ResponseHandler��new������->ʵ�ֲ�ͬ��Parse
 
//��¼��Ӧ �л�����
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
			dlg->MessageBox(rsp->errMsg, "��¼ʧ��", MB_OK);
		}
	}
};

//����Ҫ����UI����Ӧ  ��ɾ��
class UnUiRspHandler : public IRspHandler {
protected:
	void Parse(ResponseMsg* rsp, CDialogEx* dlg) override {
		dlg->MessageBox(rsp->errMsg,
			rsp->result == 0 ? "�ɹ�" : "ʧ��",
			MB_OK);
	}
};

//��Ҫ����UI����Ӧ ��
class UiRspHandler : public IRspHandler {
protected:
	void Parse(ResponseMsg* rsp, CDialogEx* dlg) override {
		if (rsp->result == 0) {
			// ���������ڵ��б�
			((MainDlg*)dlg)->UpdateList(&rsp->queryResult);
		}
		else {
			dlg->MessageBox(rsp->errMsg, "��ѯʧ��", MB_ICONERROR);
		}
	}
};

class HeartbeatRspHandler : public IRspHandler {
protected:
	void Parse(ResponseMsg* rsp, CDialogEx* dlg) override {
		// ������ֻ��Ҫ��������״̬������Ҫ����
		if (rsp->result == 0) {
			MainDlg* mainDlg = ((MainDlg*)dlg);
			if (mainDlg) {
				// �յ���Ӧ�����ö�ʧ����
				mainDlg->SetConnect();
			}
		}
	}
};

