// MyPage2.cpp : 实现文件
//

#include "stdafx.h"
#include "main.h"
#include "MyPage2.h"
#include "HaveProc.h"
#include <algorithm> 
#include "winioctl.h"
#include "ioctls.h"
#include <string>
#define SYMBOL_LINK L"\\\\.\\ProcMon"
#define DRIVER_NAME L"ProcMon"
#define DRIVER_PATH L"WATCH.sys"

#define IOCTL_NTPROCDRV_TURNON_PROCMON CTL_CODE(FILE_DEVICE_UNKNOWN,0x0802, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NTPROCDRV_TURNOFF_PROCMON CTL_CODE(FILE_DEVICE_UNKNOWN,0x0803, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

int TurnOnProcMon=1;
extern HaveProc ProcWnd;//全局是否对话框
extern CALLBACK_INFO cbkinfo={0}, cbktemp = {0};
extern DWORD pid;
extern int lp2=1;
//extern CString procdescribe;
#include "AddProcRull.h"
#include "HttpClient.h"



//队列-------------------------------
#include <list>
using   namespace   std;   
typedef struct _PROCESS_INFO{
    WCHAR lpFilePath[4][256];//4个256长的字串
}PROCESS_INFO, *PPROCESS_INFO;
typedef list<PROCESS_INFO> ProcessList;
typedef ProcessList::iterator ITERATOR;

ProcessList List;
PROCESS_INFO pi;
ITERATOR iter = List.begin();
//队列结束-------------------------------




// CMyPage2 对话框


CListBox ProcListBox;

IMPLEMENT_DYNAMIC(CMyPage2, CPropertyPage)

CMyPage2::CMyPage2()
	: CPropertyPage(CMyPage2::IDD)
	, ProcList(_T(""))
{

}

CMyPage2::~CMyPage2()
{
}

void CMyPage2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST1, ProcListBox);
	DDX_Control(pDX, IDC_LIST3, ProcRull);
}


BEGIN_MESSAGE_MAP(CMyPage2, CPropertyPage)
	ON_BN_CLICKED(IDC_CHECK1, &CMyPage2::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON1, &CMyPage2::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CMyPage2::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON2, &CMyPage2::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON4, &CMyPage2::OnBnClickedButton4)
	ON_LBN_SELCHANGE(IDC_LIST1, &CMyPage2::OnLbnSelchangeList1)
	ON_BN_CLICKED(IDC_BUTTON5, &CMyPage2::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CMyPage2::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON8, &CMyPage2::OnBnClickedButton8)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CMyPage2 消息处理程序

//线程函数

int ProcMon2(void)
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
		MessageBox(NULL,L"打开驱动失败,请安装并启动服务!\n",L"错误",MB_OK);
		return -1;
	}

	HANDLE hProcessEvent = ::OpenEventW(SYNCHRONIZE, FALSE, EVENT_NAME);
	if (hProcessEvent == NULL)
	{
		MessageBox(NULL,L"打开事件失败！\n",L"错误",MB_OK);
		return -1;
	}
	//开启
	TurnOnProcMon=1;
	ProcListBox.AddString(L"进程监控成功开启");
	//循环
