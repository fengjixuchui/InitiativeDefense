#include "memmon.h"
// Download by http://www.51xue8xue8.com
#define NT_DEVICE_NAME      L"\\Device\\MemMon"
#define DOS_DEVICE_NAME     L"\\DosDevices\\MemMon"


HANDLE             MemAppHandle;        // 事件对象句柄
PKEVENT            MemAppEvent;          // 用户和内核通信的事件对象指针
CHAR LastFile[256]="LoveSuae";


//-------------------MEMmon控制-------------------
#define IOCTL_GET_MEMINFO CTL_CODE(FILE_DEVICE_UNKNOWN,0x0830, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SET_APPEVENT_OK CTL_CODE(FILE_DEVICE_UNKNOWN,0x0831, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SET_APPEVENT_NO CTL_CODE(FILE_DEVICE_UNKNOWN,0x0832, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_TURNON_MEMMON CTL_CODE(FILE_DEVICE_UNKNOWN,0x0833, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_TURNOFF_MEMMON CTL_CODE(FILE_DEVICE_UNKNOWN,0x0834, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
int TurnOnMemMon=1;//默认开启
int MemPass=1;//默认通过
//-------------------MEMmon控制结束-------------------


PDEVICE_OBJECT  g_pDriverObject;//保存全局数据
NTSTATUS DispatchCreate(IN PDEVICE_OBJECT DeviceObject,IN PIRP	 Irp);
NTSTATUS DispatchClose(IN PDEVICE_OBJECT DeviceObject,IN PIRP	 Irp);
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,IN PUNICODE_STRING theRegistryPath)
{
	//驱动入口函数
	NTSTATUS        ntStatus = STATUS_SUCCESS;
	UNICODE_STRING  ntDeviceName,linkString;
	UNICODE_STRING  DeviceLinkString;
	UNICODE_STRING	MemEventString;
	UNICODE_STRING	MemAppEventString;
	PDEVICE_OBJECT  deviceObject = NULL;
    PDEVICE_EXTENSION	deviceExtension={0};
	RtlInitUnicodeString( &ntDeviceName, NT_DEVICE_NAME );
	ntStatus = IoCreateDevice(
		DriverObject,
		sizeof(DEVICE_EXTENSION), // 为设备扩展结构申请空间 
		&ntDeviceName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&deviceObject );

	if ( !NT_SUCCESS( ntStatus ) )
	{
		KdPrint(("[*]无法创建驱动设备"));
		return ntStatus;
	}

    RtlInitUnicodeString(&linkString,L"\\DosDevices\\MemMon");

    ntStatus = IoCreateSymbolicLink (&linkString, &ntDeviceName);

	if(!NT_SUCCESS(ntStatus))
	{
		return ntStatus;
	}

     deviceExtension=(PDEVICE_EXTENSION)deviceObject->DeviceExtension;   
	g_pDriverObject=deviceObject;	 
	 
	//初始化事件
	RtlInitUnicodeString(&MemEventString,EVENT_NAME);  //驱动层
	RtlInitUnicodeString(&MemAppEventString,APP_EVENT_NAME);  //应用层
	deviceExtension->MemEvent=IoCreateNotificationEvent(&MemEventString,&deviceExtension->MemHandle);	
	MemAppEvent=IoCreateNotificationEvent(&MemAppEventString,&MemAppHandle);
	//设置为非受信状态
	KeClearEvent(deviceExtension->MemEvent);	
	KeClearEvent(MemAppEvent);	
	
	
	
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;
	DriverObject->DriverUnload = OnUnload;

	m_MDL = MmCreateMdl(NULL,KeServiceDescriptorTable.ServiceTableBase,KeServiceDescriptorTable.NumberOfServices*4);
	if(!m_MDL)
		return STATUS_UNSUCCESSFUL;

	//非分页内存
	MmBuildMdlForNonPagedPool(m_MDL);

	m_MDL->MdlFlags = m_MDL->MdlFlags | MDL_MAPPED_TO_SYSTEM_VA;

	//锁定
	m_Mapped = MmMapLockedPages(m_MDL, KernelMode);



	
	//几处关键HOOK
	HOOK_SYSCALL(ZwCreateSection,HookZwCreateSection,RealZwCreateSection);
	
	KdPrint(("内存监视驱动程序加载完毕.\n"));

	return STATUS_SUCCESS;
}







