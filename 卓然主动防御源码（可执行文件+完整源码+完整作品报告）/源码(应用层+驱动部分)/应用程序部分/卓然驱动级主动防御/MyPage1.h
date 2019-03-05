#pragma once
#include "HaveProc.h"

// CMyPage1 对话框

class CMyPage1 : public CPropertyPage
{
	DECLARE_DYNAMIC(CMyPage1)

public:
	CMyPage1();
	virtual ~CMyPage1();

// 对话框数据
	enum { IDD = IDD_PROPPAGE_1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	bool LoadNTDriver(LPCWSTR lpszDriverName, LPCWSTR lpszDriverPath);
	afx_msg void OnBnClickedButton1();
	bool UnloadNTDriver(PCWSTR szSvrName);
	afx_msg void OnBnClickedButton4();
	bool Startservice(LPCWSTR lpszDriverName, LPCWSTR lpszDriverPath);
	bool Stopservice(PCWSTR szSvrName);
	afx_msg void OnBnClickedButton5();
	int ProcMon(void);
	CString Stat;
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedButton10();
	afx_msg void OnBnClickedButton12();
	afx_msg void OnBnClickedButton11();

	afx_msg void OnBnClickedButton13();
};
