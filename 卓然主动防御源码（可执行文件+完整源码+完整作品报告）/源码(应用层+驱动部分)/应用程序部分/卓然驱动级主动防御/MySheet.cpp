// MySheet.cpp : 实现文件
// Download by http://www.51xue8xue8.com

#include "stdafx.h"
#include "main.h"
#include "MySheet.h"


// CMySheet

IMPLEMENT_DYNAMIC(CMySheet, CPropertySheet)

CMySheet::CMySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	this->AddPage ( &page0 ) ;
	this->AddPage ( &page1 ) ;
	this->AddPage ( &page2 ) ;
	this->AddPage ( &page3 ) ;

	this->AddPage ( &page5 ) ;
	this->AddPage ( &page4 ) ;
}

CMySheet::CMySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	this->AddPage ( &page0 ) ;
	this->AddPage ( &page1 ) ;
	this->AddPage ( &page2 ) ;
	this->AddPage ( &page3 ) ;

	this->AddPage ( &page5 ) ;
	this->AddPage ( &page4 ) ;

}

CMySheet::~CMySheet()
{
}


BEGIN_MESSAGE_MAP(CMySheet, CPropertySheet)
END_MESSAGE_MAP()





// CMySheet 消息处理程序
