// MyPage1.cpp : 实现文件
//

#include "stdafx.h"
#include "main.h"
#include "MyPage1.h"
#include <winsvc.h>  
#include "ioctls.h"

//#include "getmd5.cpp"



#include "winioctl.h"
#define SYMBOL_LINK L"\\\\.\\ProcMon"
#define PROC_DRIVER_NAME L"ProcMon"
#define PROC_DRIVER_PATH L"procmon.sys"

#define REG_DRIVER_NAME L"RegMon"
#define REG_DRIVER_PATH L"regmon.sys"

#define MEM_DRIVER_NAME L"MemMon"
#define MEM_DRIVER_PATH L"memmon.sys"

#define FILE_DRIVER_NAME L"FileMon"
#define FILE_DRIVER_PATH L"filemon.sys"
// CMyPage1 对话框
HaveProc ProcWnd;//全局是否对话框
static CALLBACK_INFO cbkinfo={0}, cbktemp = {0};
DWORD pid;

CListBox MainList;

IMPLEMENT_DYNAMIC(CMyPage1, CPropertyPage)

CMyPage1::CMyPage1()
	: CPropertyPage(CMyPage1::IDD)
{


}

CMyPage1::~CMyPage1()
{
}

void CMyPage1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, Stat);
	DDX_Control(pDX, IDC_LIST1,MainList);
}


BEGIN_MESSAGE_MAP(CMyPage1, CPropertyPage)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON2, &CMyPage1::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CMyPage1::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON1, &CMyPage1::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON4, &CMyPage1::OnBnClickedButton4)
	ON_EN_CHANGE(IDC_EDIT1, &CMyPage1::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_BUTTON7, &CMyPage1::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON6, &CMyPage1::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON8, &CMyPage1::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, &CMyPage1::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON10, &CMyPage1::OnBnClickedButton10)
	ON_BN_CLICKED(IDC_BUTTON12, &CMyPage1::OnBnClickedButton12)
	ON_BN_CLICKED(IDC_BUTTON11, &CMyPage1::OnBnClickedButton11)
END_MESSAGE_MAP()


// CMyPage1 消息处理程序

void CMyPage1::OnPaint()
{

		// 取得窗口客户区域大小
		CRect WndRect ;
		this->GetWindowRect ( &WndRect ) ;
		this->ScreenToClient ( &WndRect ) ;
	

		CPaintDC   dc(this);   
		CBitmap   bitmap;   
		bitmap.LoadBitmap(IDB_BITMAP1);   
		HBITMAP   OldBitmap;   
		CDC   MemDC;CRect   rect;   
		MemDC.CreateCompatibleDC(&dc);   
		GetClientRect(rect);   
		OldBitmap=(HBITMAP)MemDC.SelectObject(&bitmap);   
		dc.BitBlt(0,0,603,380,&MemDC,0,0,SRCCOPY);   
		MemDC.SelectObject(OldBitmap);
	
		CDialog::OnPaint();
	
}

bool CMyPage1::LoadNTDriver(LPCWSTR lpszDriverName, LPCWSTR lpszDriverPath)
{
	WCHAR szDriverImagePath[256];
	//得到完整的驱动路径
	GetFullPathName(lpszDriverPath, 256, szDriverImagePath, NULL);

	bool bRet = FALSE;

	SC_HANDLE hServiceMgr=NULL;//SCM管理器的句柄
	SC_HANDLE hServiceDDK=NULL;//NT驱动程序的服务句柄

	//打开服务控制管理器
	hServiceMgr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );

	if( hServiceMgr == NULL )  
	{
		//OpenSCManager失败
		//printf( "OpenSCManager() Faild %d ! \n", GetLastError() );
		bRet = FALSE;
		goto BeforeLeave;
	}
	
	//创建驱动所对应的服务
	hServiceDDK = CreateService( hServiceMgr,
		lpszDriverName, //驱动程序的在注册表中的名字  
		lpszDriverName, // 注册表驱动程序的 DisplayName 值  
		SERVICE_ALL_ACCESS, // 加载驱动程序的访问权限  
		SERVICE_KERNEL_DRIVER,// 表示加载的服务是驱动程序  
		SERVICE_DEMAND_START, // 注册表驱动程序的 Start 值  
		SERVICE_ERROR_IGNORE, // 注册表驱动程序的 ErrorControl 值  
		szDriverImagePath, // 注册表驱动程序的 ImagePath 值  
		NULL,  
		NULL,  
		NULL,  
		NULL,  
		NULL);  

	DWORD dwRtn;
	//判断服务是否失败
	if( hServiceDDK == NULL )  
	{  
		dwRtn = GetLastError();
		if( dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS )  
		{  
			//由于其他原因创建服务失败
			MainList.AddString(L"由于其他原因创建服务失败! \n");  
			bRet = FALSE;
			goto BeforeLeave;
		}  
		else  
		{
			//服务创建失败，是由于服务已经创立过
			MainList.AddString(L"服务创建失败，是由于服务已经创立过! \n");  
		}

		// 驱动程序已经加载，只需要打开  
		hServiceDDK = OpenService( hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS );  
		if( hServiceDDK == NULL )  
		{
			//如果打开服务也失败，则意味错误
			dwRtn = GetLastError();  
			this->MessageBox(L"打开服务失败! \n");  
			bRet = FALSE;
			goto BeforeLeave;
		}  
		else 
		{

		}
	}  
	else  
	{

	}

	bRet = TRUE;
