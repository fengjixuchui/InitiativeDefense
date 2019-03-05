#include "filemon.h"
// Download by http://www.51xue8xue8.com

//保存文件变动
PCWSTR  PreCreateFileName=L"love";
PCWSTR  PreOpenFileName=L"love";
PCWSTR  PreWriteFileName=L"love";



//-------------------Filemon控制-------------------
#define IOCTL_GET_FILEINFO CTL_CODE(FILE_DEVICE_UNKNOWN,0x0840, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SET_APPEVENT_OK CTL_CODE(FILE_DEVICE_UNKNOWN,0x0841, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SET_APPEVENT_NO CTL_CODE(FILE_DEVICE_UNKNOWN,0x0842, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_TURNON_FILEMON CTL_CODE(FILE_DEVICE_UNKNOWN,0x0843, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_TURNOFF_FILEMON CTL_CODE(FILE_DEVICE_UNKNOWN,0x0844, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
int TurnOnFileMon=1;//默认开启
int FilePass=1;//默认通过
//-------------------Filemon控制结束-------------------





#define NT_DEVICE_NAME      L"\\Device\\FileMon"
#define DOS_DEVICE_NAME     L"\\DosDevices\\FileMon"

HANDLE             FileAppHandle;        // 事件对象句柄
PKEVENT            FileAppEvent;          // 用户和内核通信的事件对象指针


