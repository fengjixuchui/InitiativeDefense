// MyPage3.cpp : 实现文件
//

#include "stdafx.h"
#include "main.h"
#include "MyPage3.h"

#include "SetReg.h"
#include "regioctl.h"
#include "winioctl.h"

#include "HttpClient.h"
#define SYMBOL_LINK     L"\\\\.\\RegMon"
#define REG_EVENT_NAME L"RegEvent"
#define APP_EVENT_NAME L"RegAppEvent" //应用层是否事件

CSetReg RegWnd;//弹出窗口
int reglp=0;
CListBox reglistbox;



REG_CALLBACK_INFO reginfo={0}, regtemp = {0};
// CMyPage3 对话框


//队列-------------------------------
#include <list>
#include <string>
using   namespace   std; 

typedef struct _REG_INFO{
    WCHAR lpFilePath[4][256];//4个256长的字串
}REG_INFO, *PREG_INFO;

typedef list<REG_INFO> REGRullList;
typedef REGRullList::iterator REGITERATOR;

REGRullList RegRullList;
REG_INFO ri;
REGITERATOR regiter = RegRullList.begin();
//队列结束-------------------------------


//数据库---------------------------------



#include "AddRegRull.h"


int RegMon(void)
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
		MessageBox(NULL,L"打开驱动设备对象失败！\n",L"错误",MB_OK);
		return -1;
	}

	HANDLE hProcessEvent = ::OpenEventW(SYNCHRONIZE, FALSE, REG_EVENT_NAME);
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
							IOCTL_NTPROCDRV_GET_REGINFO,
							NULL,
							0,
							&reginfo,
							sizeof(reginfo)+64,
							&dwRet,
							NULL);

		if (bRet)
		{

						
			DWORD num;
			WCHAR *wProcName;//定义一个UNICODE的指针
			WCHAR *wProcFullPath;//定义一个UNICODE的指针
			WCHAR *wInfo1;//定义一个UNICODE的指针
			WCHAR *wInfo2;//定义一个UNICODE的指针


			//这里转成WCHAR	   	cbkinfo.ProcFullPath  to  	wword			
			num= MultiByteToWideChar(CP_ACP,0,reginfo.ProcName,-1,NULL,0); //计算这个GB2312实际有几个字组成
			wProcName= (WCHAR*)calloc(num,sizeof(WCHAR));//动态的申请空间存字
			if(wProcName == NULL) free(wProcName);
			memset(wProcName, 0, num*sizeof(WCHAR));//初始化动作
			MultiByteToWideChar(CP_ACP,0, reginfo.ProcName, -1, wProcName, num); 
			

			//这里转成WCHAR	   	wParentProcFullPath  to  wProcFullPath			
			num= MultiByteToWideChar(CP_ACP,0,reginfo.ProcFullPath,-1,NULL,0); //计算这个GB2312实际有几个字组成
			wProcFullPath= (WCHAR*)calloc(num,sizeof(WCHAR));//动态的申请空间存字
			if(wProcFullPath == NULL) free(wProcFullPath);
			memset(wProcFullPath, 0, num*sizeof(WCHAR));//初始化动作
			MultiByteToWideChar(CP_ACP,0, reginfo.ProcFullPath, -1, wProcFullPath, num); 

			//这里转成WCHAR	   	wParentProcFullPath  to  wProcFullPath			
			num= MultiByteToWideChar(CP_ACP,0,reginfo.Info1,-1,NULL,0); //计算这个GB2312实际有几个字组成
			wInfo1= (WCHAR*)calloc(num,sizeof(WCHAR));//动态的申请空间存字
			if(wInfo1 == NULL) free(wInfo1);
			memset(wInfo1, 0, num*sizeof(WCHAR));//初始化动作
			MultiByteToWideChar(CP_ACP,0, reginfo.Info1, -1, wInfo1, num); 

			//这里转成WCHAR	   	wParentProcFullPath  to  wProcFullPath			
			num= MultiByteToWideChar(CP_ACP,0,reginfo.Info2,-1,NULL,0); //计算这个GB2312实际有几个字组成
			wInfo2= (WCHAR*)calloc(num,sizeof(WCHAR));//动态的申请空间存字
			if(wInfo2 == NULL) free(wInfo2);
			memset(wInfo2, 0, num*sizeof(WCHAR));//初始化动作
			MultiByteToWideChar(CP_ACP,0, reginfo.Info2, -1, wInfo2, num); 


			RegWnd.processname.Format(L"%s",wProcName);
			RegWnd.processpath.Format(L"%s",wProcFullPath);
			RegWnd.regname.Format(L"%s",wInfo1);
			RegWnd.regvalue.Format(L"%s",wInfo2);
			CString strtemp;


			strtemp.Format (L"进程%s(%s)",wProcName,wProcFullPath);
			reglistbox.AddString(strtemp);
  
			reglistbox.AddString(L"  修改注册表:");

			strtemp.Format(L"    路径%s",wInfo2);
			reglistbox.AddString(strtemp);

			strtemp.Format(L"    键值%s",wInfo1);
			reglistbox.AddString(strtemp);


			regtemp = reginfo;




			//C:\Program Files\Thunder Network\Thunder\Program\Thunder5.exe
			int isfind=0;//初始未找到
			for (regiter=RegRullList.begin();regiter!=RegRullList.end();regiter++   )  
			{
				if (wcsstr((*regiter).lpFilePath[2],wProcFullPath)!=NULL)
				{
					isfind=1;
					break;
				}
			}

			if (isfind==1) //如果有记录
			{ 
				//0为永通过,1为一直不允许
				if (wcscmp((*regiter).lpFilePath[0],L"1")==0)//如果为1
				{	
					REG_CALLBACK_INFO reginfo1={0}, regtemp1 = {0};
					DWORD	dwRet;
					BOOL	bRet;
					HANDLE hDriver = ::CreateFile(SYMBOL_LINK,GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
					if (hDriver == INVALID_HANDLE_VALUE)
					{
						::MessageBox(NULL,L"打开驱动设备对象失败！\n",L"错误",MB_OK);
					}

					HANDLE hProcessEvent = ::OpenEventW(SYNCHRONIZE, FALSE, APP_EVENT_NAME);
					if (hProcessEvent == NULL)
					{
						::MessageBox(NULL,L"打开事件失败！\n",L"错误",MB_OK);
					}
					//IOCTL_NTPROCDRV_SET_APPEVENT_OK
					bRet = ::DeviceIoControl(hDriver,IOCTL_NTPROCDRV_SET_APPEVENT_OK,NULL,0,&reginfo1,sizeof(reginfo1)+64,&dwRet,NULL);
					if (bRet)
					{
						reglistbox.AddString(L"    用户确认,相同则一直放行!");
					} 
					::CloseHandle(hDriver);
				}
				else if (wcscmp((*regiter).lpFilePath[0],L"0")==0)//如果为0
				{	
					REG_CALLBACK_INFO reginfo1={0}, regtemp1 = {0};
					DWORD	dwRet;
					BOOL	bRet;
					HANDLE hDriver = ::CreateFile(SYMBOL_LINK,GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
					if (hDriver == INVALID_HANDLE_VALUE)
					{
						::MessageBox(NULL,L"打开驱动设备对象失败！\n",L"错误",MB_OK);
					}
				
					HANDLE hProcessEvent = ::OpenEventW(SYNCHRONIZE, FALSE, APP_EVENT_NAME);
					if (hProcessEvent == NULL)
					{
						::MessageBox(NULL,L"打开事件失败！\n",L"错误",MB_OK);
					}
					//IOCTL_NTPROCDRV_SET_APPEVENT_NO
					bRet = ::DeviceIoControl(hDriver,IOCTL_NTPROCDRV_SET_APPEVENT_NO,NULL,0,&reginfo1,sizeof(reginfo1)+64,&dwRet,NULL);
					if (bRet)
					{
						reglistbox.AddString(L"    用户确认,一直拒绝!");
					} 
					::CloseHandle(hDriver);
				}
				else 
					
				{		
/*
					CHttpClient   conn;   
					CString   value=L"http://127.0.0.1:801/f.php?procpath=";
					value=value+wProcName;
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
					RegWnd.regdescribe.Format(L"%s",wword);
//					::MessageBox(NULL,RegWnd.regdescribe,L"显",0);
*/
					RegWnd.regdescribe.Format(L"无网络服器测试版，不支持联网获取描述信息。请慎重选择放行！");
					RegWnd.DoModal();
				}

} 
else //如果没有记录,则弹窗
{ 
	/*
					CHttpClient   conn;   
					CString   value=L"http://127.0.0.1:801/f.php?procpath=";
					value=value+wProcName;
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
					RegWnd.regdescribe.Format(L"%s",wword);
					::MessageBox(NULL,RegWnd.regdescribe,L"显",0);
		*/
				RegWnd.regdescribe.Format(L"无网络服器测试版，不支持联网获取描述信息。请慎重选择放行！");
				RegWnd.DoModal();
} 					
isfind=0;//恢复全局变量





		} 
		else
		{
			break;
		}
		
	}

	return 0;
}




