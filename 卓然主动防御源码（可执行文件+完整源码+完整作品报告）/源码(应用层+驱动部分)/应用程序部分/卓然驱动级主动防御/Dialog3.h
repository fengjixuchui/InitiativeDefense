#pragma once


// CDialog3 对话框

class CDialog3 : public CDialog
{
	DECLARE_DYNAMIC(CDialog3)

public:
	CDialog3(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDialog3();

// 对话框数据
	enum { IDD = IDD_DIALOG3 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
