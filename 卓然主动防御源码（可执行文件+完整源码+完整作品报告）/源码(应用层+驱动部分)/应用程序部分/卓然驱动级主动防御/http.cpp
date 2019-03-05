// http.cpp : 定义应用程序的类行为。
// Download by http://www.51xue8xue8.com

#include "stdafx.h"
#include "http.h"
#include "MainFrm.h"

#include "httpDoc.h"
#include "httpView.h"
#include "HttpClient.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ChttpApp

BEGIN_MESSAGE_MAP(ChttpApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &ChttpApp::OnAppAbout)
	// 基于文件的标准文档命令
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
	// 标准打印设置命令
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()


// ChttpApp 构造

ChttpApp::ChttpApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 ChttpApp 对象

ChttpApp theApp;


// ChttpApp 初始化

BOOL ChttpApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// 初始化 OLE 库
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));
	LoadStdProfileSettings(4);  // 加载标准 INI 文件选项(包括 MRU)
	// 注册应用程序的文档模板。文档模板
	// 将用作文档、框架窗口和视图之间的连接
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(ChttpDoc),
		RUNTIME_CLASS(CMainFrame),       // 主 SDI 框架窗口
		RUNTIME_CLASS(ChttpView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);



	// 分析标准外壳命令、DDE、打开文件操作的命令行
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);


	// 调度在命令行中指定的命令。如果
	// 用 /RegServer、/Register、/Unregserver 或 /Unregister 启动应用程序，则返回 FALSE。
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// 唯一的一个窗口已初始化，因此显示它并对其进行更新
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// 仅当具有后缀时才调用 DragAcceptFiles
	//  在 SDI 应用程序中，这应在 ProcessShellCommand 之后发生
	return TRUE;
}



// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CAboutDlg::OnBnClickedButton1)
END_MESSAGE_MAP()

// 用于运行对话框的应用程序命令
void ChttpApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// ChttpApp 消息处理程序


void CAboutDlg::OnBnClickedButton1()
{

/*
	// TODO: 在此添加控件通知处理程序代码
	CHttpClient   conn;   
  CString   value=L"http://127.0.0.1:801/f.php";   
  CStringA f;
  CString b;
  f=conn.doGet(value);
//  b.Format("%s",f);
  ::MessageBoxA(NULL,f,"操",0);
//					this->MessageBox(b);
*/


	//让控件和对应的变量之间进行数据交换，现在将控件数据传给对应变量


//将网页内容的源代码读至编辑框

CHttpClient   conn;   
CString   value=L"http://127.0.0.1:801/f.php";   
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
this->MessageBox(b);

return ;





//myHttpFile->Close ;
//mySession.Close ;

}
