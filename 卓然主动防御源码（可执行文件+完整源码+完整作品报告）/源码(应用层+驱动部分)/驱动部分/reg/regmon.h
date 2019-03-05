#include "ntddk.h" 
#include <stdio.h> 
#include <string.h>
#include <windef.h>
#include <wchar.h>
#define DWORD unsigned long
#define WORD unsigned short


#define BOOL unsigned long
#define BYTE unsigned char
#define MAXPATHLEN 256
#define SEC_IMAGE    0x01000000



#define IOCTL_PROTECT_CONTROL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Length of process name (rounded up to next DWORD)
#define PROCNAMELEN     20
// Maximum length of NT process name
#define NT_PROCNAMELEN 16
//函数地址
int    position;//ZwCreateProcess
int    pos; //
int    po;
int    ps;//NtOpenProcess
int    pts;//ZwTerminateProcess
int    inxNtSetValueKey;
int    inxNtDeleteValueKey;
int    inxNtDeleteKey;
int    inxNtCreateKey;
int    inxNtCreateFile;
int    inxNtOpenFile;
int    inxNtWriteFile;
int    inxNtDeleteFile;

//#define   DWORD unsingned   long 
//函数申明
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,IN PUNICODE_STRING theRegistryPath);
NTSTATUS DispatchDeviceControl(IN PDEVICE_OBJECT  DeviceObject,IN PIRP  Irp);
VOID OnUnload(IN PDRIVER_OBJECT DriverObject);



#pragma pack(1)	//SSDT表的结构
typedef struct ServiceDescriptorEntry {
	unsigned int *ServiceTableBase;
	unsigned int *ServiceCounterTableBase; //Used only in checked build
	unsigned int NumberOfServices;
	unsigned char *ParamTableBase;
} ServiceDescriptorTableEntry_t, *PServiceDescriptorTableEntry_t;
#pragma pack()

PMDL m_MDL;
PVOID *m_Mapped;
__declspec(dllimport) ServiceDescriptorTableEntry_t KeServiceDescriptorTable;	//变量名是不能变的,因为是从外部导入
#define SYSTEMSERVICE(_function)  KeServiceDescriptorTable.ServiceTableBase[ *(PULONG)((PUCHAR)_function+1)]
#define SYSCALL_INDEX(_Function) *(PULONG)((PUCHAR)_Function+1)
#define HOOK_SYSCALL(_Function, _Hook, _Orig )  \
       _Orig = (PVOID) InterlockedExchange( (PLONG) &m_Mapped[SYSCALL_INDEX(_Function)], (LONG) _Hook)

NTSYSAPI NTSTATUS NTAPI ZwOpenProcess(OUT PHANDLE ProcessHandle,IN ACCESS_MASK DesiredAccess,IN POBJECT_ATTRIBUTES ObjectAttributes,IN PCLIENT_ID ClientId OPTIONAL);
NTSYSAPI NTSTATUS NTAPI ZwTerminateProcess(IN HANDLE ProcessHandle OPTIONAL,IN NTSTATUS ExitStatus);
typedef NTSTATUS (*ZWOPENPROCESS)(OUT PHANDLE ProcessHandle,IN ACCESS_MASK DesiredAccess,IN POBJECT_ATTRIBUTES ObjectAttributes,IN PCLIENT_ID ClientId OPTIONAL);
typedef NTSTATUS (*ZWTERMINATEPROCESS)(IN HANDLE ProcessHandle OPTIONAL,IN NTSTATUS ExitStatus);
NTSTATUS NewZwOpenProcess(OUT PHANDLE ProcessHandle,IN ACCESS_MASK DesiredAccess,IN POBJECT_ATTRIBUTES ObjectAttributes,IN PCLIENT_ID ClientId OPTIONAL);
NTSTATUS NewZwTerminateProcess(IN HANDLE ProcessHandle OPTIONAL,IN NTSTATUS ExitStatus);

NTSTATUS PsLookupProcessByProcessId(IN ULONG ulProcId, OUT PEPROCESS *pEProcess);



