#pragma once
#include "IReqHandler.h"
#include "StudentRequest.h"
#include "ClassRequest.h"
#include "CourseRequest.h"
#include "SelectionRequest.h"
#include "HeartbeatRequest.h"

enum RequestOperation {
    OP_ADD = 1,
    OP_DELETE,
    OP_MODIFY,
    OP_QUERY
};

class RequestFactory {
public:

    static std::unique_ptr<IRequest> CreateRequest(int tabIndex, int operation) {
        TRACE("[����] �������� - Tab:%d, Operation:%d\n", tabIndex, operation);

        switch (tabIndex) {
        case 0: // ѧ������
            switch (operation) {
            case OP_ADD: return std::make_unique<AddStudentRequest>();
            case OP_DELETE: return std::make_unique<DeleteStudentRequest>();
            case OP_MODIFY: return std::make_unique<ModifyStudentRequest>();
            case OP_QUERY: return std::make_unique<QueryStudentRequest>();
            }
            break;

        case 1: // �༶����
            switch (operation) {
            case OP_ADD: return std::make_unique<AddClassRequest>();
            case OP_DELETE: return std::make_unique<DeleteClassRequest>();
            case OP_MODIFY: return std::make_unique<ModifyClassRequest>();
            case OP_QUERY: return std::make_unique<QueryClassRequest>();
            }
            break;

        case 2: // �γ̹���
            switch (operation) {
            case OP_ADD: return std::make_unique<AddCourseRequest>();
            case OP_DELETE: return std::make_unique<DeleteCourseRequest>();
            case OP_MODIFY: return std::make_unique<ModifyCourseRequest>();
            case OP_QUERY: return std::make_unique<QueryCourseRequest>();
            }
            break;

        case 3: // ѡ�ι���
            switch (operation) {
            case OP_ADD: return std::make_unique<SelectCourseRequest>();
            case OP_DELETE: return std::make_unique<DropCourseRequest>();
            case OP_QUERY: return std::make_unique<QuerySelectionRequest>();
            }
            break;
        }

        TRACE("[����] ����ʧ�� - δ֪��Tab��Operation\n");
        return nullptr;
    }

    friend class ReqHandlerMgr;
};