//#include <ntddk.h>
//
//#define CTL_START_TIMER		CTL_CODE(FILE_DEVICE_UNKNOWN,\
//							 0x8000,\
//							 METHOD_BUFFERED,\
//							 FILE_ANY_ACCESS)
//
//#define CTL_STOP_TIMER		CTL_CODE(FILE_DEVICE_UNKNOWN,\
//							 0x8002,\
//							 METHOD_BUFFERED,\
//							 FILE_ANY_ACCESS)
//
//#define		PAGEDCODE				code_seg("PAGE")
//#define		LOCKEDCODE				code_seg()
//#define		INITCODE				code_seg("INIT")
//#define		DEVICE_NAME				TEXT("\\Device\\MyTimerDevice")
//#define		DEVICE_SYMLINKNAME		TEXT("\\??\\TimerDevice")
//#define		ONE_S					-10 * 1000 * 1000
//#define		TIME_OUT				3
//
//typedef struct _DEVICE_EXTENSION
//{
//	PDEVICE_OBJECT	pDevice;
//	UNICODE_STRING	strDeviceName;
//	UNICODE_STRING	strSymLinkName;
//	ULONG			ulTimeCount;
//}DEVICE_EXTENSION, *pDEVICE_EXTENSION;
//
//NTSTATUS DriverUnLoad(IN PDRIVER_OBJECT pDriverObject)
//{
//	NTSTATUS  status = STATUS_SUCCESS;
//	PDEVICE_OBJECT	pNextObj;
//	pNextObj = pDriverObject->DeviceObject;
//	while (pNextObj)
//	{
//		pDEVICE_EXTENSION pDeviceExten = pNextObj->DeviceExtension;
//		if (&pDeviceExten->strSymLinkName) {
//			status = IoDeleteSymbolicLink(&pDeviceExten->strSymLinkName);
//			if (!NT_SUCCESS(status)) {
//				DbgPrint("why::delete symlink fail\n");
//			}
//		}
//		pNextObj = pNextObj->NextDevice;
//		if (pDeviceExten->pDevice) {
//			IoDeleteDevice(pDeviceExten->pDevice);
//		}
//	}
//	DbgPrint("why::driver unload success\n");
//	return status;
//}
//
//NTSTATUS CommonDispatch(
//	IN PDEVICE_OBJECT pDeviceObj,
//	IN PIRP pIrp)
//{
//	NTSTATUS  status = STATUS_SUCCESS;
//	pIrp->IoStatus.Status = status;
//	pIrp->IoStatus.Information = 0;
//	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
//	return status;
//}
//
//#pragma LOCKEDCODE
//VOID
//IoTimerRoutine(
//	_In_ struct _DEVICE_OBJECT *DeviceObject,
//	_In_opt_ PVOID Context
//)
//{
//	KdPrint(("why::IoTimerRouine entry\n"));
//	pDEVICE_EXTENSION pDevice = (pDEVICE_EXTENSION)DeviceObject->DeviceExtension;
//
//	//原子方式减一
//	InterlockedDecrement(&pDevice->ulTimeCount);
//
//	//ulTimeCount与0原子方式比较,如果相等就把TIME_OUT赋值给ulTimeOut。返回值为ulTimeCount值
//	LONG timeVal = InterlockedCompareExchange(&pDevice->ulTimeCount, TIME_OUT, 0);
//	if (timeVal == 0)
//	{
//		KdPrint(("why::IoTime out\n"));
//	}
//	//证明该线程运行在任意线程上下文的,不一定是IRP派遣函数调用StartTimer那个进程环境。
//	PEPROCESS pro = IoGetCurrentProcess();
//	PCSTR	proName = (PCSTR)((LONGLONG)pro + 0x450);
//	KdPrint(("why::IoTimerRouine call pro name is %s\n", proName));
//
//	KdPrint(("why::IoTimerRouine leave\n"));
//}
//
//NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject)
//{
//	NTSTATUS		status = STATUS_SUCCESS;
//	UNICODE_STRING	strDeviceName = RTL_CONSTANT_STRING(DEVICE_NAME);
//	UNICODE_STRING	strSymLinkName = RTL_CONSTANT_STRING(DEVICE_SYMLINKNAME);
//	PDEVICE_OBJECT	pDeviceObject;
//	pDEVICE_EXTENSION deviceExten;
//	status = IoCreateDevice(pDriverObject, sizeof(DEVICE_EXTENSION),
//		&strDeviceName, FILE_DEVICE_UNKNOWN,
//		0, TRUE,
//		&pDeviceObject);
//	if (!NT_SUCCESS(status)) {
//		DbgPrint("why::Create Device failed\n");
//		return status;
//	}
//
//	status = IoCreateSymbolicLink(&strSymLinkName, &strDeviceName);
//	if (!NT_SUCCESS(status)) {
//		IoDeleteDevice(pDeviceObject);
//		DbgPrint("why::Create SymLink failed\n");
//		return status;
//	}
//
//	pDeviceObject->Flags |= DO_BUFFERED_IO;
//
//	status = IoInitializeTimer(pDeviceObject, IoTimerRoutine, NULL);
//	if (!NT_SUCCESS(status)) {
//		IoDeleteDevice(pDeviceObject);
//		IoDeleteSymbolicLink(&strSymLinkName);
//		KdPrint(("why::init timer failed\n"));
//		return status;
//	}
//
//	deviceExten = (pDEVICE_EXTENSION)pDeviceObject->DeviceExtension;
//	deviceExten->pDevice = pDeviceObject;
//	deviceExten->strDeviceName = strDeviceName;
//	deviceExten->strSymLinkName = strSymLinkName;
//	return status;
//}
//
//NTSTATUS
//IoControlDispatch(
//	_In_ struct _DEVICE_OBJECT *DeviceObject,
//	_Inout_ struct _IRP *Irp
//)
//{
//	KdPrint(("why::IoControlDispatch entry\n"));
//	NTSTATUS status = STATUS_SUCCESS;
//	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(Irp);
//	pDEVICE_EXTENSION pDevice = (pDEVICE_EXTENSION)DeviceObject->DeviceExtension;
//	ULONG	info = 0;
//	switch (pStack->Parameters.DeviceIoControl.IoControlCode)
//	{
//	case CTL_START_TIMER:
//		KdPrint(("why::IoControlDispatch CTL_START_TIMER\n"));
//		pDevice->ulTimeCount = TIME_OUT;
//		IoStartTimer(DeviceObject);
//		break;
//	case CTL_STOP_TIMER:
//		KdPrint(("why::IoControlDispatch CTL_STOP_TIMER\n"));
//		IoStopTimer(DeviceObject);
//		break;
//	default:
//		status = STATUS_INVALID_VARIANT;
//		break;
//	}
//
//	Irp->IoStatus.Status = status;
//	Irp->IoStatus.Information = info;
//	IoCompleteRequest(Irp, IO_NO_INCREMENT);
//
//	KdPrint(("why::IoControlDispatch leave\n"));
//	return status;
//}
//
//NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegeditPath)
//{
//	UNREFERENCED_PARAMETER(pRegeditPath);
//	NTSTATUS status = STATUS_SUCCESS;
//	pDriverObject->DriverUnload = DriverUnLoad;
//	for (int i = 0; i < ARRAYSIZE(pDriverObject->MajorFunction); i++) {
//		pDriverObject->MajorFunction[i] = CommonDispatch;
//	}
//
//	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControlDispatch;
//
//	status = CreateDevice(pDriverObject);
//
//	return status;
//}