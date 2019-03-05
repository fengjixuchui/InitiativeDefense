#include "regmon.h"

// Download by http://www.51xue8xue8.com
//-------------------regmon控制-------------------
int TurnOnRegMon=1;//默认开启
int RegPass=1;//默认通过
#define NT_DEVICE_NAME      L"\\Device\\RegMon"
#define DOS_DEVICE_NAME     L"\\DosDevices\\RegMon"
#define EVENT_NAME L"\\BaseNamedObjects\\RegEvent"  //驱动层通知事件
#define APP_EVENT_NAME L"\\BaseNamedObjects\\RegAppEvent" //应用层是否事件
#define IOCTL_NTPROCDRV_GET_REGINFO CTL_CODE(FILE_DEVICE_UNKNOWN,0x0910, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NTPROCDRV_SET_APPEVENT_OK CTL_CODE(FILE_DEVICE_UNKNOWN,0x0911, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NTPROCDRV_SET_APPEVENT_NO CTL_CODE(FILE_DEVICE_UNKNOWN,0x0912, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NTPROCDRV_TURNON_REGMON CTL_CODE(FILE_DEVICE_UNKNOWN,0x0913, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NTPROCDRV_TURNOFF_REGMON CTL_CODE(FILE_DEVICE_UNKNOWN,0x0914, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
//-------------------regmon控制结束-------------------




//-------------------全局变量-------------------
HANDLE             RegAppHandle;        // 事件对象句柄
PKEVENT            RegAppEvent;          // 用户和内核通信的事件对象指针
PDEVICE_OBJECT  g_pDriverObject;//保存全局数据
//-------------------------------------------------



NTSTATUS DispatchCreate(IN PDEVICE_OBJECT DeviceObject,IN PIRP	 Irp);
NTSTATUS DispatchClose(IN PDEVICE_OBJECT DeviceObject,IN PIRP	 Irp);
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,IN PUNICODE_STRING theRegistryPath)
{
	//驱动入口函数
	NTSTATUS        ntStatus = STATUS_SUCCESS;
	UNICODE_STRING  ntDeviceName,linkString;
	UNICODE_STRING  DeviceLinkString;
	UNICODE_STRING	RegEventString;
	UNICODE_STRING	RegAppEventString;
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

    RtlInitUnicodeString(&linkString,L"\\DosDevices\\RegMon");

    ntStatus = IoCreateSymbolicLink (&linkString, &ntDeviceName);

	if(!NT_SUCCESS(ntStatus))
	{
		return ntStatus;
	}

     deviceExtension=(PDEVICE_EXTENSION)deviceObject->DeviceExtension;   
	g_pDriverObject=deviceObject;	 
	 
	//初始化事件
	RtlInitUnicodeString(&RegEventString,EVENT_NAME);  //驱动层
	RtlInitUnicodeString(&RegAppEventString,APP_EVENT_NAME);  //应用层
	deviceExtension->RegEvent=IoCreateNotificationEvent(&RegEventString,&deviceExtension->RegHandle);	
	RegAppEvent=IoCreateNotificationEvent(&RegAppEventString,&RegAppHandle);
	//设置为非受信状态
	KeClearEvent(deviceExtension->RegEvent);	
	KeClearEvent(RegAppEvent);	
	
	
	
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
	HOOK_SYSCALL(ZwSetValueKey,HookZwSetValueKey,RealZwSetValueKey);
	KdPrint(("[*]注册表驱动程序加载完毕.\n"));

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
	HOOK_SYSCALL(ZwSetValueKey,RealZwSetValueKey,Oldfun);

	if(m_MDL){
		MmUnmapLockedPages(m_Mapped,m_MDL);
		IoFreeMdl(m_MDL);
	}

	KdPrint(("[*]注册表驱动卸载完毕.\n"));

}


NTSTATUS DispatchCreate(IN PDEVICE_OBJECT DeviceObject,IN PIRP	 Irp)
{
  //DbgPrint("IRP_MJ_Create\n");

  Irp->IoStatus.Information=0;
  Irp->IoStatus.Status=STATUS_SUCCESS;
  IoCompleteRequest(Irp,IO_NO_INCREMENT);
  return STATUS_SUCCESS;
}


//处理设备对象操作
NTSTATUS DispatchClose(IN PDEVICE_OBJECT DeviceObject,IN PIRP	 Irp)
{
  //DbgPrint("IRP_MJ_Close\n");
  
  Irp->IoStatus.Information=0;
  Irp->IoStatus.Status=STATUS_SUCCESS;
  IoCompleteRequest(Irp,IO_NO_INCREMENT);
  return STATUS_SUCCESS;
}



