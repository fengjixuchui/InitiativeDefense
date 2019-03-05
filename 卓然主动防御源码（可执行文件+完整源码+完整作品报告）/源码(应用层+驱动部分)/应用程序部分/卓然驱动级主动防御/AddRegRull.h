#pragma once
#include "resource.h"
// Download by http://www.codefans.net
// AddRegRull 对话框

class AddRegRull : public CDialog
{
	DECLARE_DYNAMIC(AddRegRull)

public:
	AddRegRull(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~AddRegRull();

// 对话框数据
	enum { IDD = IDD_DIALOG4 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString pass;
	CString procname;
	CString procpath;
	CString regpath;
};
