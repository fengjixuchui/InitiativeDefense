#pragma once
#include "resource.h"

// AddProcRull 对话框

class AddProcRull : public CDialog
{
	DECLARE_DYNAMIC(AddProcRull)

public:
	AddProcRull(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~AddProcRull();

// 对话框数据
	enum { IDD = IDD_DIALOG3 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString pass;
	CString parentprocname;
	CString procpath;
	CString parentprocpath;
};
