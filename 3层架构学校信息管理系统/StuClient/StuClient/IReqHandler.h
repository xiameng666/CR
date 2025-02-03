// IReqHandler.h
#pragma once
#include "pch.h"
#include "MainDlg.h"
#include <IRequest.h>

//在按钮响应的函数中使用
// 控件数据->构造请求
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

