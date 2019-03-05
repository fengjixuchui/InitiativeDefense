// Download by http://www.51xue8xue8.com
#include <ntddk.h>
#include <stdio.h>
#include <string.h>
#include "procmon.h"




//原型申明
VOID ProcessCreateMon(IN HANDLE hProcessId,IN HANDLE PId,IN BOOLEAN bCreate);
NTSTATUS DriverUnload(IN PDRIVER_OBJECT DriverObject);
NTSTATUS DispatchCreate(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp);
NTSTATUS DispatchClose(IN PDEVICE_OBJECT  DeviceObject,IN PIRP Irp);
NTSTATUS DispatchIoctl(IN PDEVICE_OBJECT  DeviceObject,IN PIRP Irp);
BOOLEAN GetProcPath(IN HANDLE PID,OUT PANSI_STRING pImageName);


PDEVICE_OBJECT  g_pDriverObject;//保存全局数据
HANDLE hAppEvent;
PKEVENT ProcessEventApp;
INT TurnOnProcMon=1;//全局开启进程监控
INT ProcMonPass=1;//初始化开启
#define IOCTL_NTPROCDRV_GET_PROCINFO CTL_CODE(FILE_DEVICE_UNKNOWN,0x0800, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NTPROCDRV_SET_APPEVENT CTL_CODE(FILE_DEVICE_UNKNOWN,0x0801, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NTPROCDRV_TURNON_PROCMON CTL_CODE(FILE_DEVICE_UNKNOWN,0x0802, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NTPROCDRV_TURNOFF_PROCMON CTL_CODE(FILE_DEVICE_UNKNOWN,0x0803, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)



// 驱动入口
NTSTATUS  DriverEntry(IN PDRIVER_OBJECT DriverObject,IN PUNICODE_STRING RegistryPath ) 
{ 
    UNICODE_STRING  nameString, linkString,ProcessEventString,AppEvent;
	PDEVICE_OBJECT      deviceObject;
    PDEVICE_EXTENSION	deviceExtension;
	NTSTATUS            status;
	IO_STATUS_BLOCK     ioStatus;
	PIRP				pIrp;
    int                 i;
	status=STATUS_SUCCESS;
	
    DriverObject->MajorFunction[IRP_MJ_CREATE]=DispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]=DispatchClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]=DispatchIoctl;   
    DriverObject->DriverUnload=DriverUnload;  

    //建立设备
    RtlInitUnicodeString(&nameString,L"\\Device\\ProcMon");

    // 创建设备对象
	status=IoCreateDevice(DriverObject,
                             sizeof(DEVICE_EXTENSION), // 为设备扩展结构申请空间 
                             &nameString,
                             FILE_DEVICE_UNKNOWN,
                             0,
                             FALSE,
                             &deviceObject);
                                                 
    if (!NT_SUCCESS(status))
	{ 
		return status;
	}
    
    deviceExtension=(PDEVICE_EXTENSION)deviceObject->DeviceExtension;


    RtlInitUnicodeString(&linkString,L"\\DosDevices\\ProcMon");

    status = IoCreateSymbolicLink (&linkString, &nameString);

    if (!NT_SUCCESS(status))
    {
        //删除上面的设备对象
		IoDeleteDevice(deviceObject);
        return status;
    }
  
	KdPrint(("进程监控启动成功!\r\n\t设备:%wZ\r\n\t路径:%wZ",&nameString,RegistryPath));
    
	//设立事件局柄
	//DriverObject->Flags=DO_BUFFERED_IO;  
	
   //保存到设备对象的指针，下面在进程回调函数中还要使用
    g_pDriverObject=deviceObject;

    RtlInitUnicodeString(&ProcessEventString,EVENT_NAME);  
	
	deviceExtension->ProcessEvent=IoCreateNotificationEvent(&ProcessEventString,&deviceExtension->hProcessHandle);
    
	//设置为非受信状态
	KeClearEvent(deviceExtension->ProcessEvent);
    //设置回调例程
	
	
	//APP EVENT
	RtlInitUnicodeString(&AppEvent,EVENT_NAME_APP);	
	ProcessEventApp=IoCreateNotificationEvent(&AppEvent,&hAppEvent);
	KeClearEvent(ProcessEventApp);

	//	KeSetEvent(ProcessEventApp,IO_NO_INCREMENT,FALSE);

   status = PsSetCreateProcessNotifyRoutine(ProcessCreateMon,FALSE);
   
   return status; 
} 




//处理设备对象操作
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