PDEVICE_OBJECT  g_pDriverObject;//保存全局数据
NTSTATUS DispatchCreate(IN PDEVICE_OBJECT DeviceObject,IN PIRP	 Irp);
NTSTATUS DispatchClose(IN PDEVICE_OBJECT DeviceObject,IN PIRP	 Irp);
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,IN PUNICODE_STRING theRegistryPath)
{
	//驱动入口函数
	NTSTATUS        ntStatus = STATUS_SUCCESS;
	UNICODE_STRING  ntDeviceName,linkString;
	UNICODE_STRING  DeviceLinkString;
	UNICODE_STRING	FileEventString;
	UNICODE_STRING	FileAppEventString;
	PDEVICE_OBJECT  deviceObject = NULL;
    PDEVICE_EXTENSION	deviceExtension={0};
	RtlInitUnicodeString( &ntDeviceName, NT_DEVICE_NAME );
	GetProcessNameOffset();
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

    RtlInitUnicodeString(&linkString,L"\\DosDevices\\FileMon");

    ntStatus = IoCreateSymbolicLink (&linkString, &ntDeviceName);

	if(!NT_SUCCESS(ntStatus))
	{
		return ntStatus;
	}

     deviceExtension=(PDEVICE_EXTENSION)deviceObject->DeviceExtension;   
	g_pDriverObject=deviceObject;	 
	 
	//初始化事件
	RtlInitUnicodeString(&FileEventString,EVENT_NAME);  //驱动层
	RtlInitUnicodeString(&FileAppEventString,APP_EVENT_NAME);  //应用层
	deviceExtension->FileEvent=IoCreateNotificationEvent(&FileEventString,&deviceExtension->FileHandle);	
	FileAppEvent=IoCreateNotificationEvent(&FileAppEventString,&FileAppHandle);
	//设置为非受信状态
	KeClearEvent(deviceExtension->FileEvent);	
	KeClearEvent(FileAppEvent);	
	
	
	
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
	HOOK_SYSCALL(ZwWriteFile,HookZwWriteFile,RealZwWriteFile);
	
	KdPrint(("文件监控驱动程序加载完毕.\n"));

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
	HOOK_SYSCALL(ZwWriteFile,RealZwWriteFile,Oldfun);

	
	
	if(m_MDL){
		MmUnmapLockedPages(m_Mapped,m_MDL);
		IoFreeMdl(m_MDL);
	}

	KdPrint(("文件监控驱动卸载完毕.\n"));

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
	

	case IOCTL_GET_FILEINFO:
	   {
			KdPrint((" 应用层获取文件信息"));
		   if(outBufLength>=sizeof(CALLBACK_INFO))
		   {
				strcpy(pCallbackInfo->ProcName , deviceExtension->ProcName);
				strcpy(pCallbackInfo->ProcFullPath  , deviceExtension->ProcFullPath);
				strcpy(pCallbackInfo->FilePre  , deviceExtension->FilePre);
				strcpy(pCallbackInfo->FilePath  , deviceExtension->FilePath);
		
				status=STATUS_SUCCESS;
			}
			else 
			{
				KdPrint(("空间不够用"));			
			}
			KeClearEvent(deviceExtension->FileEvent);		
	   }
	   	break;
	case IOCTL_SET_APPEVENT_OK: 
		{
			KdPrint((" 放行!应用通知驱动的事件"));
			FilePass=1;//全居通过,事件设置必须在后面
			KeSetEvent(FileAppEvent,IO_NO_INCREMENT,FALSE);
			status=STATUS_SUCCESS;
		}
		break;
	case IOCTL_SET_APPEVENT_NO: 
		{
			KdPrint((" 禁止!应用通知驱动的事件"));
			FilePass=0;//全居禁止			
			KeSetEvent(FileAppEvent,IO_NO_INCREMENT,FALSE);
			status=STATUS_SUCCESS;
		}
		break;
		case IOCTL_TURNON_FILEMON:
		{
			KdPrint((" 开启文件监控"));
			TurnOnFileMon=1;	//开启
			//KeSetEvent(FileAppEvent,IO_NO_INCREMENT,FALSE);
			status=STATUS_SUCCESS;
		}
		break;
	case IOCTL_TURNOFF_FILEMON:
		{
			KdPrint((" 关闭文件监控"));
			TurnOnFileMon=0;	//关闭,通知事件便于关闭
			//KeSetEvent(FileAppEvent,IO_NO_INCREMENT,FALSE);
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



	
//HookNtWriteFile
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
)
{
	if (TurnOnFileMon==1)//如果功能开启的话
	{
		PDEVICE_EXTENSION	deviceExtension;//设备扩展
		char aProcessName[PROCNAMELEN];
		UNICODE_STRING volumeDosName;
		WCHAR	DelFullName[1024];
		PCWSTR WriteFileName;
		PFILE_OBJECT fileobj;
		FILE_INFORMATION_CLASS FileInformationClass = FileNameInformation ;
		if ( FileHandle != NULL)
		{
				UNICODE_STRING uProcessPath={0};//进程路径
				ANSI_STRING aProcessPath={0};//进程路径
				
			   NTSTATUS nts = STATUS_UNSUCCESSFUL;
			   IO_STATUS_BLOCK iosb ={ 0,0 };
			   ANSI_STRING    ansiUndeleteFileName ;//
			   UNICODE_STRING usFileName ={ 0,0,0 }; //
			   
			   
			   PCWSTR ProcessName = GetCurrentProcessFileName();
				PCWSTR WDosName = NULL;
			   
			   PFILE_NAME_INFORMATION pFileInfo = NULL;
			   
			   RtlInitUnicodeString(&uProcessPath,ProcessName);
			   RtlUnicodeStringToAnsiString(&aProcessPath,&uProcessPath,TRUE);
			   RtlInitEmptyUnicodeString( &usFileName,'\0',0 );
			   pFileInfo = (PFILE_NAME_INFORMATION) ExAllocatePool( PagedPool, sizeof(FILE_NAME_INFORMATION) + MAXPATHLEN);
			   if ( NULL == pFileInfo )
			   {
					return STATUS_INSUFFICIENT_RESOURCES;
			   }//if ( NULL == pfni )
			   RtlZeroMemory( pFileInfo ,sizeof(FILE_NAME_INFORMATION) + MAXPATHLEN );
			   //由文件句柄获得文件名信息
			   nts = ZwQueryInformationFile(   
					FileHandle,
					&iosb, 
					pFileInfo,
					sizeof(FILE_NAME_INFORMATION) + MAXPATHLEN, 
					FileNameInformation
				);
			   if ( !NT_SUCCESS(nts) )
			   {
					ExFreePool(pFileInfo);
					pFileInfo = NULL;
					return nts; 
			   } 
			   else
			   {
					ANSI_STRING ansiDesFileName; 
					PWSTR pwstr=NULL;
					ULONG len =0;

	////////////////////////////////		转换路径的		////////////////////////////////
					ANSI_STRING apath;
					UNICODE_STRING upath;
					
					ANSI_STRING apathpre;
					UNICODE_STRING upathpre;
	////////////////////////////////		转换路径的结束		////////////////////////////////

					pwstr= wcsrchr( pFileInfo->FileName, L'\\' );
					RtlInitUnicodeString(&usFileName, pwstr+1); // uniFileName 不用释放
					RtlUnicodeStringToAnsiString(&ansiDesFileName, &usFileName, TRUE); // TRUE, 必须释放
					GetProcessName(aProcessName);
					ObReferenceObjectByHandle(FileHandle,GENERIC_READ,*IoFileObjectType,KernelMode,(PVOID *)&fileobj,NULL);	
					WriteFileName=fileobj->FileName.Buffer;
					
					//	此处转换	路径	
					RtlInitUnicodeString(&upath,WriteFileName);
					RtlUnicodeStringToAnsiString(&apath,&upath,TRUE);	

					
					if (wcscmp(WriteFileName,PreWriteFileName)!=0){
						IoVolumeDeviceToDosName( ((PFILE_OBJECT)fileobj)->DeviceObject, &volumeDosName );
						if  (&volumeDosName!=NULL && 
						(volumeDosName.Length<=6)//应该是这里会蓝屏
						)
						{
							//	此处转换	盘符	
							RtlInitUnicodeString(&upathpre,volumeDosName.Buffer);
							RtlUnicodeStringToAnsiString(&apathpre,&upathpre,TRUE);	

							KdPrint(("%s(%s)修改文件:%s%s \r\n",aProcessName,aProcessPath.Buffer,apathpre.Buffer,apath.Buffer));
							
							//填写设备扩展
							deviceExtension=(PDEVICE_EXTENSION)g_pDriverObject->DeviceExtension;

							strcpy(deviceExtension->ProcName,aProcessName);
							strcpy(deviceExtension->ProcFullPath,aProcessPath.Buffer);
							strcpy(deviceExtension->FilePre,apathpre.Buffer);
							strcpy(deviceExtension->FilePath,apath.Buffer);
			
							//通知应用层可以来取了
							KeSetEvent(deviceExtension->FileEvent,0,FALSE);
							KeClearEvent(deviceExtension->FileEvent);	

							PreWriteFileName=WriteFileName;
						}
					}
			   }
		}
	}
	
	return RealZwWriteFile(
	   FileHandle, 
	   Event OPTIONAL, 
	   ApcRoutine OPTIONAL, 
	   ApcContext OPTIONAL, 
	   IoStatusBlock, 
	   Buffer, 
	   Length, 
	   ByteOffset OPTIONAL, 
	   Key OPTIONAL 
	   );
}	
