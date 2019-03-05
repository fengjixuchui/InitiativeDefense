// HaveProc.cpp : 实现文件
//

#include "stdafx.h"
#include "main.h"
#include "HaveProc.h"
#include "ioctls.h"
extern DWORD pid;//保存全局变量PID
// HaveProc 对话框
#include "HttpClient.h"

extern CListBox ProcListBox;

IMPLEMENT_DYNAMIC(HaveProc, CDialog)

HaveProc::HaveProc(CWnd* pParent /*=NULL*/)
	: CDialog(HaveProc::IDD, pParent)
	, ParentPid(_T(""))
	, ParentPath(_T(""))
	, NewProcessPid(_T(""))
	, NewProcessPath(_T(""))
	, procdescribe(_T(""))
{

}

HaveProc::~HaveProc()
{
}

void HaveProc::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, ParentPid);
	DDX_Text(pDX, IDC_EDIT2, ParentPath);
	DDX_Text(pDX, IDC_EDIT3, NewProcessPid);
	DDX_Text(pDX, IDC_EDIT4, NewProcessPath);
	DDX_Text(pDX, IDC_EDIT5, procdescribe);
}


BEGIN_MESSAGE_MAP(HaveProc, CDialog)
	ON_EN_CHANGE(IDC_EDIT4, &HaveProc::OnEnChangeEdit4)
	ON_EN_CHANGE(IDC_EDIT1, &HaveProc::OnEnChangeEdit1)
	ON_BN_CLICKED(IDOK, &HaveProc::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &HaveProc::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, &HaveProc::OnBnClickedButton1)
	ON_EN_CHANGE(IDC_EDIT3, &HaveProc::OnEnChangeEdit3)

	ON_BN_CLICKED(IDOK2, &HaveProc::OnBnClickedOk2)
	ON_BN_CLICKED(IDCANCEL2, &HaveProc::OnBnClickedCancel2)
END_MESSAGE_MAP()






// HaveProc 消息处理程序

void HaveProc::OnEnChangeEdit4()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

void HaveProc::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

void HaveProc::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	ResumePro(pid);//恢复
	ProcListBox.AddString(L"用户确认,一直允许该进程开启");
	OnOK();
}

void HaveProc::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	KillPro(pid);//杀死
	ProcListBox.AddString(L"用户确认,一直阻止该进程开启");
	OnCancel();
}

void HaveProc::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strtemp;
	strtemp.Format(L"进程ID:%d",pid);
	this->MessageBox(strtemp);
	ResumePro(pid);

}

void HaveProc::OnEnChangeEdit3()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

void HaveProc::OnBnClickedOk2()
{
	// TODO: 在此添加控件通知处理程序代码
	ResumePro(pid);//恢复
	ProcListBox.AddString(L"用户确认,允许该进程开启");
	OnOK();
}

void HaveProc::OnBnClickedCancel2()
{
	// TODO: 在此添加控件通知处理程序代码
	KillPro(pid);//杀死
	ProcListBox.AddString(L"用户确认,阻止该进程开启");
	OnCancel();
}

BOOL HaveProc::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

//	this->MessageBox(b);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
