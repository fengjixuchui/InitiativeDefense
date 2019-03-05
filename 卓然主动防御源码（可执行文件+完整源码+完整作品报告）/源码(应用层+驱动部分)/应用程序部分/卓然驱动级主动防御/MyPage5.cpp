// MyPage5.cpp : 实现文件
//

#include "stdafx.h"
#include "MyPage5.h"


// CMyPage5 对话框
CListBox FileList;

#include "fileioctl.h"
#include "winioctl.h"
#define SYMBOL_LINK     L"\\\\.\\FileMon"
int filelp=0;
FILE_CALLBACK_INFO fileinfo={0}, filetemp = {0};


int FileMon(void)
{
//	FileList.AddString (L"进入线程函数");
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
		MessageBox(NULL,L"打开驱动失败,请安装并启动服务!\n",L"错误",MB_OK);
		return -1;
	}

	HANDLE hProcessEvent = ::OpenEventW(SYNCHRONIZE, FALSE, FILE_EVENT_NAME);
	if (hProcessEvent == NULL)
	{
		MessageBox(NULL,L"打开事件失败！\n",L"错误",MB_OK);
		return -1;
	}

	while (::WaitForSingleObject(hProcessEvent, INFINITE)==WAIT_OBJECT_0)
	{
		DWORD	dwRet;
		BOOL	bRet;

		bRet = ::DeviceIoControl(
							hDriver,
							IOCTL_GET_FILEINFO,
							NULL,
							0,
							&fileinfo,
							sizeof(fileinfo),
							&dwRet,
							NULL);

		if (bRet)
		{
			if (strcmp(fileinfo.ProcFullPath,filetemp.ProcFullPath)!=0)//避免重复的
			{
				CString strtemp;



/////////////////////////char *   to   WCHAR *////////////////////////////////////////
				DWORD num;
				WCHAR *wProcName;//定义一个UNICODE的指针
				WCHAR *wProcFullPath;//定义一个UNICODE的指针
				WCHAR *wFilePre;//定义一个UNICODE的指针
				WCHAR *wFilePath;//定义一个UNICODE的指针

				num= MultiByteToWideChar(CP_ACP,0,fileinfo.ProcName,-1,NULL,0);
				wProcName= (WCHAR*)calloc(num,sizeof(WCHAR));
				if(wProcName == NULL) free(wProcName);
				memset(wProcName, 0, num*sizeof(WCHAR));
				MultiByteToWideChar(CP_ACP,0, fileinfo.ProcName, -1, wProcName, num); 

				num= MultiByteToWideChar(CP_ACP,0,fileinfo.ProcFullPath,-1,NULL,0); 
				wProcFullPath= (WCHAR*)calloc(num,sizeof(WCHAR));
				if(wProcFullPath == NULL) free(wProcFullPath);
				memset(wProcFullPath, 0, num*sizeof(WCHAR));
				MultiByteToWideChar(CP_ACP,0, fileinfo.ProcFullPath, -1, wProcFullPath, num); 
	
				num= MultiByteToWideChar(CP_ACP,0,fileinfo.FilePre,-1,NULL,0);
				wFilePre= (WCHAR*)calloc(num,sizeof(WCHAR));
				if(wFilePre == NULL) free(wFilePre);
				memset(wFilePre, 0, num*sizeof(WCHAR));
				MultiByteToWideChar(CP_ACP,0, fileinfo.FilePre, -1, wFilePre, num); 
	
				num= MultiByteToWideChar(CP_ACP,0,fileinfo.FilePath,-1,NULL,0);
				wFilePath= (WCHAR*)calloc(num,sizeof(WCHAR));
				if(wFilePath == NULL) free(wFilePath);
				memset(wFilePath, 0, num*sizeof(WCHAR));
				MultiByteToWideChar(CP_ACP,0, fileinfo.FilePath, -1, wFilePath, num); 
////////////////////////////////////////////////////////////////////////////////////////////



				strtemp.Format (L"进程:%s(%s)",wProcName,wProcFullPath);
				FileList.AddString(strtemp);
				strtemp.Format (L"    修改文件%s%s",wFilePre,wFilePath);
				FileList.AddString(strtemp);

				filetemp = fileinfo;
			}

		} 
		else
		{
			CString strtemp;
			strtemp.Format(L"通信失败,原因不明");
			FileList.AddString(strtemp);			
			break;
		}
		
	}

	return 0;
}


UINT ThreadProcFileMon1(LPVOID lpParam)
{
//	FileList.AddString (L"进入线程函数ThreadProcFileMon");
	int ret=FileMon();
	return 0;
}




IMPLEMENT_DYNAMIC(CMyPage5, CPropertyPage)

CMyPage5::CMyPage5()
	: CPropertyPage(CMyPage5::IDD)

{

}

CMyPage5::~CMyPage5()
{
}

void CMyPage5::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, FileList);
}


BEGIN_MESSAGE_MAP(CMyPage5, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON1, &CMyPage5::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMyPage5::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CMyPage5::OnBnClickedButton3)
END_MESSAGE_MAP()


// CMyPage5 消息处理程序

void CMyPage5::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
//	HANDLE hProc=AfxBeginThread(ThreadProcFileMon1,&filelp);
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
		this->MessageBox(L"打开驱动失败,请安装并启动服务!\n");
		return ;
	}

	DWORD	dwRet;
	BOOL	bRet;
	

	bRet = ::DeviceIoControl(
						hDriver,
						IOCTL_TURNON_FILEMON,
						NULL,
						0,
						&fileinfo,
						sizeof(fileinfo),
						&dwRet,
						NULL);

	if (bRet)
	{
			FileList.AddString(L"文件监控开启成功");
	} 
	else
	{
		FileList.AddString (L"文件监控开启失败");
	}

}

void CMyPage5::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
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
		this->MessageBox(L"打开驱动失败,请安装并启动服务!\n");
		return ;
	}

	DWORD	dwRet;
	BOOL	bRet;

	bRet = ::DeviceIoControl(
						hDriver,
						IOCTL_TURNOFF_FILEMON,
						NULL,
						0,
						&fileinfo,
						sizeof(fileinfo),
						&dwRet,
						NULL);

	if (bRet)
	{
		this->MessageBox (L"文件监控关闭成功");
	} 
	else
	{
		this->MessageBox (L"文件监控关闭失败");
	}
	return ;
}

void CMyPage5::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	FileList.ResetContent();
}

BOOL CMyPage5::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  在此添加额外的初始化
	HANDLE hProc=AfxBeginThread(ThreadProcFileMon1,&filelp);
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
		//this->MessageBox(L"打开驱动失败,请安装并启动服务!,请安装并启动服务!！\n");
		return 0;
	}

	DWORD	dwRet;
	BOOL	bRet;
	

	bRet = ::DeviceIoControl(
						hDriver,
						IOCTL_TURNON_FILEMON,
						NULL,
						0,
						&fileinfo,
						sizeof(fileinfo),
						&dwRet,
						NULL);

	if (bRet)
	{
			FileList.AddString(L"文件监控开启成功");
	} 
	else
	{
		FileList.AddString (L"文件监控开启失败");
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
