#pragma once
#include "afxcmn.h"


#include "afxdao.h"
// Download by http://www.codefans.net
// CMyPage2 对话框

class CMyPage2 : public CPropertyPage
{
	DECLARE_DYNAMIC(CMyPage2)

public:
	CMyPage2();
	virtual ~CMyPage2();

// 对话框数据
	enum { IDD = IDD_PROPPAGE_2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedButton1();
	CString ProcList;
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnLbnSelchangeList1();
	CListCtrl ProcRull;
 #pragma deprecated
	CDaoDatabase	db ;
	virtual BOOL OnInitDialog();
	void OnBnClickedFlush(void);
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton13();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnPaint();
};
