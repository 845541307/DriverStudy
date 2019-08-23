#include <wdm.h>

#define		DeviceName		L"\\Device\\MyWDMDevice"
#define		SymLinkName		L"\\??\\HelloWDM"

typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT DeviceObject;
	PDEVICE_OBJECT NextStackDevice;
	UNICODE_STRING ustrDeviceName;	// 设备名
	UNICODE_STRING ustrSymLinkName;	// 符号链接名
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


NTSTATUS HelloWDMAddDevice(IN PDRIVER_OBJECT DriverObject,
	IN PDEVICE_OBJECT PhysicalDeviceObject);
NTSTATUS HelloWDMPnp(IN PDEVICE_OBJECT fdo,
	IN PIRP Irp);
NTSTATUS HelloWDMDispatchRoutine(IN PDEVICE_OBJECT fdo,
	IN PIRP Irp);
void HelloWDMUnload(IN PDRIVER_OBJECT DriverObject);

NTSTATUS DefaultPnpHandler(PDEVICE_EXTENSION pdx, PIRP Irp);

NTSTATUS HandleRemoveDevice(PDEVICE_EXTENSION pdx, PIRP Irp);

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject,
	IN PUNICODE_STRING pRegistryPath) 
{
	KdPrint(("why::Entry Driver\n"));
	NTSTATUS status = STATUS_SUCCESS;
	pDriverObject->DriverUnload = HelloWDMUnload;
	pDriverObject->DriverExtension->AddDevice = HelloWDMAddDevice;//创建设备对象并由PNP管理器调用
	for (int i = 0; i < ARRAYSIZE(pDriverObject->MajorFunction); i++)
	{
		pDriverObject->MajorFunction[i] = HelloWDMDispatchRoutine;
	}
	pDriverObject->MajorFunction[IRP_MJ_PNP] = HelloWDMPnp;
	KdPrint(("why::Leave Driver\n"));
	return status;
}

NTSTATUS HelloWDMAddDevice(IN PDRIVER_OBJECT pDriverObject,
	IN PDEVICE_OBJECT PhysicalDeviceObject)
{
	PAGED_CODE();//该例程中断级低于APC_LEVEL下运行
	KdPrint(("why::Enter HelloWDMAddDevice\n"));

	NTSTATUS status;
	PDEVICE_OBJECT fdo;
	UNICODE_STRING devName = RTL_CONSTANT_STRING(DeviceName);
	status = IoCreateDevice(
		pDriverObject,
		sizeof(DEVICE_EXTENSION),
		&devName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&fdo);
	if (!NT_SUCCESS(status))
		return status;
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;
	pdx->DeviceObject = fdo;
	pdx->NextStackDevice = IoAttachDeviceToDeviceStack(fdo, PhysicalDeviceObject);
	UNICODE_STRING symLinkName = RTL_CONSTANT_STRING(SymLinkName);

	pdx->ustrDeviceName = devName;
	pdx->ustrSymLinkName = symLinkName;
	status = IoCreateSymbolicLink(&symLinkName, &devName);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("why::Create symbolLink fail\n"));
		IoDeleteDevice(fdo);
		return status;
	}

	fdo->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;
	fdo->Flags &= ~DO_DEVICE_INITIALIZING;//设备对象初始化完成

	KdPrint(("why::Leave HelloWDMAddDevice\n"));
	return STATUS_SUCCESS;
}

NTSTATUS DefaultPnpHandler(PDEVICE_EXTENSION pdx, PIRP Irp)
{
	PAGED_CODE();
	KdPrint(("why::Enter DefaultPnpHandler\n"));
	IoSkipCurrentIrpStackLocation(Irp);
	KdPrint(("why::Leave DefaultPnpHandler\n"));
	return IoCallDriver(pdx->NextStackDevice, Irp);
}

NTSTATUS HandleRemoveDevice(PDEVICE_EXTENSION pdx, PIRP Irp)
{
	PAGED_CODE();
	KdPrint(("why::Enter HandleRemoveDevice\n"));

	Irp->IoStatus.Status = STATUS_SUCCESS;
	NTSTATUS status = DefaultPnpHandler(pdx, Irp);
	IoDeleteSymbolicLink(&pdx->ustrSymLinkName);

	//调用IoDetachDevice()把fdo从设备栈中脱开：
	if (pdx->NextStackDevice)
		IoDetachDevice(pdx->NextStackDevice);

	//删除fdo：
	IoDeleteDevice(pdx->DeviceObject);
	KdPrint(("why::Leave HandleRemoveDevice\n"));
	return status;
}

