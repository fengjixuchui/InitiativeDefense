#ifndef _PROCMON_H
#define _PROCMON_H 1

#include <ntddk.h>



#define EVENT_NAME L"\\BaseNamedObjects\\ProcEvent"
#define EVENT_NAME_APP L"\\BaseNamedObjects\\ProcEventApp"
#define MAXPATHLEN 256
#define DWORD unsigned long
#define WORD unsigned short
#define BOOL unsigned long
#define BYTE unsigned char

typedef struct _CallbackInfo
{
   HANDLE  hParentId;
   HANDLE  hProcessId;
   CHAR    ProcFullPath[256];          //进程路径
   CHAR ParentProcFullPath[256];
   BOOLEAN bCreate;
 } CALLBACK_INFO, *PCALLBACK_INFO;


typedef struct _DEVICE_EXTENSION
{
    HANDLE             hProcessHandle;        // 事件对象句柄
    PKEVENT            ProcessEvent;          // 用户和内核通信的事件对象指针
    HANDLE             hParentId;             // 在回调函数中保存进程信息
    HANDLE             hProcessId;
    PCHAR              ProcFullPath;         //进程路径
    CHAR 			ParentProcFullPath[256];   //父进程路径
    BOOLEAN            bCreate;
}DEVICE_EXTENSION, *PDEVICE_EXTENSION;



NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationProcess(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL) ; 

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenProcess(
	OUT PHANDLE pProcessHandle, 
	IN ACCESS_MASK          AccessMask, 
	IN POBJECT_ATTRIBUTES  ObjectAttributes,  
	IN PCLIENT_ID pClientId);


	
NTSTATUS PsLookupProcessByProcessId(
	IN ULONG ulProcId,
	OUT PEPROCESS * pEProcess);

//获得当前进程完整路径
PCWSTR GetCurrentProcessFileName() 
{ 
	DWORD dwAddress = (DWORD)PsGetCurrentProcess(); 
	DWORD dwAddress1;
	if(dwAddress == 0 || dwAddress == 0xFFFFFFFF)  return NULL; 
	dwAddress += 0x1B0; 
	if((dwAddress = *(DWORD*)dwAddress) == 0) return 0; 
	dwAddress += 0x10; 
	if((dwAddress = *(DWORD*)dwAddress) == 0) return 0; 
	dwAddress1 = dwAddress;//2000
	dwAddress += 0x3C; 
	if((dwAddress = *(DWORD*)dwAddress) == 0) return 0; 
	if (dwAddress < dwAddress1) dwAddress = dwAddress+dwAddress1;
//	KdPrint(("Current Process Full Path Name: %ws\n", (PCWSTR)dwAddress)); 
	return (PCWSTR)dwAddress; 
} 	

//获得进程完整路径
BOOLEAN GetProcPath(IN HANDLE PID,OUT PANSI_STRING pImageName)
{
  NTSTATUS status;
  HANDLE hProcess=NULL;
  CLIENT_ID clientid;
  OBJECT_ATTRIBUTES ObjectAttributes;
  ULONG returnedLength; 
  ULONG bufferLength;    
  PVOID buffer;
  PUNICODE_STRING imageName;
  
  //初始化字符串
  try
    {
		if(!MmIsAddressValid(pImageName))//判读内存是否有效
		{
			return FALSE;
		}
		pImageName->Length = 0 ;
		pImageName->MaximumLength = 0 ;
		pImageName->Buffer = NULL ;
    }
 except (EXCEPTION_EXECUTE_HANDLER)
    {
        return FALSE ;
    }


  InitializeObjectAttributes(&ObjectAttributes,0,OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,0,0);
  clientid.UniqueProcess=PID;
  clientid.UniqueThread=0;
  //通过PID获得进程句柄
  ZwOpenProcess(&hProcess,PROCESS_ALL_ACCESS,&ObjectAttributes,&clientid);
 
  //获得大小
  status = ZwQueryInformationProcess(hProcess, 
                                     ProcessImageFileName,
                                     NULL, // buffer
                                     0, // buffer size
                                     &returnedLength);

  if (status!=STATUS_INFO_LENGTH_MISMATCH)
  {
        return FALSE;
   }

  bufferLength = returnedLength - sizeof(UNICODE_STRING);  
  
  buffer=ExAllocatePoolWithTag(PagedPool,returnedLength,'ipgD');
  
  if (buffer==NULL)
  {
     return FALSE;        
   }


//获得文件镜像名
  status=ZwQueryInformationProcess(hProcess,ProcessImageFileName,buffer,returnedLength,&returnedLength);
  
  if (NT_SUCCESS(status)) 
  {
	imageName=(PUNICODE_STRING)buffer;	  
  }
        
  if (RtlUnicodeStringToAnsiString (pImageName,imageName,TRUE)!=STATUS_SUCCESS )
   {
		KdPrint(("Current ProcessImageFileName: Unknow\r\n")) ;
		return FALSE ;
   }
 else
   {
//		KdPrint (("Current ProcessImageFileName: \"%s\"\r\n", pImageName->Buffer)) ;
		return TRUE ;
   }

  ExFreePool(buffer);	
}






#endif 