IMPLEMENT_DYNAMIC(CMyPage3, CPropertyPage)

CMyPage3::CMyPage3()
	: CPropertyPage(CMyPage3::IDD)
{

}

CMyPage3::~CMyPage3()
{
}

void CMyPage3::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, reglistbox);
	DDX_Control(pDX, IDC_LIST3, RegRull);
}


BEGIN_MESSAGE_MAP(CMyPage3, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON1, &CMyPage3::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMyPage3::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CMyPage3::OnBnClickedButton3)
	ON_LBN_SELCHANGE(IDC_LIST1, &CMyPage3::OnLbnSelchangeList1)
	ON_BN_CLICKED(IDC_BUTTON5, &CMyPage3::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CMyPage3::OnBnClickedButton6)
END_MESSAGE_MAP()


// CMyPage3 消息处理程序

UINT ThreadProcRegMon1(LPVOID lpParam)
{

	int ret=RegMon();
	return 0;
}




void CMyPage3::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	//新线程
//	HANDLE hProc=AfxBeginThread(ThreadProcRegMon1,&reglp);


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
		return;
	}
	DWORD	dwRet;
	BOOL	bRet;
	bRet = ::DeviceIoControl(
							hDriver,
							IOCTL_NTPROCDRV_TURNON_REGMON,
							NULL,
							0,
							&reginfo,
							sizeof(reginfo)+64,
							&dwRet,
							NULL);
	if (bRet)
	{
		reglistbox.AddString(L"注册表监控成功开启!");
	} 
	else
	{
		::MessageBox(NULL,L"监控开启失败",L"注册表",0);
		return;
	}
	return;
}