typedef struct _CallbackInfo
{
	CHAR    ProcName[256];	//进程名
	CHAR    ProcFullPath[256];          //进程路径
	CHAR    Info1[256];					//信息1
	CHAR    Info2[256];					//信息2
 } CALLBACK_INFO, *PCALLBACK_INFO;


typedef struct _DEVICE_EXTENSION
{
    HANDLE             RegHandle;        // 事件对象句柄
    PKEVENT            RegEvent;          // 用户和内核通信的事件对象指针
	CHAR			ProcName[256];					//进程名
    CHAR			ProcFullPath[256];         //进程路径
	CHAR 			Info1[256];					//信息1
	CHAR			Info2[256];					//信息2
}DEVICE_EXTENSION, *PDEVICE_EXTENSION;



//删字符串用的
/*
ANSI_STRING DelString(PANSI_STRING InStr,IN PANSI_STRING InDel){
	if (&InStr != NULL) {
		PANSI_STRING strs;
		strs=strstr(InStr,InDel);
		if (strs!=NULL) return *(strs+strlen(InDel));
	}
	return *InStr;
}
*/










//新加
NTSYSAPI NTSTATUS NTAPI
  ZwCreateSection(
    OUT PHANDLE  SectionHandle,
    IN ACCESS_MASK  DesiredAccess,
    IN POBJECT_ATTRIBUTES  ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER  MaximumSize OPTIONAL,
    IN ULONG  SectionPageProtection,
    IN ULONG  AllocationAttributes,
    IN HANDLE  FileHandle OPTIONAL
    ); 	

NTSTATUS 
  HookZwCreateSection(
    OUT PHANDLE  SectionHandle,
    IN ACCESS_MASK  DesiredAccess,
    IN POBJECT_ATTRIBUTES  ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER  MaximumSize OPTIONAL,
    IN ULONG  SectionPageProtection,
    IN ULONG  AllocationAttributes,
    IN HANDLE  FileHandle OPTIONAL
    ); 	

 
typedef  (*ZWCREATESECTION)(
    OUT PHANDLE  SectionHandle,
    IN ACCESS_MASK  DesiredAccess,
    IN POBJECT_ATTRIBUTES  ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER  MaximumSize OPTIONAL,
    IN ULONG  SectionPageProtection,
    IN ULONG  AllocationAttributes,
    IN HANDLE  FileHandle OPTIONAL
    ); 		
	
ZWCREATESECTION RealZwCreateSection=NULL;	














//ZwCreateFile
NTSYSAPI NTSTATUS NTAPI 
  ZwCreateFile(
    OUT PHANDLE  FileHandle,
    IN ACCESS_MASK  DesiredAccess,
    IN POBJECT_ATTRIBUTES  ObjectAttributes,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN PLARGE_INTEGER  AllocationSize  OPTIONAL,
    IN ULONG  FileAttributes,
    IN ULONG  ShareAccess,
    IN ULONG  CreateDisposition,
    IN ULONG  CreateOptions,
    IN PVOID  EaBuffer  OPTIONAL,
    IN ULONG  EaLength
    );
	
typedef NTSTATUS (*ZWCREATEFILE)(
    OUT PHANDLE  FileHandle,
    IN ACCESS_MASK  DesiredAccess,
    IN POBJECT_ATTRIBUTES  ObjectAttributes,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN PLARGE_INTEGER  AllocationSize  OPTIONAL,
    IN ULONG  FileAttributes,
    IN ULONG  ShareAccess,
    IN ULONG  CreateDisposition,
    IN ULONG  CreateOptions,
    IN PVOID  EaBuffer  OPTIONAL,
    IN ULONG  EaLength
    );


	

	
	
NTSTATUS  
  HookZwCreateFile(
    OUT PHANDLE  FileHandle,
    IN ACCESS_MASK  DesiredAccess,
    IN POBJECT_ATTRIBUTES  ObjectAttributes,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN PLARGE_INTEGER  AllocationSize  OPTIONAL,
    IN ULONG  FileAttributes,
    IN ULONG  ShareAccess,
    IN ULONG  CreateDisposition,
    IN ULONG  CreateOptions,
    IN PVOID  EaBuffer  OPTIONAL,
    IN ULONG  EaLength
    );	