NTSTATUS HelloWDMPnp(IN PDEVICE_OBJECT fdo,
	IN PIRP Irp)
{
	PAGED_CODE();
	KdPrint(("why::Enter HelloWDMPnp\n"));
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	static NTSTATUS(*fcntab[])(PDEVICE_EXTENSION pdx, PIRP Irp) =
	{
		DefaultPnpHandler,		// IRP_MN_START_DEVICE
		DefaultPnpHandler,		// IRP_MN_QUERY_REMOVE_DEVICE
		HandleRemoveDevice,		// IRP_MN_REMOVE_DEVICE
		DefaultPnpHandler,		// IRP_MN_CANCEL_REMOVE_DEVICE
		DefaultPnpHandler,		// IRP_MN_STOP_DEVICE
		DefaultPnpHandler,		// IRP_MN_QUERY_STOP_DEVICE
		DefaultPnpHandler,		// IRP_MN_CANCEL_STOP_DEVICE
		DefaultPnpHandler,		// IRP_MN_QUERY_DEVICE_RELATIONS
		DefaultPnpHandler,		// IRP_MN_QUERY_INTERFACE
		DefaultPnpHandler,		// IRP_MN_QUERY_CAPABILITIES
		DefaultPnpHandler,		// IRP_MN_QUERY_RESOURCES
		DefaultPnpHandler,		// IRP_MN_QUERY_RESOURCE_REQUIREMENTS
		DefaultPnpHandler,		// IRP_MN_QUERY_DEVICE_TEXT
		DefaultPnpHandler,		// IRP_MN_FILTER_RESOURCE_REQUIREMENTS
		DefaultPnpHandler,		// 
		DefaultPnpHandler,		// IRP_MN_READ_CONFIG
		DefaultPnpHandler,		// IRP_MN_WRITE_CONFIG
		DefaultPnpHandler,		// IRP_MN_EJECT
		DefaultPnpHandler,		// IRP_MN_SET_LOCK
		DefaultPnpHandler,		// IRP_MN_QUERY_ID
		DefaultPnpHandler,		// IRP_MN_QUERY_PNP_DEVICE_STATE
		DefaultPnpHandler,		// IRP_MN_QUERY_BUS_INFORMATION
		DefaultPnpHandler,		// IRP_MN_DEVICE_USAGE_NOTIFICATION
		DefaultPnpHandler,		// IRP_MN_SURPRISE_REMOVAL
	};

	ULONG fcn = stack->MinorFunction; //保存具体的IRP请求,如IRP_MN_START_DEVICE
	if (fcn >= ARRAYSIZE(fcntab))
	{						// unknown function
		status = DefaultPnpHandler(pdx, Irp); // some function we don't know about
		return status;
	}						// unknown function

#if DBG
	static char* fcnname[] =
	{
		"IRP_MN_START_DEVICE",
		"IRP_MN_QUERY_REMOVE_DEVICE",
		"IRP_MN_REMOVE_DEVICE",
		"IRP_MN_CANCEL_REMOVE_DEVICE",
		"IRP_MN_STOP_DEVICE",
		"IRP_MN_QUERY_STOP_DEVICE",
		"IRP_MN_CANCEL_STOP_DEVICE",
		"IRP_MN_QUERY_DEVICE_RELATIONS",
		"IRP_MN_QUERY_INTERFACE",
		"IRP_MN_QUERY_CAPABILITIES",
		"IRP_MN_QUERY_RESOURCES",
		"IRP_MN_QUERY_RESOURCE_REQUIREMENTS",
		"IRP_MN_QUERY_DEVICE_TEXT",
		"IRP_MN_FILTER_RESOURCE_REQUIREMENTS",
		"",
		"IRP_MN_READ_CONFIG",
		"IRP_MN_WRITE_CONFIG",
		"IRP_MN_EJECT",
		"IRP_MN_SET_LOCK",
		"IRP_MN_QUERY_ID",
		"IRP_MN_QUERY_PNP_DEVICE_STATE",
		"IRP_MN_QUERY_BUS_INFORMATION",
		"IRP_MN_DEVICE_USAGE_NOTIFICATION",
		"IRP_MN_SURPRISE_REMOVAL",
	};

	KdPrint(("why::PNP Request (%s)\n", fcnname[fcn]));
#endif // DBG

	status = (*fcntab[fcn])(pdx, Irp);
	KdPrint(("why::Leave HelloWDMPnp\n"));
	return status;
}

NTSTATUS HelloWDMDispatchRoutine(IN PDEVICE_OBJECT fdo,
	IN PIRP Irp)
{
	PAGED_CODE();
	KdPrint(("why::Enter HelloWDMDispatchRoutine\n"));
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;	// no bytes xfered
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	KdPrint(("why::Leave HelloWDMDispatchRoutine\n"));
	return STATUS_SUCCESS;
}

void HelloWDMUnload(IN PDRIVER_OBJECT DriverObject)
{
	PAGED_CODE();
	KdPrint(("why::Enter HelloWDMUnload\n"));
	KdPrint(("why::Leave HelloWDMUnload\n"));
}