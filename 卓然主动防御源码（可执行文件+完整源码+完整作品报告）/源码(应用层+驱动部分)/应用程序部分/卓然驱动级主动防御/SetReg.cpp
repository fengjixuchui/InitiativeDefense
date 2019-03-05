// SetReg.cpp : 实现文件
//

#include "stdafx.h"
#include "main.h"
#include "SetReg.h"
#include "regioctl.h"
#include "winioctl.h"
#define SYMBOL_LINK L"\\\\.\\RegMon"
#define APP_EVENT_NAME L"RegAppEvent" //应用层是否事件

// CSetReg 对话框


extern CListBox reglistbox;//引入注册表全局列表框全局变量

IMPLEMENT_DYNAMIC(CSetReg, CDialog)

CSetReg::CSetReg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetReg::IDD, pParent)
	, processname(_T(""))
	, processpath(_T(""))
	, regname(_T(""))
	, regvalue(_T(""))
	, regdescribe(_T(""))
{

}

CSetReg::~CSetReg()
{
}

void CSetReg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, processname);
	DDX_Text(pDX, IDC_EDIT2, processpath);
	DDX_Text(pDX, IDC_EDIT3, regname);
	DDX_Text(pDX, IDC_EDIT4, regvalue);
	DDX_Text(pDX, IDC_EDIT5, regdescribe);
}


BEGIN_MESSAGE_MAP(CSetReg, CDialog)
	ON_BN_CLICKED(IDOK, &CSetReg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CSetReg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, &CSetReg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CSetReg::OnBnClickedButton3)
	ON_BN_CLICKED(IDOK2, &CSetReg::OnBnClickedOk2)
	ON_BN_CLICKED(IDCANCEL2, &CSetReg::OnBnClickedCancel2)
END_MESSAGE_MAP()


// CSetReg 消息处理程序

void CSetReg::OnBnClickedOk()
{
	REG_CALLBACK_INFO reginfo1={0}, regtemp1 = {0};
	HANDLE hDriver = ::CreateFile(
							SYMBOL_LINK,
							GENERIC_READ | GENERIC_WRITE,
							0,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
	if (hDriver == INVALID_HANDLE_VALUE)
	{
		::MessageBox(NULL,L"打开驱动设备对象失败！\n",L"错误",MB_OK);
	}

	HANDLE hProcessEvent = ::OpenEventW(SYNCHRONIZE, FALSE, APP_EVENT_NAME);
	if (hProcessEvent == NULL)
	{
		::MessageBox(NULL,L"打开事件失败！\n",L"错误",MB_OK);
	}

	DWORD	dwRet;
	BOOL	bRet;

	bRet = ::DeviceIoControl(
						hDriver,
						IOCTL_NTPROCDRV_SET_APPEVENT_OK,
						NULL,
						0,
						&reginfo1,
						sizeof(reginfo1)+64,
						&dwRet,
						NULL);
	if (bRet)
	{
		//::MessageBox(NULL,L"设置事件成功！\n",L"错误",MB_OK);

		reglistbox.AddString(L"    用户确认,相同则一直放行!");
	} 


	::CloseHandle(hDriver);

	OnOK();
}

void CSetReg::OnBnClickedCancel()
{
	REG_CALLBACK_INFO reginfo1={0}, regtemp1 = {0};
	HANDLE hDriver = ::CreateFile(
							SYMBOL_LINK,
							GENERIC_READ | GENERIC_WRITE,
							0,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
	if (hDriver == INVALID_HANDLE_VALUE)
	{
		::MessageBox(NULL,L"打开驱动设备对象失败！\n",L"错误",MB_OK);
	}

	HANDLE hProcessEvent = ::OpenEventW(SYNCHRONIZE, FALSE, APP_EVENT_NAME);
	if (hProcessEvent == NULL)
	{
		::MessageBox(NULL,L"打开事件失败！\n",L"错误",MB_OK);
	}

	DWORD	dwRet;
	BOOL	bRet;

	bRet = ::DeviceIoControl(
						hDriver,
						IOCTL_NTPROCDRV_SET_APPEVENT_NO,
						NULL,
						0,
						&reginfo1,
						sizeof(reginfo1)+64,
						&dwRet,
						NULL);
	if (bRet)
	{
		//::MessageBox(NULL,L"设置事件成功！\n",L"错误",MB_OK);
		reglistbox.AddString(L"    用户确认,相同则一直拒绝!");
	} 


	::CloseHandle(hDriver);
	// TODO: 在此添加控件通知处理程序代码
	OnCancel();
}

void CSetReg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码


	// TODO: 在此添加控件通知处理程序代码
}

void CSetReg::OnBnClickedButton3()
{

	// TODO: 在此添加控件通知处理程序代码
}

int CSetReg::TurnOffRegMon(void)
{

	return 0;
}

void CSetReg::OnBnClickedOk2()
{
	// TODO: 在此添加控件通知处理程序代码
		REG_CALLBACK_INFO reginfo1={0}, regtemp1 = {0};
	HANDLE hDriver = ::CreateFile(
							SYMBOL_LINK,
							GENERIC_READ | GENERIC_WRITE,
							0,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
	if (hDriver == INVALID_HANDLE_VALUE)
	{
		::MessageBox(NULL,L"打开驱动设备对象失败！\n",L"错误",MB_OK);
	}

	HANDLE hProcessEvent = ::OpenEventW(SYNCHRONIZE, FALSE, APP_EVENT_NAME);
	if (hProcessEvent == NULL)
	{
		::MessageBox(NULL,L"打开事件失败！\n",L"错误",MB_OK);
	}

	DWORD	dwRet;
	BOOL	bRet;

	bRet = ::DeviceIoControl(
						hDriver,
						IOCTL_NTPROCDRV_SET_APPEVENT_OK,
						NULL,
						0,
						&reginfo1,
						sizeof(reginfo1)+64,
						&dwRet,
						NULL);
	if (bRet)
	{
		//::MessageBox(NULL,L"设置事件成功！\n",L"错误",MB_OK);

		reglistbox.AddString(L"    用户确认,放行!");
	} 

	OnOK();

	::CloseHandle(hDriver);
}

void CSetReg::OnBnClickedCancel2()
{
	// TODO: 在此添加控件通知处理程序代码
		REG_CALLBACK_INFO reginfo1={0}, regtemp1 = {0};
	HANDLE hDriver = ::CreateFile(
							SYMBOL_LINK,
							GENERIC_READ | GENERIC_WRITE,
							0,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
	if (hDriver == INVALID_HANDLE_VALUE)
	{
		::MessageBox(NULL,L"打开驱动设备对象失败！\n",L"错误",MB_OK);
	}

	HANDLE hProcessEvent = ::OpenEventW(SYNCHRONIZE, FALSE, APP_EVENT_NAME);
	if (hProcessEvent == NULL)
	{
		::MessageBox(NULL,L"打开事件失败！\n",L"错误",MB_OK);
	}

	DWORD	dwRet;
	BOOL	bRet;

	bRet = ::DeviceIoControl(
						hDriver,
						IOCTL_NTPROCDRV_SET_APPEVENT_NO,
						NULL,
						0,
						&reginfo1,
						sizeof(reginfo1)+64,
						&dwRet,
						NULL);
	if (bRet)
	{
		//::MessageBox(NULL,L"设置事件成功！\n",L"错误",MB_OK);
		reglistbox.AddString(L"    用户确认,相同则一直拒绝!");
	} 


	::CloseHandle(hDriver);
	// TODO: 在此添加控件通知处理程序代码
	OnCancel();
}