//	while (1) 
	{

		while (::WaitForSingleObject(hProcessEvent, INFINITE)==WAIT_OBJECT_0)
		{
			DWORD	dwRet;
			BOOL	bRet;

			bRet = ::DeviceIoControl(
								hDriver,
								IOCTL_NTPROCDRV_GET_PROCINFO,
								NULL,
								0,
								&cbkinfo,
								sizeof(cbkinfo)+64,
								&dwRet,
								NULL);
			if (bRet)
			{
				if (//防止重复出现
					cbkinfo.hParentId != cbktemp.hParentId || \
					cbkinfo.hProcessId != cbktemp.hProcessId || \
					cbkinfo.bCreate != cbktemp.bCreate
					)
				{
					if (cbkinfo.bCreate)//如果是创建
					{

//						MessageBox(NULL,cbkinfo.ParentProcFullPath,L"错误",MB_OK);

						
						DWORD num;


						//这里转成WCHAR	   	cbkinfo.ProcFullPath  to  	wword			
						num= MultiByteToWideChar(CP_ACP,0,cbkinfo.ProcFullPath,-1,NULL,0); //计算这个GB2312实际有几个字组成
						WCHAR *wProcFullPath;//定义一个UNICODE的指针
						wProcFullPath= (WCHAR*)calloc(num,sizeof(WCHAR));//动态的申请空间存字
						if(wProcFullPath == NULL) free(wProcFullPath);
						memset(wProcFullPath, 0, num*sizeof(WCHAR));//初始化动作
						MultiByteToWideChar(CP_ACP,0, cbkinfo.ProcFullPath, -1, wProcFullPath, num); 
						
						CString wprocname;
						wprocname.Format(L"%s",wProcFullPath);
						int n=wprocname.ReverseFind(L'\\');
						wprocname=wprocname.Right(wprocname.GetLength()-n-1);


						//这里转成WCHAR	   	wParentProcFullPath  to  wProcFullPath			
						num= MultiByteToWideChar(CP_ACP,0,cbkinfo.ParentProcFullPath,-1,NULL,0); //计算这个GB2312实际有几个字组成
						WCHAR *wParentProcFullPath;//定义一个UNICODE的指针
						wParentProcFullPath= (WCHAR*)calloc(num,sizeof(WCHAR));//动态的申请空间存字
						if(wParentProcFullPath == NULL) free(wProcFullPath);
						memset(wParentProcFullPath, 0, num*sizeof(WCHAR));//初始化动作
						MultiByteToWideChar(CP_ACP,0, cbkinfo.ParentProcFullPath, -1, wParentProcFullPath, num); 


						ProcWnd.ParentPid.Format(L"%d",cbkinfo.hParentId);
						ProcWnd.ParentPath.Format(L"%s",wParentProcFullPath);
						ProcWnd.NewProcessPid.Format(L"%d",cbkinfo.hProcessId);
						ProcWnd.NewProcessPath.Format(L"%s",wProcFullPath);
						pid = (DWORD)cbkinfo.hProcessId;

					
						if (TurnOnProcMon==1) //如果监控开启
						{
							int isfind=0;//初始未找到
							for (iter=List.begin();iter!=List.end();iter++   )  
							{
								if (wcsstr((*iter).lpFilePath[2],wProcFullPath)!=NULL)
								{
									isfind=1;
									break;
								}
							}

							if (isfind==1) //如果有记录
							{ 
								//0为永通过,1为一直不允许
								if (wcscmp((*iter).lpFilePath[0],L"0")==0)//如果为0
								{	
									KillPro(pid);//干掉
								}
								if (wcscmp((*iter).lpFilePath[0],L"1")==0)//如果为1
								{	
									//什么也不做
								}

							} 
							else //如果没有记录,则弹窗
							{ 
								SuspendPro(pid);//挂起进程

										
	/*						
								CHttpClient   conn;   
								CString   value=L"http://127.0.0.1:801/f.php?procpath=";
								value=value+wprocname;
								CString f;
								CString b;
								f=conn.doGet(value);//取值
								char * p = (char *)f.GetBuffer();
								DWORD num= MultiByteToWideChar(CP_ACP,0,p,-1,NULL,0); 
								WCHAR *wword;//定义一个UNICODE的指针
								wword= (WCHAR*)calloc(num,sizeof(WCHAR));//动态的申请空间存字
								memset(wword, 0, num*sizeof(WCHAR));       //初始化动作
								MultiByteToWideChar(CP_ACP,0, p, -1, wword, num); 

								b.Format(L"%s",wword);
								ProcWnd.procdescribe.Format(L"%s",wword);
	*/
								//ProcWnd.UpdateData(true);
								ProcWnd.procdescribe.Format(L"无网络服器测试版，不支持联网获取描述信息。请慎重选择放行！");
								ProcWnd.DoModal();//弹窗
							} 					
							isfind=0;//恢复全局变量
							
						}
						//添加到listbox
						CString procstrtemp;
						procstrtemp.Format(L"进程PID%d:(%S)",cbkinfo.hParentId,cbkinfo.ParentProcFullPath);
						ProcListBox.AddString(procstrtemp);
						procstrtemp.Format(L"  新进程创建PID:%d:(%S)",cbkinfo.hProcessId,cbkinfo.ProcFullPath);
						ProcListBox.AddString(procstrtemp);
						ProcListBox.AddString(L"");
					} 
					else
					{
						CString procstrtemp;
						procstrtemp.Format(L"进程结束PID:%d:(%S)",cbkinfo.hProcessId,cbkinfo.ProcFullPath);
						ProcListBox.AddString(procstrtemp);
						ProcListBox.AddString(L"");
					}

					cbktemp = cbkinfo;
				}
			} 
			else
			{
				CString procstrtemp;
				procstrtemp.Format(L"获取进程信息失败");
				ProcListBox.AddString(procstrtemp);
				break;
			}
		}
	}
	::CloseHandle(hDriver);

	return 0;

}


