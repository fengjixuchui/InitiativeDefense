// MyPage4.cpp : 实现文件
//

#include "stdafx.h"
#include "main.h"
#include "MyPage4.h"

#include "memioctl.h"
#include "winioctl.h"
#define SYMBOL_LINK     L"\\\\.\\MemMon"
#define NT_DEVICE_NAME      L"\\Device\\MemMon"
#define DOS_DEVICE_NAME     L"\\DosDevices\\MemMon"
#define MEM_EVENT_NAME L"MemEvent"  //驱动层通知事件
#define APP_EVENT_NAME L"MenAppEvent" //应用层是否事件


int memlp=0;
CListBox MemList;
MEM_CALLBACK_INFO meminfo={0}, memtemp = {0};



int MemMon(void)
{
	// 打开驱动设备对象
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
		MessageBox(NULL,L"打开驱动失败,请安装并启动服务！\n",L"错误",MB_OK);
		return -1;
	}

	HANDLE hProcessEvent = ::OpenEventW(SYNCHRONIZE, FALSE, MEM_EVENT_NAME);
	if (hProcessEvent == NULL)
	{
		MessageBox(NULL,L"打开事件失败！\n",L"错误",MB_OK);
		return -1;
	}
	//开启监控
	MemList.AddString(L"内存加载监控开启成功");
	while (::WaitForSingleObject(hProcessEvent, INFINITE)==WAIT_OBJECT_0)
	{
		DWORD	dwRet;
		BOOL	bRet;

		bRet = ::DeviceIoControl(
							hDriver,
							IOCTL_GET_MEMINFO,
							NULL,
							0,
							&meminfo,
							sizeof(meminfo)+64,
							&dwRet,
							NULL);

		if (bRet)
		{
			if (strcmp(meminfo.ProcFullPath,memtemp.ProcFullPath)!=0)
			{
//				CString strtemp;
				DWORD num;
				WCHAR *wProcFullPath;//定义一个UNICODE的指针

				//这里转成WCHAR	   	wParentProcFullPath  to  wProcFullPath			
				num= MultiByteToWideChar(CP_ACP,0,meminfo.ProcFullPath,-1,NULL,0); //计算这个GB2312实际有几个字组成
				wProcFullPath= (WCHAR*)calloc(num,sizeof(WCHAR));//动态的申请空间存字
				if(wProcFullPath == NULL) free(wProcFullPath);
				memset(wProcFullPath, 0, num*sizeof(WCHAR));//初始化动作
				MultiByteToWideChar(CP_ACP,0, meminfo.ProcFullPath, -1, wProcFullPath, num);


//				strtemp.Format (L"%S",wProcFullPath);
				MemList.AddString(wProcFullPath);


	//			SetListBoxHScroll();
				memtemp = meminfo;
			}

		} 
		else
		{
			break;
		}
		
	}

	return 0;
}


UINT ThreadProcMemMon1(LPVOID lpParam)
{

	int ret=MemMon();
	return 0;
}

// CMyPage4 对话框

IMPLEMENT_DYNAMIC(CMyPage4, CPropertyPage)

CMyPage4::CMyPage4()
	: CPropertyPage(CMyPage4::IDD)
	, data1(0)
	, data2(0)
	, data3(0)
{

}

CMyPage4::~CMyPage4()
{
}

void CMyPage4::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, MemList);

}


BEGIN_MESSAGE_MAP(CMyPage4, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON1, &CMyPage4::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMyPage4::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CMyPage4::OnBnClickedButton3)
END_MESSAGE_MAP()


// CMyPage4 消息处理程序

void CMyPage4::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	MemList.AddString(L"内存加载监控开启成功");
//	HANDLE hProc=AfxBeginThread(ThreadProcMemMon1,&memlp);

}


void CMyPage4::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
}



BOOL CMyPage4::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  在此添加额外的初始化
	HANDLE hProc=AfxBeginThread(ThreadProcMemMon1,&memlp);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CMyPage4::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	MemList.ResetContent();
}
