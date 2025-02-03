
// StuClientDlg.h: 头文件
//

#pragma once
#include "pch.h"
#include "UdpConnect.h"


// CStuClientDlg 对话框
class CStuClientDlg : public CDialogEx
{
// 构造
public:
	CStuClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_STUCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;



	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	afx_msg LRESULT OnSocket(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedLoginbtn();
	DECLARE_MESSAGE_MAP()
public:

	// 处理登录响应
	//void CStuClientDlg::HandleLoginResponse(ResponseMsg* rsp);


};
