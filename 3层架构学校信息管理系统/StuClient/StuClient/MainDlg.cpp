// MainDlg.cpp: 实现文件
//

#include "pch.h"
#include "StuClient.h"
#include "afxdialogex.h"
#include "MainDlg.h"
#include <StudentRequest.h>
#include "RspHandlerMgr.h"
#include "ReqHandlerMgr.h"

static const UINT HEARTBEAT_TIMER = 1;
static const UINT HEARTBEAT_INTERVAL = 10000;//10s

// MainDlg 对话框

IMPLEMENT_DYNAMIC(MainDlg, CDialogEx)

MainDlg::MainDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MAIN_DIALOG, pParent)
	, strEdit1_(_T(""))
	, strEdit2_(_T(""))
	, strEdit3_(_T(""))
	, strText1_(_T(""))
	, strText2_(_T(""))
	, strText3_(_T(""))
{

}

MainDlg::~MainDlg()
{
    KillTimer(HEARTBEAT_TIMER);
}

void MainDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, AddBtn, addBtn_);
    DDX_Control(pDX, DeleteBtn, deleteBtn_);
    DDX_Control(pDX, IDC_LIST1, resultList_);
    DDX_Control(pDX, IDC_TAB1, tab_);
    DDX_Control(pDX, ModifyBtn, modifyBtn_);
    DDX_Control(pDX, QueryBtn, queryBtn_);
    DDX_Text(pDX, Edit1, strEdit1_);
    DDX_Text(pDX, Edit2, strEdit2_);
    DDX_Text(pDX, Edit3, strEdit3_);
    DDX_Text(pDX, Text1, strText1_);
    DDX_Text(pDX, Text2, strText2_);
    DDX_Text(pDX, Text3, strText3_);

}


BEGIN_MESSAGE_MAP(MainDlg, CDialogEx)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &MainDlg::OnSelchangeTab)
    ON_MESSAGE(WM_SOCKET, &MainDlg::OnSocket)
    ON_BN_CLICKED(AddBtn, &MainDlg::OnBnClickedAddbtn)
    ON_BN_CLICKED(DeleteBtn, &MainDlg::OnBnClickedDeletebtn)
    ON_BN_CLICKED(ModifyBtn, &MainDlg::OnBnClickedModifybtn)
    ON_BN_CLICKED(QueryBtn, &MainDlg::OnBnClickedQuerybtn)
    ON_WM_TIMER()
END_MESSAGE_MAP()


// MainDlg 消息处理程序


BOOL MainDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

    HWND wnd = GetSafeHwnd();
    UdpConnect::GetInstance()->SetMessageWindow(wnd);

	tab_.InsertItem(0, _T("学生管理"));
	tab_.InsertItem(1, _T("班级管理"));
	tab_.InsertItem(2, _T("课程管理"));
    tab_.InsertItem(3, _T("选课管理"));

	resultList_.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	UpdateUIByTab(0);

    SetTimer(HEARTBEAT_TIMER, HEARTBEAT_INTERVAL, nullptr);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}



void MainDlg::UpdateUIByTab(int tabIndex)
{
    // 删除列表内容
    resultList_.DeleteAllItems();
    while (resultList_.DeleteColumn(0));

    //清空edit内容
    strEdit1_ = _T("");
    strEdit2_ = _T("");
    strEdit3_ = _T("");

    // 根据Tab页设置 标签文本 控件显示状态 列头 configList
    CListBox* pList = (CListBox*)GetDlgItem(ConfigList);
    pList->ResetContent();

    switch (tabIndex) {
    case 0: // 学生管理
        strText1_ = _T("学号：");
        strText2_ = _T("姓名：");
        strText3_ = _T("班级ID：");
        GetDlgItem(Edit3)->ShowWindow(SW_SHOW);
        GetDlgItem(Text3)->ShowWindow(SW_SHOW);
       /* TRACE("[UI] 设置学生管理标签: %s, %s, %s\n",
            strText1_, strText2_, strText3_);*/
        resultList_.InsertColumn(0, _T("学号"), LVCFMT_LEFT, 100);
        resultList_.InsertColumn(1, _T("姓名"), LVCFMT_LEFT, 100);
        resultList_.InsertColumn(2, _T("班级ID"), LVCFMT_LEFT, 100);
        resultList_.InsertColumn(3, _T("班级名称"), LVCFMT_LEFT, 150);

        pList->AddString(_T("学生管理："));
        pList->AddString(_T("添加：学号,姓名,班级ID (1001,张三,1)"));
        pList->AddString(_T("删除：学号 (1001)"));
        pList->AddString(_T("修改：学号,新姓名,新班级ID (1001,张三,2)"));
        pList->AddString(_T("查询：学号/姓名/班级ID，空查所有"));
        break;

    case 1: // 班级管理
        strText1_ = _T("班级ID：");
        strText2_ = _T("班级名称：");
        strText3_.Empty();
        GetDlgItem(Edit3)->ShowWindow(SW_HIDE);
        GetDlgItem(Text3)->ShowWindow(SW_HIDE);

        resultList_.InsertColumn(0, _T("班级ID"), LVCFMT_LEFT, 100);
        resultList_.InsertColumn(1, _T("班级名称"), LVCFMT_LEFT, 200);

        pList->AddString(_T("班级管理："));
        pList->AddString(_T("添加：班级ID,班级名称 (1,计算机1班)"));
        pList->AddString(_T("删除：班级ID (1)"));
        pList->AddString(_T("修改：班级ID,新班级名称 (1,软件1班)"));
        pList->AddString(_T("查询：班级名称，空查所有"));
        break;

    case 2: // 课程管理
        strText1_ = _T("课程ID：");
        strText2_ = _T("课程名称：");
        strText3_ = _T("学分：");
        GetDlgItem(Edit3)->ShowWindow(SW_SHOW);
        GetDlgItem(Text3)->ShowWindow(SW_SHOW);

        resultList_.InsertColumn(0, _T("课程ID"), LVCFMT_LEFT, 100);
        resultList_.InsertColumn(1, _T("课程名称"), LVCFMT_LEFT, 150);
        resultList_.InsertColumn(2, _T("学分"), LVCFMT_LEFT, 100);

        pList->AddString(_T("课程管理：\r\n"));
        pList->AddString(_T("添加：课程ID,课程名称,学分 (101,C++,4)\r\n"));
        pList->AddString(_T("删除：课程ID (101)\r\n"));
        pList->AddString(_T("修改：课程ID,新课程名称,新学分 (101,Java,3)\r\n"));
        pList->AddString(_T("查询：课程名称，空查所有"));

        break;

    case 3: // 选课管理
        strText1_ = _T("学号：");
        strText2_ = _T("课程ID：");
        strText3_.Empty();
        GetDlgItem(Edit3)->ShowWindow(SW_HIDE);
        GetDlgItem(Text3)->ShowWindow(SW_HIDE);

        resultList_.InsertColumn(0, _T("学号"), LVCFMT_LEFT, 80);
        resultList_.InsertColumn(1, _T("姓名"), LVCFMT_LEFT, 100);
        resultList_.InsertColumn(2, _T("课程ID"), LVCFMT_LEFT, 80);
        resultList_.InsertColumn(3, _T("课程名称"), LVCFMT_LEFT, 120);
        resultList_.InsertColumn(4, _T("学分"), LVCFMT_LEFT, 60);

        pList->AddString(_T("选课管理：\r\n"));
        pList->AddString(_T("选课：学号,课程ID (1001,101)\r\n"));
        pList->AddString(_T("退课：学号,课程ID (1001,101)\r\n"));
        pList->AddString(_T("查询：学号/课程ID，空查所有\r\n"));
        pList->AddString(_T("注：支持组合查询"));

        break;
    }

    UpdateData(FALSE);
    //TRACE("[UI] UpdateData完成\n");
}

