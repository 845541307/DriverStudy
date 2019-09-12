//#include <ntddk.h>
//
//
//#define		BUFFERREAD_IO
//
///*
//CTL_CODE(DeviceType, Function, Method, Access)
//DeviceType:设备对象的类型,这个类型应和创建设备IoCreateDevice时的类型相匹配。
//Function:这是驱动程序定义的IOCTL码。
//		其中:
//		0x0000~0x7FFF:微软保留。
//		0x8000~0xFFFF:程序员自己定义。
//Method:这个是操作模式,可以是下列四种模式之一。
//		METHOD_BUFFERED:使用缓冲区方式操作。
//		METHOD_IN_DIRECT:使用直接写方式操作。
//		METHOD_OUT_DIRECT:使用直接读方式操作。
//		METHOD_NEITHER:使用其他方式操作。
//Access:访问权限,如果没有特殊要求,一般使用FILE_ANY_ACCESS。
//*/
//
//#ifdef BUFFERREAD_IO
//#define CTL_SPIN_LOCK	 CTL_CODE(FILE_DEVICE_UNKNOWN,\
//							 0x8000,\
//							 METHOD_BUFFERED,\
//							 FILE_ANY_ACCESS)
//#endif
//
//#define		DEVICE_NAME				TEXT("\\Device\\MySpinLockSyncDevice")
//#define		DEVICE_SYMLINKNAME		TEXT("\\??\\SpinLockSyncDevice")
//
//
//typedef struct _DEVICE_EXTENSION
//{
//	PDEVICE_OBJECT	pDevice;
//	UNICODE_STRING	strDeviceName;
//	UNICODE_STRING	strSymLinkName;
//	KSPIN_LOCK		spinLock;
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
//#ifdef BUFFERREAD_IO
//NTSTATUS IoControlDispatch(
//	IN PDEVICE_OBJECT pDeviceObj,
//	IN PIRP pIrp)
//{
//	NTSTATUS  status = STATUS_SUCCESS;
//	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);
//	ULONG ulInLength = pStack->Parameters.DeviceIoControl.InputBufferLength;
//	ULONG ulOutLength = pStack->Parameters.DeviceIoControl.OutputBufferLength;
//	pDEVICE_EXTENSION pDeviceEx = (pDEVICE_EXTENSION)pDeviceObj->DeviceExtension;
//	switch (pStack->Parameters.DeviceIoControl.IoControlCode)
//	{
//	case CTL_SPIN_LOCK:
//	{
//		KdPrint(("why::Get IRQL level is %d\n", KeGetCurrentIrql())); 
//		KIRQL OldIrql;
//		KeAcquireSpinLock(&pDeviceEx->spinLock, &OldIrql);
//		KdPrint(("why::SpinLock entry\n"));
//		KdPrint(("why::Get IRQL level is %d\n", KeGetCurrentIrql()));
//		PUCHAR pPagebuffer = (PUCHAR)ExAllocatePool(NonPagedPool, 10);
//		RtlZeroMemory(pPagebuffer, 10);
//		KdPrint(("why::SpinLock entry ExAllocatePool\n"));
//		PUCHAR pInbuffer = (PUCHAR)pIrp->AssociatedIrp.SystemBuffer;
//		for (int i = 0; i < ulInLength; i++)
//		{
//			KdPrint(("why::buffer data is %X\n", pInbuffer[i]));
//		}
//		RtlFillMemory(pIrp->AssociatedIrp.SystemBuffer, 0xBB, ulOutLength);
//		pIrp->IoStatus.Status = status;
//		pIrp->IoStatus.Information = ulOutLength;
//		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
//		KdPrint(("why::SpinLock leave\n"));
//		ExFreePool(pPagebuffer);
//		KeReleaseSpinLock(&pDeviceEx->spinLock, OldIrql);
//	}
//	break;
//	default:
//		status = STATUS_INVALID_VARIANT;
//	}
//	return status;
//}
//#endif
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
//#ifdef BUFFERREAD_IO
//	pDeviceObject->Flags |= DO_BUFFERED_IO;
//#endif
//
//	deviceExten = (pDEVICE_EXTENSION)pDeviceObject->DeviceExtension;
//	deviceExten->pDevice = pDeviceObject;
//	deviceExten->strDeviceName = strDeviceName;
//	deviceExten->strSymLinkName = strSymLinkName;
//	KeInitializeSpinLock(&deviceExten->spinLock);
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
//	return status;
//}