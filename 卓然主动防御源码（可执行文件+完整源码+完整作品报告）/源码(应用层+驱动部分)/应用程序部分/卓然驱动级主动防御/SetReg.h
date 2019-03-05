#pragma once


// CSetReg 对话框

class CSetReg : public CDialog
{
	DECLARE_DYNAMIC(CSetReg)

public:
	CSetReg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSetReg();

// 对话框数据
	enum { IDD = IDD_DIALOG2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString processname;
	CString processpath;
	CString regname;
	CString regvalue;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton3();
	int TurnOffRegMon(void);
	afx_msg void OnBnClickedOk2();
	afx_msg void OnBnClickedCancel2();
	CString regdescribe;
};
