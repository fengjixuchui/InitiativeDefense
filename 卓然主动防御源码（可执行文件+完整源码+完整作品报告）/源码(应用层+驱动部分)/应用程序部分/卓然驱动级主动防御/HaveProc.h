#pragma once


// HaveProc 对话框

class HaveProc : public CDialog
{
	DECLARE_DYNAMIC(HaveProc)

public:
	HaveProc(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~HaveProc();

// 对话框数据
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeEdit4();
	CString ParentPid;
	CString ParentPath;
	CString NewProcessPid;
	CString NewProcessPath;
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnBnClickedOk2();
	afx_msg void OnBnClickedCancel2();
	virtual BOOL OnInitDialog();
	CString procdescribe;
};
