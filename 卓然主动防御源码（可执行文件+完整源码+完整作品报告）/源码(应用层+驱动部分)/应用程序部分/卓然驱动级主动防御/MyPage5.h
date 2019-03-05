#pragma once
#include "Resource.h"

// CMyPage5 对话框

class CMyPage5 : public CPropertyPage
{
	DECLARE_DYNAMIC(CMyPage5)

public:
	CMyPage5();
	virtual ~CMyPage5();

// 对话框数据
	enum { IDD = IDD_PROPPAGE_5 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();

	afx_msg void OnBnClickedButton3();
	virtual BOOL OnInitDialog();
};
