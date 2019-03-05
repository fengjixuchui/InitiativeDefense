#pragma once
#include "Resource.h"

// CMyPage0 对话框

class CMyPage0 : public CPropertyPage
{
	DECLARE_DYNAMIC(CMyPage0)

public:
	CMyPage0();
	virtual ~CMyPage0();

// 对话框数据
	enum { IDD = IDD_PROPPAGE_0 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
};
