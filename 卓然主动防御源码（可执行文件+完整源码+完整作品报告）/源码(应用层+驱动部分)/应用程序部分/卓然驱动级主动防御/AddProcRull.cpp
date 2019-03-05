// AddProcRull.cpp : 实现文件
// Download by http://www.51xue8xue8.com
#include "stdafx.h"
#include "AddProcRull.h"


// AddProcRull 对话框

IMPLEMENT_DYNAMIC(AddProcRull, CDialog)

AddProcRull::AddProcRull(CWnd* pParent /*=NULL*/)
	: CDialog(AddProcRull::IDD, pParent)
	, pass(_T(""))
	, parentprocname(_T(""))
	, procpath(_T(""))
	, parentprocpath(_T(""))
{

}

AddProcRull::~AddProcRull()
{
}

void AddProcRull::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, pass);
	DDX_Text(pDX, IDC_EDIT3, parentprocname);
	DDX_Text(pDX, IDC_EDIT2, procpath);
	DDX_Text(pDX, IDC_EDIT4, parentprocpath);
}


BEGIN_MESSAGE_MAP(AddProcRull, CDialog)
END_MESSAGE_MAP()


// AddProcRull 消息处理程序