ZWCREATEFILE RealZwCreateFile=NULL;




//ZwOpenFile
NTSYSAPI NTSTATUS NTAPI
  ZwOpenFile(
    OUT PHANDLE  FileHandle,
    IN ACCESS_MASK  DesiredAccess,
    IN POBJECT_ATTRIBUTES  ObjectAttributes,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN ULONG  ShareAccess,
    IN ULONG  OpenOptions
    );

typedef NTSTATUS
  (*ZWOPENFILE)(
    OUT PHANDLE  FileHandle,
    IN ACCESS_MASK  DesiredAccess,
    IN POBJECT_ATTRIBUTES  ObjectAttributes,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN ULONG  ShareAccess,
    IN ULONG  OpenOptions
    );
	
NTSTATUS 
  HookZwOpenFile(
    OUT PHANDLE  FileHandle,
    IN ACCESS_MASK  DesiredAccess,
    IN POBJECT_ATTRIBUTES  ObjectAttributes,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN ULONG  ShareAccess,
    IN ULONG  OpenOptions
    );
	
ZWOPENFILE RealZwOpenFile;




//NTSTATUS ObQueryNameString(IN PVOID  Object,OUT POBJECT_NAME_INFORMATION  ObjectNameInfo,IN ULONG  Length,OUT PULONG  ReturnLength); 
/*
NTSTATUS 
  ZwSetValueKey(
    IN HANDLE  KeyHandle,
    IN PUNICODE_STRING  ValueName,
    IN ULONG  TitleIndex  OPTIONAL,
    IN ULONG  Type,
    IN PVOID  Data,
    IN ULONG  DataSize
    );
	*/
extern NTSTATUS 
ObQueryNameString(void *, 
void *, 
int size, 
int *); 
extern NTSYSAPI NTSTATUS NTAPI 
ZwSetValueKey( IN HANDLE KeyHandle,
IN PUNICODE_STRING ValueName,
IN ULONG TitleIndex OPTIONAL,
IN ULONG Type,
IN PVOID Data,
IN ULONG DataSize); 


typedef NTSTATUS (*REALZWSETVALUEKEY)(
IN HANDLE KeyHandle,
IN PUNICODE_STRING ValueName,
IN ULONG TitleIndex OPTIONAL,
IN ULONG Type,
IN PVOID Data,
IN ULONG DataSize
);



NTSTATUS HookZwSetValueKey(
IN HANDLE KeyHandle,
IN PUNICODE_STRING ValueName,
IN ULONG TitleIndex OPTIONAL,
IN ULONG Type,
IN PVOID Data,
IN ULONG DataSize
);


PVOID GetPointer(HANDLE);
NTSTATUS HookZwSetValueKey(
IN HANDLE,
IN PUNICODE_STRING,
IN ULONG TitleIndex,
IN ULONG,
IN PVOID,
IN ULONG);


//HOOK ZwWriteFile
NTSYSAPI NTSTATUS NTAPI
  ZwWriteFile(
    IN HANDLE  FileHandle,
    IN HANDLE  Event  OPTIONAL,
    IN PIO_APC_ROUTINE  ApcRoutine  OPTIONAL,
    IN PVOID  ApcContext  OPTIONAL,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN PVOID  Buffer,
    IN ULONG  Length,
    IN PLARGE_INTEGER  ByteOffset  OPTIONAL,
    IN PULONG  Key  OPTIONAL
    );

typedef NTSTATUS 
  (*ZWWRITEFILE)(
    IN HANDLE  FileHandle,
    IN HANDLE  Event  OPTIONAL,
    IN PIO_APC_ROUTINE  ApcRoutine  OPTIONAL,
    IN PVOID  ApcContext  OPTIONAL,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN PVOID  Buffer,
    IN ULONG  Length,
    IN PLARGE_INTEGER  ByteOffset  OPTIONAL,
    IN PULONG  Key  OPTIONAL
    );

