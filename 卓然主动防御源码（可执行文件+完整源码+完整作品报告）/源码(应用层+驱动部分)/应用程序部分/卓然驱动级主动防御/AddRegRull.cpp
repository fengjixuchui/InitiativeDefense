// AddRegRull.cpp : 实现文件
//

#include "stdafx.h"
#include "AddRegRull.h"


// AddRegRull 对话框

IMPLEMENT_DYNAMIC(AddRegRull, CDialog)

AddRegRull::AddRegRull(CWnd* pParent /*=NULL*/)
	: CDialog(AddRegRull::IDD, pParent)
	, pass(_T(""))
	, procname(_T(""))
	, procpath(_T(""))
	, regpath(_T(""))
{

}

AddRegRull::~AddRegRull()
{
}

void AddRegRull::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, pass);
	DDX_Text(pDX, IDC_EDIT3, procname);
	DDX_Text(pDX, IDC_EDIT2, procpath);
	DDX_Text(pDX, IDC_EDIT4, regpath);
}


BEGIN_MESSAGE_MAP(AddRegRull, CDialog)
END_MESSAGE_MAP()


// AddRegRull 消息处理程序