// I/O控制派遣例程
NTSTATUS DispatchIoctl(IN PDEVICE_OBJECT DeviceObject,IN PIRP	Irp) 
{

	PDEVICE_EXTENSION  deviceExtension;
	ULONG IoControlCode,InBufLength,outBufLength; 
	PCALLBACK_INFO pCallbackInfo;
	//通信事件
	UNICODE_STRING uEventName;  
	PKEVENT pEvent;  
	HANDLE hEvent,hThread;  
	

	NTSTATUS status=STATUS_INVALID_DEVICE_REQUEST;

	// 取得此IRP（pIrp）的I/O堆栈指针
	PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation(Irp);
	// 取得设备扩展结构指针
	deviceExtension=(PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	// 取得I/O控制代码
	IoControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
	pCallbackInfo=(PCALLBACK_INFO)Irp->AssociatedIrp.SystemBuffer;
	InBufLength  =irpStack->Parameters.DeviceIoControl.InputBufferLength;
	outBufLength =irpStack->Parameters.DeviceIoControl.OutputBufferLength;
		KdPrint(("进入ioctl\r\n"));


	switch(IoControlCode)
	{
	case IOCTL_NTPROCDRV_GET_PROCINFO:
	   {
		   KdPrint(("获取进程信息"));
		   if(outBufLength>=sizeof(CALLBACK_INFO))
		   {
				pCallbackInfo->hParentId   = deviceExtension->hParentId;
				pCallbackInfo->hProcessId  = deviceExtension->hProcessId;
				
				strcpy(pCallbackInfo->ProcFullPath,deviceExtension->ProcFullPath);
				strcpy(pCallbackInfo->ParentProcFullPath,deviceExtension->ParentProcFullPath);
				
				pCallbackInfo->bCreate     = deviceExtension->bCreate;
				
				status=STATUS_SUCCESS;
			}
	  
	   }
	   break;
	   
	case IOCTL_NTPROCDRV_SET_APPEVENT: 
		{
			KdPrint(("设置应用层事件"));
			KeSetEvent(ProcessEventApp,IO_NO_INCREMENT,FALSE);
			status=STATUS_SUCCESS;
		}
	 break;
	 
	case IOCTL_NTPROCDRV_TURNON_PROCMON: 
		{
			KdPrint(("开启进程监控"));
			TurnOnProcMon=1;
			status=STATUS_SUCCESS;
		}
	 break;
	
	case IOCTL_NTPROCDRV_TURNOFF_PROCMON: 
		{
			KdPrint(("关闭进程监控"));
			TurnOnProcMon=0;
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




NTSTATUS DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING  linkString;
	NTSTATUS status=STATUS_SUCCESS;

	// 删除回调例程   
	status = PsSetCreateProcessNotifyRoutine(ProcessCreateMon,TRUE);
	//必须删除符号链接才卸载完成
	RtlInitUnicodeString(&linkString,L"\\DosDevices\\ProcMon");
	IoDeleteDevice (DriverObject->DeviceObject);
	IoDeleteSymbolicLink(&linkString);
	KdPrint(("进程监控卸载成功!\n"));
	return status;
}



//回调函数
VOID ProcessCreateMon(IN HANDLE hParentId,IN HANDLE PId,IN BOOLEAN bCreate)
{
	
    PEPROCESS    EProcess;
    ULONG        ulCurrentProcessId;
    NTSTATUS     status;
	PDEVICE_EXTENSION	deviceExtension;
	HANDLE hProcess=NULL;
    ANSI_STRING pImageName;
    PCHAR outbuf;
	ULONG outlen;
	PCWSTR ParentProcessName;	//父进程完整路径
	UNICODE_STRING uParentProcessName;
	ANSI_STRING aParentProcessName;
//	PLARGE_INTEGER WaitTime=NULL;
//	WaitTime->QuadPart=-5000000;
	DbgPrint("进入ProcessCreateMon\n");	


	ParentProcessName=GetCurrentProcessFileName();
	RtlInitUnicodeString(&uParentProcessName,ParentProcessName);
	RtlUnicodeStringToAnsiString(&aParentProcessName,&uParentProcessName,TRUE);
	
	status = PsLookupProcessByProcessId((ULONG)PId, &EProcess);
	if (!NT_SUCCESS( status ))
	{
		DbgPrint("PsLookupProcessByProcessId出错\n");
		return;
	}
	
	//不做结束与新建的判断
	RtlInitAnsiString(&pImageName,"test"); 
	GetProcPath(PId,&pImageName);//取镜像文件路径

	outbuf=(PCHAR)pImageName.Buffer;
	outlen=pImageName.Length+1;

	// 得到设备扩展结构的指针
	deviceExtension=(PDEVICE_EXTENSION)g_pDriverObject->DeviceExtension;

	// 安排当前值到设备扩展结构
	deviceExtension->hParentId=hParentId;
	deviceExtension->hProcessId=PId;
	deviceExtension->ProcFullPath=(PCHAR)outbuf;
	strcpy(deviceExtension->ParentProcFullPath,aParentProcessName.Buffer);
	deviceExtension->bCreate=bCreate;    
	
	//触发事件，通知应用程序
	KeSetEvent(deviceExtension->ProcessEvent,0,FALSE);		
	KeClearEvent(deviceExtension->ProcessEvent);
	if ( bCreate){
	DbgPrint( "新进程创建\r\n\t文件路径:%s\r\n\t父进程ID:%d\r\n\t父进程路径:%ws\r\n\t进程ID:%d\r\n\t内存地址:%x\r\n", 
							  deviceExtension->ProcFullPath,
							  deviceExtension->hParentId,
							  ParentProcessName,
							  deviceExtension->hProcessId,
							  EProcess);
	}
	else 
	{
	DbgPrint( "进程结束\r\n\t文件路径:%s\r\n\t父进程ID:%d\r\n\t父进程路径:%ws\r\n\t进程ID:%d\r\n\t内存地址:%x\r\n", 
							  deviceExtension->ProcFullPath,
							  deviceExtension->hParentId,
							  ParentProcessName,
							  deviceExtension->hProcessId,
							  EProcess);
	}
}






