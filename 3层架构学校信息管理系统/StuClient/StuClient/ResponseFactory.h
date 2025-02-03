#pragma once
#include <memory>
#include "IRspHandler.h"


class ResponseFactory {
private:
	static std::shared_ptr<IRspHandler> CreateResponse(int rspType) {//TODO 这里静态的作用是什么
		switch (rspType) {
		case RSP_LOGIN:
			return std::make_shared<LoginRspHandler>();
		case RSP_NO_RESULT:
			return std::make_shared<UnUiRspHandler>();
		case RSP_HAS_RESULT:
			return std::make_shared<UiRspHandler>();
		case RSP_HEARTBEAT:
			return std::make_shared<HeartbeatRspHandler>();
		default:
			return nullptr;
		}
	}

	friend class RspHandlerMgr;
};

