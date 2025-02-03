#pragma once
#include <map>
#include <memory>
#include "IRspHandler.h"
#include "ResponseFactory.h"

//���� ��Ӧ����������
class RspHandlerMgr {
private:
	std::map<int, std::shared_ptr<IRspHandler>> handlers_;

	RspHandlerMgr() = default;

public:
	//TODO ���������ĵ���ʵ��
	static RspHandlerMgr& Instance() {
		static RspHandlerMgr instance;
		return instance;
	}

	//������Ӧ
	void HandleRsp(ResponseMsg* rsp, CDialogEx* dlg) {

		//�����Ƿ����д�����
		auto it = handlers_.find(rsp->rspType);
		if (it == handlers_.end()) {
			//û�У����๤����̬����
			auto handler = ResponseFactory::CreateResponse(rsp->rspType);
			if (handler) {
				TRACE("[��Ӧ����] �����´�����\n");
				handlers_[rsp->rspType] = handler;
				handler->Parse(rsp, dlg);
			}
			else {
				TRACE("CreateResponse nullptr");
			}
		}
		else {
			// ʹ�����д�����
			TRACE("[��Ӧ����] ʹ�����д�����\n");
			it->second->Parse(rsp, dlg);
		}

	}

	~RspHandlerMgr() = default;
	RspHandlerMgr(const RspHandlerMgr&) = delete;
	RspHandlerMgr& operator=(const RspHandlerMgr&) = delete;
};