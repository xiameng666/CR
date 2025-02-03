#pragma once
#include "afxdialogex.h"
#include "UdpConnect.h"


// MainDlg 对话框

class MainDlg : public CDialogEx
{
	DECLARE_DYNAMIC(MainDlg)

public:
	MainDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~MainDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAIN_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	//std::unique_ptr<UdpConnect> m_pUdpConnect;//通讯类指针，跳转界面需要转移所有权

	CButton addBtn_;
	CButton deleteBtn_;
	CListCtrl resultList_;
	CTabCtrl tab_;
	CButton modifyBtn_;
	CButton queryBtn_;
	CString strEdit1_;
	CString strEdit2_;
	CString strEdit3_;
	CString strText1_;
	CString strText2_;
	CString strText3_;

	UINT missedHeartbeats_;
	void SetConnect();
	void SetDisConnect();

	void UpdateUIByTab(int tabIndex);
	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnBnClickedAddbtn();
	afx_msg void OnBnClickedDeletebtn();
	afx_msg void OnBnClickedModifybtn();
	afx_msg void OnBnClickedQuerybtn();

	afx_msg LRESULT OnSocket(WPARAM wParam, LPARAM lParam);
	//void HandleStudentResponse(ResponseMsg* rsp);
	void UpdateList(QueryResult* result);

	// 提供获取界面数据的接口
	CString GetEdit1() const { return strEdit1_; }
	CString GetEdit2() const { return strEdit2_; }
	CString GetEdit3() const { return strEdit3_; }

	int GetCurrentTab() const { return tab_.GetCurSel(); }
	void ClearEdit();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