VOID OnUnload(IN PDRIVER_OBJECT DriverObject)
{
	//卸载时会调用
	UNICODE_STRING DeviceLinkString;
	PDEVICE_OBJECT DeviceObjectTemp1=NULL;
	PDEVICE_OBJECT DeviceObjectTemp2=NULL;
	PVOID Oldfun = NULL;

	RtlInitUnicodeString(&DeviceLinkString,DOS_DEVICE_NAME);
	IoDeleteSymbolicLink(&DeviceLinkString);
	if(DriverObject)
	{
		DeviceObjectTemp1=DriverObject->DeviceObject;
		while(DeviceObjectTemp1)
		{
			DeviceObjectTemp2=DeviceObjectTemp1;
			DeviceObjectTemp1=DeviceObjectTemp1->NextDevice;
			IoDeleteDevice(DeviceObjectTemp2);
		}
	}  

	
	//还原SSDT
	HOOK_SYSCALL(ZwCreateSection,RealZwCreateSection,Oldfun);
	
	
	if(m_MDL){
		MmUnmapLockedPages(m_Mapped,m_MDL);
		IoFreeMdl(m_MDL);
	}

	KdPrint(("内存监视驱动卸载完毕.\n"));

}


NTSTATUS DispatchCreate(IN PDEVICE_OBJECT DeviceObject,IN PIRP	 Irp)
{
  DbgPrint("IRP_MJ_Create\n");

  Irp->IoStatus.Information=0;
  Irp->IoStatus.Status=STATUS_SUCCESS;
  IoCompleteRequest(Irp,IO_NO_INCREMENT);
  return STATUS_SUCCESS;
}


//处理设备对象操作
NTSTATUS DispatchClose(IN PDEVICE_OBJECT DeviceObject,IN PIRP	 Irp)
{
  DbgPrint("IRP_MJ_Close\n");
  
  Irp->IoStatus.Information=0;
  Irp->IoStatus.Status=STATUS_SUCCESS;
  IoCompleteRequest(Irp,IO_NO_INCREMENT);
  return STATUS_SUCCESS;
}