void CMyPage3::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	//停止监控
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
		return;
	}
	DWORD	dwRet;
	BOOL	bRet;
	bRet = ::DeviceIoControl(
							hDriver,
							IOCTL_NTPROCDRV_TURNOFF_REGMON,
							NULL,
							0,
							&reginfo,
							sizeof(reginfo)+64,
							&dwRet,
							NULL);
	if (bRet)
	{
		reglistbox.AddString(L"注册表监控停止成功!");
	} 
	else
	{
		::MessageBox(NULL,L"监控停止失败",L"注册表",0);
		return;
	}
	return;
}

void CMyPage3::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	reglistbox.ResetContent();
}

void CMyPage3::OnLbnSelchangeList1()
{
	// TODO: 在此添加控件通知处理程序代码
}





void CMyPage3::SetListBoxHScroll(void)
{

CDC* dc = GetDC();
SIZE s;
int i;
TCHAR str[100];
HDC hdc = dc->m_hDC; // 获得相关设备句柄
for(i = 0; i < reglistbox.GetCount(); i++)
{
     reglistbox.GetText(i, str);
     GetTextExtentPoint32(hdc, str, lstrlen(str), &s); // 获取字符串的像素大小
     // 如果新的字符串宽度大于先前的水平滚动条宽度，则重新设置滚动条宽度
     // IDC_LISTBOX为m_reglistboxBox的资源ID
     if (s.cx > (LONG)SendDlgItemMessage(IDC_LIST1, LB_GETHORIZONTALEXTENT, 0, 0))
      SendDlgItemMessage(IDC_LIST1, LB_SETHORIZONTALEXTENT, (WPARAM)s.cx, 0);
}
ReleaseDC(dc);
}

BOOL CMyPage3::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  在此添加额外的初始化
	RegRull.InsertColumn ( 0, L"ID",LVCFMT_LEFT, 30 ) ;
	RegRull.InsertColumn ( 1, L"是否通过",LVCFMT_CENTER, 80 ) ;
	RegRull.InsertColumn ( 2, L"进程名",LVCFMT_CENTER, 100 ) ;
	RegRull.InsertColumn ( 3, L"进程路径",LVCFMT_LEFT, 500 ) ;
	RegRull.InsertColumn ( 4, L"注册表路径",LVCFMT_LEFT, 500 ) ;
	RegRull.SetExtendedStyle(     LVS_EX_FULLROWSELECT   |   LVS_EX_GRIDLINES   |LVS_EX_HEADERDRAGDROP   |   LVS_EX_ONECLICKACTIVATE     );   

	this->db.Open ( L"rulls.mdb", FALSE, FALSE, NULL ) ;
	this->OnBnClickedFlush () ;

