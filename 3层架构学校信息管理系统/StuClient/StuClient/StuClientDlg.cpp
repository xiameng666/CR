
// StuClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "StuClient.h"
#include "StuClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <LoginRequest.h>
#include "MainDlg.h"
#include "RspHandlerMgr.h"
#include "ReqHandlerMgr.h"


// CStuClientDlg 对话框



CStuClientDlg::CStuClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_STUCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CStuClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CStuClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(LoginBtn, &CStuClientDlg::OnBnClickedLoginbtn)
	ON_MESSAGE(WM_SOCKET, &CStuClientDlg::OnSocket)
END_MESSAGE_MAP()


// CStuClientDlg 消息处理程序

BOOL CStuClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CStuClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CStuClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CStuClientDlg::OnBnClickedLoginbtn()
{
	auto connect = UdpConnect::GetInstance();
	HWND wnd = GetSafeHwnd();
	if (connect->Init(wnd) < 0)
	{
		AfxMessageBox(_T("连接服务器失败"));
		return;
	}

	CString username = "admin";
	CString password = "123456";
	if (!ReqHandlerMgr::Instance().SendLoginRequest(username, password)) {
		MessageBox(_T("发送请求失败"), _T("错误"));
	}
}

afx_msg LRESULT CStuClientDlg::OnSocket(WPARAM wParam, LPARAM lParam)
{
	std::unique_ptr<uint8_t[]> data((uint8_t*)wParam);//不理解
	ResponseMsg* rsp = (ResponseMsg*)data.get();

	TRACE("[响应] 收到数据包,长度:%d,消息类型:%d,响应类型:%d,结果:%d,消息:%s\n",
		lParam,rsp->msgType, rsp->rspType, rsp->result, rsp->errMsg);

	// 处理响应
	RspHandlerMgr::Instance().HandleRsp(rsp, this);
	return 0;
}