void MainDlg::OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult)
{
    UpdateUIByTab(tab_.GetCurSel());
    *pResult = 0;
}

afx_msg LRESULT MainDlg::OnSocket(WPARAM wParam, LPARAM lParam)
{
    std::unique_ptr<uint8_t[]> data((uint8_t*)wParam);//不理解
    ResponseMsg* rsp = (ResponseMsg*)data.get();

    TRACE("[响应] 收到数据包,长度:%d,消息类型:%d,响应类型:%d,结果:%d,消息:%s\n",
        lParam, rsp->msgType, rsp->rspType, rsp->result, rsp->errMsg);

    // 处理响应
    RspHandlerMgr::Instance().HandleRsp(rsp, this);
    return 0;
}

void MainDlg::UpdateList(QueryResult* result) {
    TRACE("[列表] 开始更新列表\n");

    // 1. 清空数据 保留列头
    resultList_.DeleteAllItems();

    // 2. 解析并显示数据
    const char* p = result->data;
    for (int row = 0; row < result->rows; row++) {
        std::vector<CString> rowData;

        // 3.1 读取一行数据
        for (int col = 0; col < result->cols; col++) {
            // 读取字符串长度
            int len = *(int*)p;
            p += sizeof(int);

            // 读取字符串内容
            CString str;
            if (len > 0) {
                str = CString(p, len);
            }
            p += len;

            rowData.push_back(str);
        }

        // 3.2 插入到列表
        int index = resultList_.GetItemCount();
        resultList_.InsertItem(index, rowData[0]); 
        for (size_t i = 1; i < rowData.size(); i++) {
            resultList_.SetItemText(index, i, rowData[i]);
        }
    }
    resultList_.Invalidate(FALSE);
    UpdateWindow();
    //resultList_.UpdateWindow();
    TRACE("[列表] 更新完成，共%d条记录\n", result->rows);
}

void MainDlg::OnBnClickedAddbtn() {
    ReqHandlerMgr::Instance().SendRequest(tab_.GetCurSel(), OP_ADD, this);
}

void MainDlg::OnBnClickedDeletebtn() {
    ReqHandlerMgr::Instance().SendRequest(tab_.GetCurSel(), OP_DELETE, this);
}

void MainDlg::OnBnClickedModifybtn() {
    ReqHandlerMgr::Instance().SendRequest(tab_.GetCurSel(), OP_MODIFY, this);
}

void MainDlg::OnBnClickedQuerybtn() {
    ReqHandlerMgr::Instance().SendRequest(tab_.GetCurSel(), OP_QUERY, this);
}

void MainDlg::ClearEdit()
{
    //清空edit内容
    strEdit1_ = _T("");
    strEdit2_ = _T("");
    strEdit3_ = _T("");
    UpdateData(FALSE);
}


static const UINT MAX_MISSED_HEARTBEATS = 2;

void MainDlg::OnTimer(UINT_PTR nIDEvent) {
    if (nIDEvent == HEARTBEAT_TIMER) {
        ReqHandlerMgr::Instance().SendHeartbeat();
        missedHeartbeats_++;// 增加丢失计数 

        if (missedHeartbeats_ >= MAX_MISSED_HEARTBEATS) {
            SetDisConnect();
        }
    }
    
    CDialogEx::OnTimer(nIDEvent);
}

void MainDlg::SetConnect()
{
    missedHeartbeats_ = 0;
    SetWindowText(_T("学生管理系统"));
}

void MainDlg::SetDisConnect()
{
    SetWindowText(_T("学生管理系统 (已断开)"));
}