//开启
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
		return 0;
	}
	DWORD	dwRet;
	BOOL	bRet;
	bRet = ::DeviceIoControl(
							hDriver,
							IOCTL_NTPROCDRV_TURNON_REGMON,
							NULL,
							0,
							&reginfo,
							sizeof(reginfo)+64,
							&dwRet,
							NULL);
	if (bRet)
	{
		reglistbox.AddString(L"注册表监控成功开启!");
	} 
	else
	{
		::MessageBox(NULL,L"监控开启失败",L"注册表",0);
		return 0;
	}

	HANDLE hProc=AfxBeginThread(ThreadProcRegMon1,&reglp);


	return TRUE;

	// 异常: OCX 属性页应返回 FALSE
}



void CMyPage3::OnBnClickedFlush(void)
{
	// 检测数据库是否已连接
	if ( this->db.IsOpen () == FALSE )
	{
		this->MessageBox ( L"未连接数据库！" ) ;
		return ;
	}

	// 删除列表控件所有记录
	this->RegRull.DeleteAllItems () ;
	RegRullList.clear();
	// 定义记录集对象，并打开记录集
	CDaoRecordset	Record ( &this->db ) ;
	Record.Open ( dbOpenDynaset, L"SELECT * FROM regrull" ) ;

	UINT		nIndex = 0 ;
	CString		TempStr ;
	COleVariant	OleVariant ;

	// 移动到第一条记录
	Record.MoveFirst () ;
	while( !Record.IsEOF() )//全部列出来
	{
		// 在列表控件添加记录
		TempStr.Format ( L"%d", nIndex+1 ) ;
		this->RegRull.InsertItem ( nIndex, TempStr ) ;

		for ( int i = 1; i <= 4; i++ )
		{//一行里有4列
			Record.GetFieldValue ( i, OleVariant ) ;
			this->RegRull.SetItemText ( nIndex, i, OleVariant.bstrVal ) ;
			wcscpy(ri.lpFilePath[i-1],OleVariant.bstrVal);
		}
		//加入队列,成功
		RegRullList.push_back(ri);

		// 移到下一条记录
		Record.MoveNext () ;
		nIndex++ ;
	}
	// 关闭记录集
	Record.Close () ;

}





void CMyPage3::OnBnClickedButton5()
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
	Record.Open ( dbOpenDynaset, L"SELECT * FROM regrull" ) ;

	// 定义对话框对象，并设置为“添加”模式
	AddRegRull AddBookDlg ;


	if ( AddBookDlg.DoModal() == IDOK )
	{
		// 使用CDaoRecordset::AddNew函数表明要开始添加新记录
		Record.AddNew () ;
		// 设置新记录
		Record.SetFieldValue ( 1, AddBookDlg.pass.GetBuffer() ) ;
		Record.SetFieldValue ( 2, AddBookDlg.procname.GetBuffer() ) ;
		Record.SetFieldValue ( 3, AddBookDlg.procpath.GetBuffer() ) ;
		Record.SetFieldValue ( 4, AddBookDlg.regpath.GetBuffer() ) ;
		// 只有在执行CDaoRecordset::Update函数后才会写入到数据库
		Record.Update () ;
		// 关闭记录集
		Record.Close () ;
		// 刷新界面列表控件数据显示
		this->OnBnClickedFlush () ;

	}


}

void CMyPage3::OnBnClickedButton6()
{
	// TODO: 在此添加控件通知处理程序代码
	if ( this->db.IsOpen () == FALSE )
	{
		this->MessageBox ( L"未连接数据库！" ) ;
		return ;
	}

	// 检测是否已选择书目
	int nCurSel = this->RegRull.GetNextItem ( -1,LVNI_SELECTED) ;
	if ( nCurSel == -1 )
	{
		this->MessageBox ( L"没有选择书目！" ) ;
		return ;
	}

	// 定义记录集对象，并移动指针到指定位置
	CDaoRecordset Record ( &this->db ) ;
	Record.Open ( dbOpenDynaset, L"SELECT * FROM regrull" ) ;
	Record.Move ( nCurSel ) ;
	// 删除当前记录
	Record.Delete () ;
	// 关闭记录集
	Record.Close () ;
	// 刷新界面列表控件数据显示
	this->OnBnClickedFlush () ;
}