//离开前关闭句柄
BeforeLeave:
	if(hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if(hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;
}



bool CMyPage1::Startservice(LPCWSTR lpszDriverName, LPCWSTR lpszDriverPath)
{
	
	WCHAR szDriverImagePath[256];
	//得到完整的驱动路径
	GetFullPathName(lpszDriverPath, 256, szDriverImagePath, NULL);

	BOOL bRet = FALSE;

	SC_HANDLE hServiceMgr=NULL;//SCM管理器的句柄
	SC_HANDLE hServiceDDK=NULL;//NT驱动程序的服务句柄

	//打开服务控制管理器
	hServiceMgr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );

	if( hServiceMgr == NULL )  
	{
		//OpenSCManager失败
		this->MessageBox(L"打开SCManager()失败! \n");
		bRet = FALSE;
		goto BeforeLeave;
	}

	hServiceDDK = OpenService( hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS );  
	if( hServiceDDK == NULL )  
	{
		//如果打开服务也失败，则意味错误
		DWORD dwRtn = GetLastError();  
		this->MessageBox(L"打开服务失败! \n");  
		bRet = FALSE;
		goto BeforeLeave;
	}  
	//开启服务
	bRet= StartService( hServiceDDK, NULL, NULL );  
	if( !bRet )  
	{  
		DWORD dwRtn = GetLastError();  
		if( dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING )  
		{  
			this->MessageBox(L"开启服务失败! \n");  
			bRet = FALSE;
			goto BeforeLeave;
		}  
		else  
		{  
			if( dwRtn == ERROR_IO_PENDING )  
			{  
				//设备被挂住
				this->MessageBox(L"StartService() Faild ERROR_IO_PENDING ! \n");
				bRet = FALSE;
				goto BeforeLeave;
			}  
			else  
			{  
				//服务已经开启
				MainList.AddString(L"服务已经开启! \n");
				bRet = TRUE;
				goto BeforeLeave;
			}  
		}  
	}
	bRet = TRUE;

BeforeLeave:
	if(hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if(hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;
}








bool CMyPage1::UnloadNTDriver(PCWSTR szSvrName)
{
	bool bRet = FALSE;
	SC_HANDLE hServiceMgr=NULL;//SCM管理器的句柄
	SC_HANDLE hServiceDDK=NULL;//NT驱动程序的服务句柄
	SERVICE_STATUS SvrSta;
	//打开SCM管理器
	hServiceMgr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );  
	if( hServiceMgr == NULL )  
	{
		//带开SCM管理器失败
		this->MessageBox( L"打开SCM管理器失败! \n");  
		bRet = FALSE;
		goto BeforeLeave;
	}  

	//打开驱动所对应的服务
	hServiceDDK = OpenService( hServiceMgr, szSvrName, SERVICE_ALL_ACCESS );  

	if( hServiceDDK == NULL )  
	{
		//打开驱动所对应的服务失败
		this->MessageBox( L"打开驱动所对应的服务失败,因为驱动没有安装! \n");  
		bRet = FALSE;
		goto BeforeLeave;
	}  

	//停止驱动程序，如果停止失败，只有重新启动才能，再动态加载。  
	if( !ControlService( hServiceDDK, SERVICE_CONTROL_STOP , &SvrSta ) )  
	{  
//		this->MessageBox( L"停止驱动程序失败!\n");  
	}  
	else  
	{
		//打开驱动所对应的失败
	//	this->MessageBox( L"打开驱动所对应的失败\n" );  
	}  
	//动态卸载驱动程序。  
	if( !DeleteService( hServiceDDK ) )  
	{
		//卸载失败
		this->MessageBox( L"卸载失败\n");  
		goto BeforeLeave;
	}  

	bRet = TRUE;
BeforeLeave:
//离开前关闭打开的句柄
	if(hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if(hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;	
}




bool CMyPage1::Stopservice(PCWSTR szSvrName)
{
	bool bRet = FALSE;
	SC_HANDLE hServiceMgr=NULL;//SCM管理器的句柄
	SC_HANDLE hServiceDDK=NULL;//NT驱动程序的服务句柄
	SERVICE_STATUS SvrSta;
	//打开SCM管理器
	hServiceMgr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );  
	if( hServiceMgr == NULL )  
	{
		//带开SCM管理器失败
		this->MessageBox( L"打开SCM管理器失败! \n");  
		bRet = FALSE;
		goto BeforeLeave;
	}  
	//打开驱动所对应的服务
	hServiceDDK = OpenService( hServiceMgr, szSvrName, SERVICE_ALL_ACCESS );  

	if( hServiceDDK == NULL )  
	{
		//打开驱动所对应的服务失败
		this->MessageBox( L"打开驱动所对应的服务失败,因为驱动没有安装! \n");  
		bRet = FALSE;
		goto BeforeLeave;
	}  
	//停止驱动程序，如果停止失败，只有重新启动才能，再动态加载。  
	if( !ControlService( hServiceDDK, SERVICE_CONTROL_STOP , &SvrSta ) )  
	{  
		this->MessageBox( L"停止驱动程序失败!\n");  
	} 

	bRet = TRUE;
BeforeLeave:
//离开前关闭打开的句柄
	if(hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if(hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;	

}





void CMyPage1::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	bool Ret=Startservice(PROC_DRIVER_NAME,PROC_DRIVER_PATH);
	if (Ret) {
		MainList.AddString(L"进程监控启动服务成功");
	}
	else 
	{
		this->MessageBox(L"启动进程监控服务失败,因为驱动没有安装");
	}
}



void CMyPage1::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	bool Ret=Stopservice(PROC_DRIVER_NAME);
	if (Ret) {
		MainList.AddString(L"进程监控服务停止成功");
	}
}



void CMyPage1::OnBnClickedButton3()
{

	bool Ret=LoadNTDriver(PROC_DRIVER_NAME,PROC_DRIVER_PATH);
	if (Ret) {
		MainList.AddString(L"进程监控服务安装成功");
	}
	Ret=LoadNTDriver(REG_DRIVER_NAME,REG_DRIVER_PATH);
	if (Ret) {
		MainList.AddString(L"注册表监控服务安装成功");
	}
	Ret=LoadNTDriver(MEM_DRIVER_NAME,MEM_DRIVER_PATH);
	if (Ret) {
		MainList.AddString(L"内存监控服务安装成功");
	}
	Ret=LoadNTDriver(FILE_DRIVER_NAME,FILE_DRIVER_PATH);
	if (Ret) {
		MainList.AddString(L"文件监控服务安装成功");
	}
}


void CMyPage1::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
	bool bRet=UnloadNTDriver(PROC_DRIVER_NAME);
	if (bRet){
		MainList.AddString(L"进程服务卸载成功");
	}
	bRet=UnloadNTDriver(REG_DRIVER_NAME);
	if (bRet){
		MainList.AddString(L"注册表服务卸载成功");
	}
	bRet=UnloadNTDriver(MEM_DRIVER_NAME);
	if (bRet){
		MainList.AddString(L"内存服务卸载成功");
	}
	bRet=UnloadNTDriver(FILE_DRIVER_NAME);
	if (bRet){
		MainList.AddString(L"文件服务卸载成功");
	}
}



void CMyPage1::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CPropertyPage::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


#include "haveproc.h"


void CMyPage1::OnBnClickedButton7()
{
	// TODO: 在此添加控件通知处理程序代码
	//测试取MD5
	//char *str="c:\\ch.php";
	//md5main(1,&str);

	//blg3.DoModal();
	//HaveProc pr;
	//pr.DoModal();

}

void CMyPage1::OnBnClickedButton6()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL Ret=Startservice(REG_DRIVER_NAME,REG_DRIVER_PATH);
	if (Ret) {
		MainList.AddString(L"注册表监控启动服务成功");

	}
	else 
	{
		this->MessageBox(L"启动注册表监控服务失败,因为驱动没有安装");
	}
}