//IO CTL/IO通信之用
NTSTATUS DispatchDeviceControl(IN PDEVICE_OBJECT  DeviceObject,IN PIRP  Irp)
{

	PDEVICE_EXTENSION  deviceExtension;
	ULONG IoControlCode,InBufLength,outBufLength; 
	PCALLBACK_INFO pCallbackInfo;
	UNICODE_STRING uEventName;  
	PKEVENT pEvent;  
	HANDLE hEvent,hThread;  
	NTSTATUS status=STATUS_INVALID_DEVICE_REQUEST;
	PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation(Irp);
	deviceExtension=(PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
	IoControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
	pCallbackInfo=(PCALLBACK_INFO)Irp->AssociatedIrp.SystemBuffer;
	InBufLength  =irpStack->Parameters.DeviceIoControl.InputBufferLength;
	outBufLength =irpStack->Parameters.DeviceIoControl.OutputBufferLength;

	switch(IoControlCode)
	{
	

	case IOCTL_GET_MEMINFO:
	   {
			KdPrint((" 获取内存加载信息"));
		   if(outBufLength>=sizeof(CALLBACK_INFO))
		   {
				strcpy(pCallbackInfo->ProcName , deviceExtension->ProcName);
				strcpy(pCallbackInfo->ProcFullPath  , deviceExtension->ProcFullPath);
				strcpy(pCallbackInfo->Info1  , deviceExtension->Info1);
				strcpy(pCallbackInfo->Info2  , deviceExtension->Info2);
				
				status=STATUS_SUCCESS;
			}
	   }
	   	break;
	case IOCTL_SET_APPEVENT_OK: 
		{
			KdPrint((" 放行!应用通知驱动的事件"));
			MemPass=1;//全居通过,事件设置必须在后面
			KeSetEvent(MemAppEvent,IO_NO_INCREMENT,FALSE);
			status=STATUS_SUCCESS;
		}
		break;
	case IOCTL_SET_APPEVENT_NO: 
		{
			KdPrint((" 禁止!应用通知驱动的事件"));
			MemPass=0;//全居禁止			
			KeSetEvent(MemAppEvent,IO_NO_INCREMENT,FALSE);
			status=STATUS_SUCCESS;
		}
		break;
		case IOCTL_TURNON_MEMMON:
		{
			KdPrint((" 开启内存加载监控"));
			TurnOnMemMon=1;	//开启
			//KeSetEvent(MemAppEvent,IO_NO_INCREMENT,FALSE);
			status=STATUS_SUCCESS;
		}
		break;
	case IOCTL_TURNOFF_MEMMON:
		{
			KdPrint((" 关闭内存加载监控"));
			TurnOnMemMon=0;	//关闭,通知事件便于关闭
			//KeSetEvent(MemAppEvent,IO_NO_INCREMENT,FALSE);
			status=STATUS_SUCCESS;
		}
	 break;
	}

	if(status==STATUS_SUCCESS)
		Irp->IoStatus.Information=outBufLength; 
	else
		Irp->IoStatus.Information=0;

	Irp->IoStatus.Status=status;
	IoCompleteRequest(Irp,IO_NO_INCREMENT);
	return status;

}





NTSTATUS HookZwCreateSection(
OUT PHANDLE             SectionHandle,
IN ULONG                DesiredAccess,
IN POBJECT_ATTRIBUTES   ObjectAttributes OPTIONAL,
IN PLARGE_INTEGER       MaximumSize OPTIONAL,
IN ULONG                PageAttributess,
IN ULONG                SectionAttributes,
IN HANDLE               FileHandle OPTIONAL )
{ 
    HANDLE hHandle;
    PFILE_OBJECT pFileObject;
	PDEVICE_EXTENSION deviceExtension;
    OBJECT_HANDLE_INFORMATION HandleInformationObject;
    ANSI_STRING asFileName;
    char aPathName[MAXPATHLEN];
    char aProcessName[PROCNAMELEN];
   UNICODE_STRING   dosName, fullUniName; 
    char* findpointer=NULL;

    dosName.Buffer=(PWSTR)ExAllocatePool(PagedPool, 16);
    dosName.MaximumLength=8;
    
    fullUniName.Buffer = (PWSTR)ExAllocatePool(PagedPool, MAXPATHLEN*2);//1024*2   
    fullUniName.MaximumLength   =   MAXPATHLEN*2; 

	deviceExtension=(PDEVICE_EXTENSION)g_pDriverObject->DeviceExtension;
    hHandle=(HANDLE)FileHandle;
    ObReferenceObjectByHandle(hHandle,0,0,KernelMode,&pFileObject,&HandleInformationObject);
    if(pFileObject != NULL)
    {
		   RtlVolumeDeviceToDosName(pFileObject->DeviceObject, &dosName); 
		// DbgPrint("   %ws\n",dosName.Buffer);
		   //获得盘符
		   RtlCopyUnicodeString(&fullUniName, &dosName);                      
		  
		   //将盘符和名字放在一起，得到总路径
		   RtlAppendUnicodeStringToString(&fullUniName,&((PFILE_OBJECT)pFileObject)->FileName);
			RtlUnicodeStringToAnsiString(&asFileName,&(fullUniName),TRUE); 
		if  (_stricmp(LastFile,asFileName.Buffer)!=0)//防止重复
		{
				strcpy(LastFile,asFileName.Buffer);
			   //DbgPrint(" %s %ws\n",asFileName,dosName.Buffer);
				if((findpointer=strstr(asFileName.Buffer, "."))==NULL)
				 return ((ZWCREATESECTION)(RealZwCreateSection))(SectionHandle, DesiredAccess, ObjectAttributes, MaximumSize, PageAttributess, SectionAttributes, FileHandle);
			if (

				(_stricmp(findpointer, ".exe") == 0) || (_stricmp(findpointer, ".dll") == 0) || (_stricmp(findpointer, ".sys") == 0)

				)   // 蓝
			   {
					//写入设备扩展
			   		strcpy(deviceExtension->ProcFullPath,(PCHAR )asFileName.Buffer);				
					//这里开始通知应用层
					KeSetEvent(deviceExtension->MemEvent,0,FALSE);
					KeClearEvent(deviceExtension->MemEvent);

					DbgPrint("   %s\n",asFileName.Buffer);

				}
				
		}
	}
  
 
return ((ZWCREATESECTION)(RealZwCreateSection))(SectionHandle, 
                 DesiredAccess, 
                ObjectAttributes, 
                MaximumSize, 
                PageAttributess, 
                SectionAttributes, 
                FileHandle);

}