NTSTATUS 
HookZwWriteFile(
	IN HANDLE FileHandle, 
	IN HANDLE Event OPTIONAL, 
	IN PIO_APC_ROUTINE ApcRoutine OPTIONAL, 
	IN PVOID ApcContext OPTIONAL, 
	OUT PIO_STATUS_BLOCK IoStatusBlock, 
	IN PVOID Buffer, 
	IN ULONG Length, 
	IN PLARGE_INTEGER ByteOffset OPTIONAL, 
	IN PULONG Key OPTIONAL 
);
	
ZWWRITEFILE RealZwWriteFile;



ULONG gProcessNameOffset;
void GetProcessNameOffset()
{
	PEPROCESS curproc;
	int i;
	curproc = PsGetCurrentProcess();
	for( i = 0; i < 3*PAGE_SIZE; i++ )
	{
	   if( !strncmp( "System", (PCHAR) curproc + i, strlen("System") ))
	   {
	    gProcessNameOffset = i; 
	   }
	}
}


//获取当前进程名称
BOOL GetProcessName( PCHAR theName )
{
PEPROCESS       curproc;
char            *nameptr;
ULONG           i;
KIRQL           oldirql;
if( gProcessNameOffset )
{
   curproc = PsGetCurrentProcess();
   nameptr   = (PCHAR) curproc + gProcessNameOffset;
   strncpy( theName, nameptr, NT_PROCNAMELEN );
   theName[NT_PROCNAMELEN] = 0; /* NULL at end */
   return TRUE;
}
return FALSE;
}

//注册表要用
PVOID GetPointer( HANDLE handle )
{
    PVOID pKey;
    if(!handle) 
		return NULL;
    // ObReferenceObjectByHandle函数来获得这个Handle对应的FileObject, 得到的指针转换成文件对象的指针
    if( ObReferenceObjectByHandle( handle, 0, NULL, KernelMode, &pKey, NULL ) != STATUS_SUCCESS ) 
    {
        pKey = NULL;
    } 
    return pKey;
}



ZWOPENPROCESS OldZwOpenProcess = NULL;
ZWTERMINATEPROCESS OldZwTerminateProcess = NULL;
REALZWSETVALUEKEY RealZwSetValueKey=NULL;
//新加
ZWCREATESECTION  OldZwCreateSetion=NULL;


//获得进程完整路径
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

/*
char *WriteFileNameFull(IN HANDLE filehandle){
OUT char filefullname_c[1024];
PFILE_OBJECT file=0;
PFILE_OBJECT relatedfile=0;
POBJECT_HANDLE_INFORMATION info=0; 
ANSI_STRING filefullname_a; 
UNICODE_STRING filefullname_u;
int relatedfilelength;
POBJECT_NAME_INFORMATION      fileNameInformation;
ULONG retSize;

//先得到盘符
RtlVolumeDeviceToDosName(file->DeviceObject,&filefullname_u);
RtlUnicodeStringToAnsiString(&filefullname_a,&filefullname_u,1);
strncpy(filefullname_c,filefullname_a.Buffer,filefullname_a.Length); 
filefullname_c[filefullname_a.Length]='\0';
relatedfilelength=filefullname_a.Length;

//相对路径
relatedfile=file->RelatedFileObject;
RtlUnicodeStringToAnsiString(&filefullname_a,&relatedfile->FileName,1);
strncat(filefullname_c,filefullname_a.Buffer,filefullname_a.Length); 
filefullname_c[relatedfilelength+filefullname_a.Length]='\0';
relatedfilelength+=filefullname_a.Length;

//文件名
RtlUnicodeStringToAnsiString(&filefullname_a,&file->FileName,1);
strcat(filefullname_c,"\\");
strncat(filefullname_c,filefullname_a.Buffer,filefullname_a.Length);
filefullname_c[relatedfilelength+filefullname_a.Length+1]='\0';
return filefullname_c;
}
*/

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, OnUnload)
#pragma alloc_text(PAGE, DispatchDeviceControl)
#pragma alloc_text(PAGE, NewZwOpenProcess)
#pragma alloc_text(PAGE, NewZwTerminateProcess)