void CMyPage1::OnBnClickedButton8()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL Ret=Stopservice(REG_DRIVER_NAME);
	if (Ret) {
		MainList.AddString(L"注册表监控服务停止成功");
	}
}

void CMyPage1::OnBnClickedButton9()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL Ret=Startservice(MEM_DRIVER_NAME,MEM_DRIVER_PATH);
	if (Ret) {
		MainList.AddString(L"内存加载监控启动服务成功");
	}
	else 
	{
		this->MessageBox(L"内存加载监控服务启动失败,因为驱动没有安装");
	}
}

void CMyPage1::OnBnClickedButton10()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL Ret=Stopservice(MEM_DRIVER_NAME);
	if (Ret) {
		MainList.AddString(L"内存监控服务停止成功");
	}
}

void CMyPage1::OnBnClickedButton12()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL Ret=Stopservice(MEM_DRIVER_NAME);
	if (Ret) {
		MainList.AddString(L"文件监控服务停止成功");
	}
}

void CMyPage1::OnBnClickedButton11()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL Ret=Startservice(FILE_DRIVER_NAME,FILE_DRIVER_PATH);
	if (Ret) {
		MainList.AddString(L"文件监控启动服务成功");
		UpdateData(false);

	}
	else 
	{
		this->MessageBox(L"文件加载监控服务启动失败,因为驱动没有安装");
	}
}


