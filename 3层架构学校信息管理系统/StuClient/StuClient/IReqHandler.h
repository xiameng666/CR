// IReqHandler.h
#pragma once
#include "pch.h"
#include "MainDlg.h"
#include <IRequest.h>

//�ڰ�ť��Ӧ�ĺ�����ʹ��
// �ؼ�����->��������
class IReqHandler {
protected:

	virtual ~IReqHandler() = default;
	virtual bool Handle(CDialogEx* dlg) = 0;

	friend class ReqHandlerMgr;
};

class BaseReqHandler :public IReqHandler {
protected:
	bool GetViewData(CDialogEx* dlg, IRequest* req) {

	}
};

