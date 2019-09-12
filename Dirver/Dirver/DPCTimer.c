#include <ntddk.h>

#define CTL_START_TIMER		CTL_CODE(FILE_DEVICE_UNKNOWN,\
							 0x8000,\
							 METHOD_BUFFERED,\
							 FILE_ANY_ACCESS)

#define CTL_STOP_TIMER		CTL_CODE(FILE_DEVICE_UNKNOWN,\
							 0x8002,\
							 METHOD_BUFFERED,\
							 FILE_ANY_ACCESS)

#define		PAGEDCODE				code_seg("PAGE")
#define		LOCKEDCODE				code_seg()
#define		INITCODE				code_seg("INIT")
#define		DEVICE_NAME				TEXT("\\Device\\MyDPCTimerDevice")
#define		DEVICE_SYMLINKNAME		TEXT("\\??\\DPCTimerDevice")
#define		ONE_S					- 1 * 1000 * 10000
#define		ONE_MILLI				- 1 * 10000
#define		TIME_OUT				3

typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT	pDevice;
	UNICODE_STRING	strDeviceName;
	UNICODE_STRING	strSymLinkName;
	KTIMER			kTimer;
	KDPC			kDpc;
	LARGE_INTEGER	TimerInter; //单位为100纳秒
}DEVICE_EXTENSION, *pDEVICE_EXTENSION;

NTSTATUS DriverUnLoad(IN PDRIVER_OBJECT pDriverObject)
{
	NTSTATUS  status = STATUS_SUCCESS;
	PDEVICE_OBJECT	pNextObj;
	pNextObj = pDriverObject->DeviceObject;
	while (pNextObj)
	{
		pDEVICE_EXTENSION pDeviceExten = pNextObj->DeviceExtension;
		if (&pDeviceExten->strSymLinkName) {
			status = IoDeleteSymbolicLink(&pDeviceExten->strSymLinkName);
			if (!NT_SUCCESS(status)) {
				DbgPrint("why::delete symlink fail\n");
			}
		}
		pNextObj = pNextObj->NextDevice;
		if (pDeviceExten->pDevice) {
			IoDeleteDevice(pDeviceExten->pDevice);
		}
	}
	DbgPrint("why::driver unload success\n");
	return status;
}

NTSTATUS CommonDispatch(
	IN PDEVICE_OBJECT pDeviceObj,
	IN PIRP pIrp)
{
	NTSTATUS  status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}

VOID
DpcTimerRoutine(
	_In_ struct _KDPC *Dpc,
	_In_opt_ PVOID DeferredContext,
	_In_opt_ PVOID SystemArgument1,
	_In_opt_ PVOID SystemArgument2
)
{
	KdPrint(("why::DpcTimerRoutine entry\n"));
	PDEVICE_OBJECT pDevice = (PDEVICE_OBJECT)DeferredContext;
	pDEVICE_EXTENSION pDeviceEx = (pDEVICE_EXTENSION)pDevice->DeviceExtension;

	BOOLEAN bRet = KeSetTimer(&pDeviceEx->kTimer, pDeviceEx->TimerInter, &pDeviceEx->kDpc);
	if (bRet)
	{
		KdPrint(("why::DpcTimerRoutine KeSetTimer success\n"));
	}

	//检验是运行在任意线程上下文
	PEPROCESS pro = IoGetCurrentProcess();
	PCSTR	proName = (PCSTR)((LONGLONG)pro + 0x450);
	KdPrint(("why::IoTimerRouine call pro name is %s\n", proName));
	KdPrint(("why::DpcTimerRoutine leave\n"));
}

NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject)
{
	NTSTATUS		status = STATUS_SUCCESS;
	UNICODE_STRING	strDeviceName = RTL_CONSTANT_STRING(DEVICE_NAME);
	UNICODE_STRING	strSymLinkName = RTL_CONSTANT_STRING(DEVICE_SYMLINKNAME);
	PDEVICE_OBJECT	pDeviceObject;
	pDEVICE_EXTENSION deviceExten;
	status = IoCreateDevice(pDriverObject, sizeof(DEVICE_EXTENSION),
		&strDeviceName, FILE_DEVICE_UNKNOWN,
		0, TRUE,
		&pDeviceObject);
	if (!NT_SUCCESS(status)) {
		DbgPrint("why::Create Device failed\n");
		return status;
	}

	status = IoCreateSymbolicLink(&strSymLinkName, &strDeviceName);
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(pDeviceObject);
		DbgPrint("why::Create SymLink failed\n");
		return status;
	}

	pDeviceObject->Flags |= DO_BUFFERED_IO;

	deviceExten = (pDEVICE_EXTENSION)pDeviceObject->DeviceExtension;
	deviceExten->pDevice = pDeviceObject;
	deviceExten->strDeviceName = strDeviceName;
	deviceExten->strSymLinkName = strSymLinkName;
	KeInitializeTimer(&deviceExten->kTimer);
	KeInitializeDpc(&deviceExten->kDpc, DpcTimerRoutine, (PVOID)pDeviceObject);
	return status;
}

NTSTATUS
IoControlDispatch(
	_In_ struct _DEVICE_OBJECT *DeviceObject,
	_Inout_ struct _IRP *Irp
)
{
	KdPrint(("why::IoControlDispatch entry\n"));
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(Irp);
	pDEVICE_EXTENSION pDevice = (pDEVICE_EXTENSION)DeviceObject->DeviceExtension;	
	ULONG	info = 0;
	ULONG	ulTimerInter = 0;
	switch (pStack->Parameters.DeviceIoControl.IoControlCode)
	{
	case CTL_START_TIMER:
		ulTimerInter =  *(PULONG)Irp->AssociatedIrp.SystemBuffer;
		KdPrint(("why::IoControlDispatch CTL_START_TIMER\n"));
		pDevice->TimerInter = RtlConvertLongToLargeInteger(ONE_S * ulTimerInter);
		KeSetTimer(&pDevice->kTimer, pDevice->TimerInter, &pDevice->kDpc);
		break;
	case CTL_STOP_TIMER:
		KdPrint(("why::IoControlDispatch CTL_STOP_TIMER\n"));
		KeCancelTimer(&pDevice->kTimer);
		break;
	default:
		status = STATUS_INVALID_VARIANT;
		break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = info;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	KdPrint(("why::IoControlDispatch leave\n"));
	return status;
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegeditPath)
{
	UNREFERENCED_PARAMETER(pRegeditPath);
	NTSTATUS status = STATUS_SUCCESS;
	pDriverObject->DriverUnload = DriverUnLoad;
	for (int i = 0; i < ARRAYSIZE(pDriverObject->MajorFunction); i++) {
		pDriverObject->MajorFunction[i] = CommonDispatch;
	}

	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControlDispatch;

	status = CreateDevice(pDriverObject);

	return status;
}