UINT ThreadProc2(LPVOID lpParam)
{
	
//	MessageBox(NULL,L"进入线程",L"线程1",MB_OK);
	int ret=ProcMon2();
	return 0;
}

void CMyPage2::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CMyPage2::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	TurnOnProcMon=1;
	ProcListBox.AddString(L"进程监控成功开启");
	//HANDLE hProc=AfxBeginThread(ThreadProc2,&lp2);
}

void CMyPage2::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	ProcListBox.AddString(L"进程添加测试");
}

void CMyPage2::OnBnClickedButton2()
{
	TurnOnProcMon=0;
	DWORD	dwRet;
	BOOL	bRet;
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
		::MessageBox(NULL,L"打开驱动失败,请安装并启动服务!\n",L"错误",MB_OK);
		return ;
	}

	bRet = ::DeviceIoControl(
						hDriver,
						IOCTL_NTPROCDRV_TURNOFF_PROCMON,
						NULL,
						0,
						&cbkinfo,
						sizeof(cbkinfo)+64,
						&dwRet,
						NULL);
	if (bRet)
	{
		CString procstrtemp;
		procstrtemp.Format(L"成功关闭进程监控");
		ProcListBox.AddString(procstrtemp);		
	} 
	else
	{
		CString procstrtemp;
		procstrtemp.Format(L"关闭进程监控失败");
		ProcListBox.AddString(procstrtemp);
	}

	::CloseHandle(hDriver);

	return ;
	// TODO: 在此添加控件通知处理程序代码
}

void CMyPage2::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
	ProcListBox.ResetContent();
}

void CMyPage2::OnLbnSelchangeList1()
{
	// TODO: 在此添加控件通知处理程序代码
}

BOOL CMyPage2::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  在此添加额外的初始化
	ProcRull.InsertColumn ( 0, L"ID",LVCFMT_LEFT, 30 ) ;
	ProcRull.InsertColumn ( 1, L"是否通过",LVCFMT_CENTER, 80 ) ;
	ProcRull.InsertColumn ( 2, L"父进程名",LVCFMT_CENTER, 100 ) ;
	ProcRull.InsertColumn ( 3, L"进程路径",LVCFMT_LEFT, 500 ) ;
	ProcRull.InsertColumn ( 4, L"父进程路径",LVCFMT_LEFT, 500 ) ;
	ProcRull.SetExtendedStyle(     LVS_EX_FULLROWSELECT   |   LVS_EX_GRIDLINES   |LVS_EX_HEADERDRAGDROP   |   LVS_EX_ONECLICKACTIVATE     );   



	this->ProcRull.SetExtendedStyle ( this->ProcRull.GetExtendedStyle()|LVS_EX_FULLROWSELECT) ;

	this->db.Open ( L"rulls.mdb", FALSE, FALSE, NULL ) ;
	this->OnBnClickedFlush () ;


	//开启线程
	HANDLE hProc=AfxBeginThread(ThreadProc2,&lp2);
	return TRUE;  // return TRUE unless you set the focus to a control

	// 异常: OCX 属性页应返回 FALSE
}

