#include "ntddk.h" 
#include <stdio.h> 
#include <string.h>
#include <windef.h>
#include <wchar.h>


#define EVENT_NAME L"\\BaseNamedObjects\\MemEvent"  //驱动层通知事件
#define APP_EVENT_NAME L"\\BaseNamedObjects\\MenAppEvent" //应用层是否事件

// Length of process name (rounded up to next DWORD)
#define PROCNAMELEN     20
// Maximum length of NT process name
#define NT_PROCNAMELEN 16
#define MAXPATHLEN 256
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
    HANDLE             MemHandle;        // 事件对象句柄
    PKEVENT            MemEvent;          // 用户和内核通信的事件对象指针
	CHAR	*			ProcName[256];					//进程名
    CHAR	*			ProcFullPath[256];         //进程路径
	CHAR   *			Info1[256];					//信息1
	CHAR	*			Info2[256];					//信息2
}DEVICE_EXTENSION, *PDEVICE_EXTENSION;



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
ZWCREATESECTION  OldZwCreateSetion=NULL;