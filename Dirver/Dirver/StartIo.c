//#include <ntddk.h>
//
//#define		PAGEDCODE				code_seg("PAGE")
//#define		LOCKEDCODE				code_seg()
//#define		INITCODE				code_seg("INIT")
//#define		DEVICE_NAME				TEXT("\\Device\\MyStartIoDevice")
//#define		DEVICE_SYMLINKNAME		TEXT("\\??\\StartIoDevice")
//#define		ONE_S					-10 * 1000 * 1000
//
//typedef struct _DEVICE_EXTENSION
//{
//	PDEVICE_OBJECT	pDevice;
//	UNICODE_STRING	strDeviceName;
//	UNICODE_STRING	strSymLinkName;
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
//VOID
//IRP_CANCEL(
//	_Inout_ struct _DEVICE_OBJECT *DeviceObject,
//	_Inout_ _IRQL_uses_cancel_ struct _IRP *Irp
//)
//{
//	KdPrint(("why::IRP_CANCEL entry\n"));
//
//	if (Irp == DeviceObject->CurrentIrp)
//	{
//		//这个IRP正在或者即将被StartIo处理,所以这里应该立即释放自旋锁什么都不做返回
//		KdPrint(("why::IRP_CANCEL irp processed now\n"));
//		KIRQL oldirql = Irp->CancelIrql;
//		IoReleaseCancelSpinLock(Irp->CancelIrql);
//		//从队列中抽取下一个IRP进行处理
//		IoStartNextPacket(DeviceObject, TRUE);
//		//降低IRQL ????
//		KeLowerIrql(oldirql);
//	}
//	else
//	{
//		KdPrint(("why::IRP_CANCEL irp canceled\n"));
//		//从系统队列中取出IRP
//		KeRemoveEntryDeviceQueue(&DeviceObject->DeviceQueue, &Irp->Tail.Overlay.DeviceQueueEntry);
//		IoReleaseCancelSpinLock(Irp->CancelIrql);
//	}
//
//	Irp->IoStatus.Status = STATUS_CANCELLED;
//	Irp->IoStatus.Information = 0;
//	IoCompleteRequest(Irp, IO_NO_INCREMENT);
//	KdPrint(("why::IRP_CANCEL leave\n"));
//}
//
//NTSTATUS IoReadDispatch(
//	IN PDEVICE_OBJECT pDeviceObj,
//	IN PIRP pIrp)
//{
//	KdPrint(("why::IoReadDispatch entry threadId is %d\n", PsGetThreadId(pIrp->Tail.Overlay.Thread)));
//	NTSTATUS status = STATUS_SUCCESS;
//	pDEVICE_EXTENSION pDevEx = (pDEVICE_EXTENSION)pDeviceObj->DeviceExtension;
//	IoMarkIrpPending(pIrp);
//	//将IRP插入系统队列中
//	IoStartPacket(pDeviceObj, pIrp, 0, IRP_CANCEL);
//	status = STATUS_PENDING;
//	KdPrint(("why::IoReadDispatch leave threadId is %d\n", PsGetThreadId(pIrp->Tail.Overlay.Thread)));
//	return status;
//}
//
//#pragma LOCKEDCODE
//VOID
//StartIoDispatch(
//	_Inout_ struct _DEVICE_OBJECT *DeviceObject,
//	_Inout_ struct _IRP *Irp
//)
//{
//	KdPrint(("why::StartIoDispatch entry\n"));
//	
//	KIRQL oldirql;
//	IoAcquireCancelSpinLock(&oldirql);
//	if (Irp != DeviceObject->CurrentIrp || Irp->Cancel)
//	{
//		//如果当前有正在处理的IRP,则简单的入队列并直接返回
//		//入队列是由系统完成的,在SatrtIO中不用处理
//		IoReleaseCancelSpinLock(oldirql);
//		KdPrint(("why::StartIoDispatch irp entry queue\n"));
//		return;
//	}
//	else
//	{
//		//正在处理IRP,所以不允许调用取消例程
//		IoSetCancelRoutine(Irp, NULL);
//		IoReleaseCancelSpinLock(oldirql);
//	}
//
//	KEVENT event;
//	KeInitializeEvent(&event, NotificationEvent, FALSE);
//	LARGE_INTEGER timeout;
//	timeout.QuadPart = 3 * ONE_S;
//	KdPrint(("why::StartIoDispatch process Irp now\n"));
//	//模拟IRP处理3秒
//	KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, &timeout);
//	Irp->IoStatus.Status = STATUS_SUCCESS;
//	Irp->IoStatus.Information = 0;
//	IoCompleteRequest(Irp, IO_NO_INCREMENT);
//	IoStartNextPacket(DeviceObject, TRUE);
//
//	KdPrint(("why::StartIoDispatch leave\n"));
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
//	deviceExten = (pDEVICE_EXTENSION)pDeviceObject->DeviceExtension;
//	deviceExten->pDevice = pDeviceObject;
//	deviceExten->strDeviceName = strDeviceName;
//	deviceExten->strSymLinkName = strSymLinkName;
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
//	pDriverObject->MajorFunction[IRP_MJ_READ] = IoReadDispatch;
//	pDriverObject->DriverStartIo = StartIoDispatch;
//
//	status = CreateDevice(pDriverObject);
//	return status;
//}