//#include <ntddk.h>
//#define		DEVICE_NAME				TEXT("\\Device\\MyThreadDevice")
//#define		DEVICE_SYMLINKNAME		TEXT("\\??\\ThreadDevice")
//
//#define		ONE_S					-10 * 1000 * 1000
//
//typedef struct _DEVICE_EXTENSION
//{
//	PDEVICE_OBJECT	pDevice;
//	UNICODE_STRING	strDeviceName;
//	UNICODE_STRING	strSymLinkName;
//}DEVICE_EXTENSION, *pDEVICE_EXTENSION;
//
//NTSTATUS
//DriverUnLoad(
//	_In_ struct _DRIVER_OBJECT *DriverObject
//)
//{
//	KdPrint(("why::DriverUnLoad begin\n"));
//
//	NTSTATUS status = STATUS_SUCCESS;
//	PDEVICE_OBJECT pDevice = DriverObject->DeviceObject;
//	while (pDevice != NULL)
//	{
//		pDEVICE_EXTENSION pEx = pDevice->DeviceExtension;
//		UNICODE_STRING symbolLink = pEx->strSymLinkName;
//		status = IoDeleteSymbolicLink(&symbolLink);
//		if (!NT_SUCCESS(status)) {
//			KdPrint(("why::delete symlink fail\n"));
//		}
//		pDevice = pDevice->NextDevice;
//		IoDeleteDevice(pEx->pDevice);
//	}
//	KdPrint(("why::DriverUnLoad end\n"));
//	return status;
//}
//
//NTSTATUS
//DriverCommonDispatch(
//	_In_ struct _DEVICE_OBJECT *DeviceObject,
//	_Inout_ struct _IRP *Irp
//)
//{
//	NTSTATUS status = STATUS_SUCCESS;
//
//	Irp->IoStatus.Information = 0;
//	Irp->IoStatus.Status = status;
//	IoCompleteRequest(Irp, IO_NO_INCREMENT);
//
//	return status;
//}
//
//VOID
//SystemThd(
//	_In_ PVOID StartContext
//)
//{
//	KdPrint(("why::SystemThd entry\n"));
//	PEPROCESS pro = IoGetCurrentProcess();
//	PCSTR ProNameStr = (PCSTR)((LONGLONG)pro + 0x450);
//	KdPrint(("why::SystemThd belong process name is %s\n", ProNameStr));
//	
//	KdPrint(("why::SystemThd leave\n"));
//	PsTerminateSystemThread(STATUS_SUCCESS);	
//}
//
//VOID
//ProcThd(
//	_In_ PVOID StartContext
//)
//{
//	KdPrint(("why::ProcThd entry\n"));
//	PEPROCESS pro = IoGetCurrentProcess();
//	PCSTR ProNameStr = (PCSTR)ExAllocatePool(PagedPool, 15);
//	RtlCopyMemory(ProNameStr, (PCSTR)((LONGLONG)pro + 0x450), 15);
//	KdPrint(("why::ProcThd belong process name is %s\n", ProNameStr));
//	
//	KdPrint(("why::ProcThd leave\n"));
//	PsTerminateSystemThread(STATUS_SUCCESS);
//}
//
//VOID CreateThreadTest()
//{
//	KdPrint(("why::CreateThreadTest entry\n"));
//	HANDLE hSystemThd = NULL;
//	HANDLE hProcThd = NULL;
//	NTSTATUS status = PsCreateSystemThread(&hSystemThd,
//										    0,
//										    NULL,
//										    NULL,
//										    NULL,
//										    SystemThd,
//											NULL);
//	if (!NT_SUCCESS(status))
//	{
//		KdPrint(("why::CreateThread System failed\n"));
//	}
//
//	 status = PsCreateSystemThread(&hProcThd,
//		0,
//		NULL,
//		NtCurrentProcess(),
//		NULL,
//		ProcThd,
//		NULL);
//	if (!NT_SUCCESS(status))
//	{
//		KdPrint(("why::CreateThread Proc failed\n"));
//	}
//	PKWAIT_BLOCK wait_block = (PKWAIT_BLOCK)ExAllocatePool(NonPagedPool, sizeof(KWAIT_BLOCK) * 2);
//	PVOID *ObjectD = (PVOID *)ExAllocatePool(NonPagedPool, sizeof(HANDLE) * 2);
//	//得到对象指针
//	ObReferenceObjectByHandle(hSystemThd, 0, NULL, KernelMode, &ObjectD[0], NULL);
//	ObReferenceObjectByHandle(hProcThd, 0, NULL, KernelMode, &ObjectD[1], NULL);
//	status = KeWaitForMultipleObjects(2, ObjectD, WaitAll, Executive, KernelMode, FALSE, NULL, wait_block);
//	if (NT_SUCCESS(status))
//	{
//		KdPrint(("why::wait all thread end success\n"));
//	}
//	//LARGE_INTEGER sleepTime;
//	//sleepTime.QuadPart = -50 * 1000 * 1000;
//	//KeDelayExecutionThread(KernelMode, FALSE, &sleepTime);
//	ObDereferenceObject(ObjectD[0]);
//	ObDereferenceObject(ObjectD[1]);
//	ExFreePool(ObjectD);
//	ExFreePool(wait_block);
//	KdPrint(("why::CreateThreadTest leave\n"));
//}
//
//VOID EventThd(_In_ PVOID StartContext)
//{
//	KdPrint(("why::EventThd entry\n"));
//	PKEVENT pKevent = (PKEVENT)StartContext;
//	LARGE_INTEGER sleepTime;
//	sleepTime.QuadPart = 6 * ONE_S;
//	KeDelayExecutionThread(KernelMode, FALSE, &sleepTime);
//	//设置事件为激发状态
//	KeSetEvent(pKevent, IO_NO_INCREMENT, FALSE);
//	KdPrint(("why::EventThd leave\n"));
//	PsTerminateSystemThread(STATUS_SUCCESS);
//}
//KEVENT kEvent;
//VOID CreateThreadEventTest()
//{
//	KdPrint(("why::CreateThreadEventTest entry\n"));
//	HANDLE hEventThd = NULL;
//	
//	//初始化一个未激发状态, 自动变为未激发状态的内核事件
//	KeInitializeEvent(&kEvent, SynchronizationEvent, FALSE);
//
//	NTSTATUS status = PsCreateSystemThread(&hEventThd,
//											0,
//											NULL,
//											NULL,
//											NULL,
//											EventThd,
//											&kEvent);
//	if (NT_SUCCESS(status))
//	{
//		KdPrint(("why::CreateThreadEventTest success\n"));
//	}
//	LARGE_INTEGER sleepTime;
//	sleepTime.QuadPart = ONE_S * 5;
//	//如果不等待或超时时还没有调用keSetEvent,PsCreateSystemThread引用了本函数的栈上的变量
//	//当函数退出,同时栈上的变量被回收,再调用keSetEvent,参数KEVENT对象已经被销毁
//	status = KeWaitForSingleObject(&kEvent,
//						Executive,
//						KernelMode,
//						FALSE,
//						&sleepTime);
//	if (status == STATUS_SUCCESS)
//	{
//		KdPrint(("why::CreateThreadEventTest leave\n"));
//	}
//	else if (status == STATUS_TIMEOUT)
//	{
//		KdPrint(("why::CreateThreadEventTest timeout\n"));
//	}
//}
//
//NTSTATUS
//DriverRead(
//	_In_ struct _DEVICE_OBJECT *DeviceObject,
//	_Inout_ struct _IRP *Irp
//)
//{
//	KdPrint(("why::Driver read entry\n"));
//	NTSTATUS status = STATUS_SUCCESS;
//	PIO_STACK_LOCATION pIoStack = IoGetCurrentIrpStackLocation(Irp);
//	ULONG ulReadSize = pIoStack->Parameters.Read.Length;
//
//	//创建线程
//	CreateThreadTest();
//	RtlFillMemory(Irp->AssociatedIrp.SystemBuffer, ulReadSize, 0XAA);
//
//	Irp->IoStatus.Information = ulReadSize;
//	Irp->IoStatus.Status = status;
//	IoCompleteRequest(Irp, IO_NO_INCREMENT);
//	KdPrint(("why::Driver read leave\n"));
//	return status;
//}
//
//
//NTSTATUS CreateDevice(IN PDRIVER_OBJECT	pDriverObject)
//{
//	NTSTATUS status = STATUS_SUCCESS;
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
//
//	return status;
//}
//
//NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObj, IN PUNICODE_STRING pRegistryPath)
//{
//	KdPrint(("why::DriverEntry begin\n"));
//	NTSTATUS status = STATUS_SUCCESS;
//	pDriverObj->DriverUnload = DriverUnLoad;
//
//	for (int i = 0; i < ARRAYSIZE(pDriverObj->MajorFunction); i++)
//	{
//		pDriverObj->MajorFunction[i] = DriverCommonDispatch;
//	}
//	pDriverObj->MajorFunction[IRP_MJ_READ] = DriverRead;
//
//	status = CreateDevice(pDriverObj);
//	CreateThreadEventTest();
//	KdPrint(("why::DriverEntry end\n"));
//	return status;
//}