//HookZwSetValueKey 
NTSTATUS HookZwSetValueKey(
IN HANDLE KeyHandle,
IN PUNICODE_STRING ValueName,
IN ULONG TitleIndex OPTIONAL,
IN ULONG Type,
IN PVOID Data,
IN ULONG DataSize)
{
    NTSTATUS rc;
    UNICODE_STRING *pUniName,uProcessPath; //定义得到修改注册表的UNI路径
    ULONG actualLen;
    ANSI_STRING keyname, akeyname, m_keyname,aProcessPath,aProcessName1;       //定义得到修改注册表的UNI路径
    PVOID pKey;
	char aProcessName[PROCNAMELEN];//进程名
	
	PCSTR APN;
	PCSTR regkeyname;
	PCSTR regkeyvalue;
	
	PDEVICE_EXTENSION	deviceExtension;
	PCWSTR ProcessName = GetCurrentProcessFileName();//进程路径
	RtlInitUnicodeString(&uProcessPath,ProcessName);
	RtlUnicodeStringToAnsiString( &aProcessPath, &uProcessPath, TRUE);
    RtlUnicodeStringToAnsiString( &akeyname, ValueName, TRUE);
	GetProcessName(aProcessName);
	RtlInitAnsiString(&aProcessName1,aProcessName);
	APN=aProcessName;
    // 得到文件对象的指针
	
	
	if (TurnOnRegMon==0)  //如果注册表监控功能不开启
	{
		RealZwSetValueKey(KeyHandle,ValueName,TitleIndex,Type,Data,DataSize);
	}
	
	if (TurnOnRegMon==1)//如果注册表监控功能开启
	{
		if( pKey = GetPointer( KeyHandle))
		{
			RegPass=1;//应用层默认通过
			pUniName = ExAllocatePool( NonPagedPool, 512*2+2*sizeof(ULONG));
			pUniName->MaximumLength = 512*2;
			memset(pUniName,0,pUniName->MaximumLength); 
		   
			if( NT_SUCCESS( ObQueryNameString( pKey, pUniName, 512*2, &actualLen) ) )
			{
				RtlUnicodeStringToAnsiString( &keyname, pUniName, TRUE);
				
				//这两个保留
				regkeyname=keyname.Buffer;
				regkeyvalue=akeyname.Buffer;
				
				regkeyname=_strupr(keyname.Buffer);
				regkeyvalue=_strupr(akeyname.Buffer);
				
				//此处判断,可独立出来函数
				if (strstr(regkeyname,"\\REGISTRY\\MACHINE\\SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\RUN"))
				{

					DbgPrint("进程:%s(%ws)\r\n修改注册表:%s\r\n%s\r\n",aProcessName,ProcessName,akeyname.Buffer,keyname.Buffer); 

					deviceExtension=(PDEVICE_EXTENSION)g_pDriverObject->DeviceExtension;
					//信息加入设备扩展
					strcpy(deviceExtension->ProcName,APN);//这里正常了
					strcpy(deviceExtension->ProcFullPath,(PCHAR )aProcessPath.Buffer);
					strcpy(deviceExtension->Info1,akeyname.Buffer);//名称
					strcpy(deviceExtension->Info2,keyname.Buffer);//路径
					
					KdPrint(("deviceExtension->ProcName:%s",deviceExtension->ProcName));
					KdPrint(("deviceExtension->ProcFullPath:%s",deviceExtension->ProcFullPath));
					KdPrint(("deviceExtension->Info1:%s",(PCSTR)deviceExtension->Info1));
					KdPrint(("deviceExtension->Info2:%s",(PCSTR)deviceExtension->Info2));

					//设置事件,应用层取信息
					KeClearEvent(RegAppEvent);	//避免不正常
					KeSetEvent(deviceExtension->RegEvent,0,FALSE);
					KeClearEvent(deviceExtension->RegEvent);
					
					KdPrint(("等待应用层确认"));	
					KeWaitForSingleObject(RegAppEvent,Executive,KernelMode,0,NULL);
					KeClearEvent(RegAppEvent);	
					KdPrint(("应用层确认结束:%d",RegPass));		
					
					if (RegPass==1)//应用层确认通过
					{
						//缺信息输出,缺释放资源
						KdPrint(("     放行"));	
						return RealZwSetValueKey(KeyHandle,ValueName,TitleIndex,Type,Data,DataSize);

					} 
					if (RegPass==0)//应用层确认通过
					{
						KdPrint(("    拒绝"));	
						return STATUS_ACCESS_DENIED;
					} 
				}
			}
		}
	}			

		

    // 释放内存
    return STATUS_ACCESS_DENIED;
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
	

	case IOCTL_NTPROCDRV_GET_REGINFO:
	   {
			KdPrint(("获得修改注册表的信息"));
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
	case IOCTL_NTPROCDRV_SET_APPEVENT_OK: 
		{
			KdPrint(("放行!应用通知驱动的事件"));
			RegPass=1;//全居通过,事件设置必须在后面
			KeSetEvent(RegAppEvent,IO_NO_INCREMENT,FALSE);
			status=STATUS_SUCCESS;
		}
		break;
	case IOCTL_NTPROCDRV_SET_APPEVENT_NO: 
		{
			KdPrint(("禁止!应用通知驱动的事件"));
			RegPass=0;//全居禁止			
			KeSetEvent(RegAppEvent,IO_NO_INCREMENT,FALSE);
			status=STATUS_SUCCESS;
		}
		break;
		case IOCTL_NTPROCDRV_TURNON_REGMON:
		{
			KdPrint(("开启注册表监控"));
			TurnOnRegMon=1;	//开启
			//KeSetEvent(RegAppEvent,IO_NO_INCREMENT,FALSE);
			status=STATUS_SUCCESS;
		}
		break;
	case IOCTL_NTPROCDRV_TURNOFF_REGMON:
		{
			KdPrint(("关闭注册表监控"));
			TurnOnRegMon=0;	//关闭,通知事件便于关闭
			//KeSetEvent(RegAppEvent,IO_NO_INCREMENT,FALSE);
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











	