void CMyPage2::OnBnClickedFlush(void)
{
	// 检测数据库是否已连接
	if ( this->db.IsOpen () == FALSE )
	{
		this->MessageBox ( L"未连接数据库！" ) ;
		return ;
	}

	// 删除列表控件所有记录
	this->ProcRull.DeleteAllItems () ;
	List.clear();
	// 定义记录集对象，并打开记录集
	CDaoRecordset	Record ( &this->db ) ;
	Record.Open ( dbOpenDynaset, L"SELECT * FROM procrull" ) ;

	UINT		nIndex = 0 ;
	CString		TempStr ;
	COleVariant	OleVariant ;

	// 移动到第一条记录
	Record.MoveFirst () ;
	while( !Record.IsEOF() )//全部列出来
	{
		// 在列表控件添加记录
		TempStr.Format ( L"%d", nIndex+1 ) ;
		this->ProcRull.InsertItem ( nIndex, TempStr ) ;

		for ( int i = 1; i <= 4; i++ )
		{//一行里有4列
			Record.GetFieldValue ( i, OleVariant ) ;
			this->ProcRull.SetItemText ( nIndex, i, OleVariant.bstrVal ) ;
			wcscpy(pi.lpFilePath[i-1],OleVariant.bstrVal);
		}
		//加入队列,成功
		List.push_back(pi);


//		Record.GetFieldValue ( 4, OleVariant ) ;
//		TempStr.Format ( L"%d", OleVariant.uintVal ) ;
//		this->ProcRull.SetItemText ( nIndex, 4, TempStr ) ;

		// 移到下一条记录
		Record.MoveNext () ;
		nIndex++ ;
	}
	// 关闭记录集
	Record.Close () ;

}




void CMyPage2::OnBnClickedButton5()
{
	// TODO: 在此添加控件通知处理程序代码
	// 检测数据库是否已连接
	if ( this->db.IsOpen () == FALSE )
	{
		this->MessageBox ( L"未连接数据库！" ) ;
		return ;
	}

	// 定义记录集对象，打开记录集
	CDaoRecordset Record ( &this->db ) ;
	Record.Open ( dbOpenDynaset, L"SELECT * FROM procrull" ) ;

	// 定义对话框对象，并设置为“添加”模式
	AddProcRull AddBookDlg ;


	if ( AddBookDlg.DoModal() == IDOK )
	{
		// 使用CDaoRecordset::AddNew函数表明要开始添加新记录
		Record.AddNew () ;
		// 设置新记录
		Record.SetFieldValue ( 1, AddBookDlg.pass.GetBuffer() ) ;
		Record.SetFieldValue ( 2, AddBookDlg.parentprocname.GetBuffer() ) ;
		Record.SetFieldValue ( 3, AddBookDlg.procpath.GetBuffer() ) ;
		Record.SetFieldValue ( 4, AddBookDlg.parentprocpath.GetBuffer() ) ;
		// 只有在执行CDaoRecordset::Update函数后才会写入到数据库
		Record.Update () ;
		// 关闭记录集
		Record.Close () ;
		// 刷新界面列表控件数据显示
		this->OnBnClickedFlush () ;
	}


}



void CMyPage2::OnBnClickedButton6()
{
	// TODO: 在此添加控件通知处理程序代码
	if ( this->db.IsOpen () == FALSE )
	{
		this->MessageBox ( L"未连接数据库！" ) ;
		return ;
	}

	// 检测是否已选择
	int nCurSel = this->ProcRull.GetNextItem ( -1,LVNI_SELECTED) ;
	if ( nCurSel == -1 )
	{
		this->MessageBox ( L"没有选择！" ) ;
		return ;
	}

	// 定义记录集对象，并移动指针到指定位置
	CDaoRecordset Record ( &this->db ) ;
	Record.Open ( dbOpenDynaset, L"SELECT * FROM procrull" ) ;
	Record.Move ( nCurSel ) ;
	// 删除当前记录
	Record.Delete () ;
	// 关闭记录集
	Record.Close () ;
	// 刷新界面列表控件数据显示
	this->OnBnClickedFlush () ;

}



void CMyPage2::OnBnClickedButton8()
{
	// TODO: 在此添加控件通知处理程序代码
	//此测试数据库
	CDaoRecordset Record ( &this->db ) ;

	Record.Open ( dbOpenDynaset, L"SELECT Author FROM procrull WHERE BookName='加密与解密'" ) ;
	COleVariant	OleVariant ;

	Record.GetFieldValue ( 0, OleVariant ) ;
	this->MessageBox(OleVariant.bstrVal);

}

void CMyPage2::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CPropertyPage::OnPaint()
}
