 //   HttpClient.h:   interface   for   the   CHttpClient   class.   
  //   
  //////////////////////////////////////////////////////////////////////   
    
  #if   !defined(AFX_HTTPCLIENT_H__EA769DCB_AAB9_47CD_BD87_FBD6913592C5__INCLUDED_)   
  #define   AFX_HTTPCLIENT_H__EA769DCB_AAB9_47CD_BD87_FBD6913592C5__INCLUDED_   
    
  #if   _MSC_VER   >   1000   
  #pragma   once   
  #endif   //   _MSC_VER   >   1000   
  #include   "wininet.h"   
  #include   "afxinet.h"   
  class   CHttpClient       
  {   
  public:   
  void   addParam(CString   name,CString   value);   
  CString   doPost(CString   href);   
  CString   doGet(CString   href);   
  CHttpClient();   
  virtual   ~CHttpClient();   
    
  private:   
  CString   CONTENT;   
  int   CL;   
  CStringList   values;   
  CStringList   names;   
  };   
    
  #endif   //   !defined