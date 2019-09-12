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
//#define CTL_COMM_EVENT	 CTL_CODE(FILE_DEVICE_UNKNOWN,\
//							 0x8000,\
//							 METHOD_BUFFERED,\
//							 FILE_ANY_ACCESS)
//#endif
//
//#define		DEVICE_NAME				TEXT("\\Device\\MyCommEventDevice")
//#define		DEVICE_SYMLINKNAME		TEXT("\\??\\CommEventDevice")
//
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
//#ifdef BUFFERREAD_IO
//NTSTATUS IoControlDispatch(
//	IN PDEVICE_OBJECT pDeviceObj,
//	IN PIRP pIrp)
//{
//	NTSTATUS  status = STATUS_SUCCESS;
//	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);
//	ULONG ulInLength = pStack->Parameters.DeviceIoControl.InputBufferLength;
//	ULONG ulOutLength = pStack->Parameters.DeviceIoControl.OutputBufferLength;
//	ULONG ulInfoSize = 0;
//	switch (pStack->Parameters.DeviceIoControl.IoControlCode)
//	{
//	case CTL_COMM_EVENT:
//	{
//		DbgPrint("why::io control type is CTL_COMM_EVENT\n");
//		HANDLE hUserEvent = *(HANDLE *)pIrp->AssociatedIrp.SystemBuffer;
//		PKEVENT pKevent;
//		status = ObReferenceObjectByHandle(hUserEvent,
//								EVENT_MODIFY_STATE,
//								*ExEventObjectType,
//								KernelMode,
//								(PVOID *)&pKevent,
//								NULL);
//		if (NT_SUCCESS(status))
//		{
//			KeSetEvent(pKevent, IO_NO_INCREMENT, FALSE);
//		}
//		ObDereferenceObject(pKevent);
//	}
//	break;
//	default:
//		status = STATUS_INVALID_VARIANT;
//	}
//
//	pIrp->IoStatus.Status = status;
//	pIrp->IoStatus.Information = ulInfoSize;
//	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
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
//
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