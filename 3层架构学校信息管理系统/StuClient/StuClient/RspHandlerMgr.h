#pragma once
#include <map>
#include <memory>
#include "IRspHandler.h"
#include "ResponseFactory.h"

//单例 响应处理器管理
class RspHandlerMgr {
private:
	std::map<int, std::shared_ptr<IRspHandler>> handlers_;

	RspHandlerMgr() = default;

public:
	//TODO 不理解这里的单例实现
	static RspHandlerMgr& Instance() {
		static RspHandlerMgr instance;
		return instance;
	}

	//处理响应
	void HandleRsp(ResponseMsg* rsp, CDialogEx* dlg) {

		//查找是否已有处理器
		auto it = handlers_.find(rsp->rspType);
		if (it == handlers_.end()) {
			//没有，用类工厂动态创建
			auto handler = ResponseFactory::CreateResponse(rsp->rspType);
			if (handler) {
				TRACE("[响应处理] 创建新处理器\n");
				handlers_[rsp->rspType] = handler;
				handler->Parse(rsp, dlg);
			}
			else {
				TRACE("CreateResponse nullptr");
			}
		}
		else {
			// 使用已有处理器
			TRACE("[响应处理] 使用已有处理器\n");
			it->second->Parse(rsp, dlg);
		}

	}

	~RspHandlerMgr() = default;
	RspHandlerMgr(const RspHandlerMgr&) = delete;
	RspHandlerMgr& operator=(const RspHandlerMgr&) = delete;
};