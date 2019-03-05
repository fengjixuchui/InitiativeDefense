#ifndef IOCTLS_H
#define IOCTLS_H 1

// 定义IOCTL
#define FILE_DEVICE_PROCMON	0x8000
#define FILE_DEVICE_PROCMON_APP	0x8001


#define IOCTL_NTPROCDRV_SET_APPEVENT CTL_CODE(FILE_DEVICE_UNKNOWN,0x0801, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NTPROCDRV_GET_PROCINFO CTL_CODE(FILE_DEVICE_UNKNOWN,0x0800, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)


// 定义事件对象名称
#define EVENT_NAME L"ProcEvent"
#define EVENT_NAME_APP L"ProcEventApp"


typedef struct _ProcCallbackInfo
{
   HANDLE  hParentId;
   HANDLE  hProcessId;
   CHAR    ProcFullPath[256];//进程路径
   CHAR ParentProcFullPath[256];
   BOOLEAN bCreate;
 } CALLBACK_INFO, *PCALLBACK_INFO;




typedef DWORD(WINAPI *pfsuspend)(HANDLE hThread);
typedef DWORD(WINAPI *pfresume)(HANDLE hThread);
static pfsuspend SuspendProcess;
static pfresume ResumeProcess;
static HMODULE hdll;




void static SuspendPro(DWORD pid)
{
	//::MessageBox(NULL,L"in susp",L"fuck",MB_OK);  
	hdll=LoadLibrary(L"NTDLL.DLL");
        if(hdll==NULL)
        {
			return;
        }
        SuspendProcess=(pfsuspend)GetProcAddress(hdll,"ZwSuspendProcess");
        HANDLE hpro;
        if((hpro=OpenProcess(PROCESS_SUSPEND_RESUME,NULL,pid))==NULL)
        {

        }

        SuspendProcess(hpro);
        FreeLibrary(hdll);
}

void static ResumePro(DWORD pid)
{

        hdll=LoadLibrary(L"NTDLL.DLL");
        if(hdll==NULL)
        {

        }
        ResumeProcess=(pfresume)GetProcAddress(hdll,"ZwResumeProcess");
        HANDLE hpro;
        if((hpro=OpenProcess(PROCESS_SUSPEND_RESUME,NULL,pid))==NULL)
        {

        }
//		::MessageBox(NULL,L"in resu",L"fuck",MB_OK);
        ResumeProcess(hpro);
        FreeLibrary(hdll);
}

void static KillPro(DWORD pid)
{
        HANDLE hpro;
//	::MessageBox(NULL,L"in kill",L"fuck",MB_OK);
        if((hpro=OpenProcess(PROCESS_TERMINATE,NULL,pid))==NULL)
        {

        }

        if(!TerminateProcess(hpro,0))
        {

        }

        else
        {

        }
}



#endif