// MyPage0.cpp : 实现文件
// Download by http://www.51xue8xue8.com

#include "stdafx.h"
#include "MyPage0.h"
#include "HttpClient.h"

// CMyPage0 对话框

IMPLEMENT_DYNAMIC(CMyPage0, CPropertyPage)

CMyPage0::CMyPage0()
	: CPropertyPage(CMyPage0::IDD)
{

}

CMyPage0::~CMyPage0()
{
}

void CMyPage0::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMyPage0, CPropertyPage)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON1, &CMyPage0::OnBnClickedButton1)
END_MESSAGE_MAP()


// CMyPage0 消息处理程序


void CMyPage0::OnPaint()
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
		///显示它   
		//这里是显示大小603*380
//		dc.BitBlt(0,0,rect.Width(),rect.Height(),&MemDC,0,0,SRCCOPY);   
		dc.BitBlt(0,0,603,380,&MemDC,0,0,SRCCOPY);   
		MemDC.SelectObject(OldBitmap);
	
		CDialog::OnPaint();
	
}

void CMyPage0::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
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
}
