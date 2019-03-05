#pragma once
#include "afxcmn.h"
#include "afxdao.h"
// CMyPage3 对话框


class CMyPage3 : public CPropertyPage
{
	DECLARE_DYNAMIC(CMyPage3)

public:
	CMyPage3();
	virtual ~CMyPage3();

// 对话框数据
	enum { IDD = IDD_PROPPAGE_3 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	UINT ThreadProcRegMon(LPVOID lparam);

	CString RegMonStatus;

	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnLbnSelchangeList1();

	void SetListBoxHScroll(void);
	CListCtrl RegRull;
	CDaoDatabase	db ;
	void OnBnClickedFlush(void);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
};



