#include <ntddk.h>
#define		DEVICE_NAME				TEXT("\\Device\\MyTryExceptDevice")
#define		DEVICE_SYMLINKNAME		TEXT("\\??\\TryExceptDevice")
#define ARRAY_NUMBER 50
#define BUFFER_SIZE 1024
typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT	pDevice;
	UNICODE_STRING	strDeviceName;
	UNICODE_STRING	strSymLinkName;
}DEVICE_EXTENSION, *pDEVICE_EXTENSION;

VOID ProbeTest()
{
	PVOID pNUll = NULL;
	KdPrint(("why::Entry ProbeTest\n"));
	try
	{
		KdPrint(("why::Entry try\n"));
		ProbeForWrite(pNUll, 10, 4);
		KdPrint(("why::Leave try\n"));
	}
	except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("why::catch the except\n"));
	}
	KdPrint(("why::Leave ProbeTest\n"));
}

VOID TryFinallyTest()
{
	try
	{
		KdPrint(("why::Entry  try\n"));
		return;
	}
	finally
	{
		KdPrint(("why::Entry Finally \n"));
	}
}

NTSTATUS
DriverUnLoad(
	_In_ struct _DRIVER_OBJECT *DriverObject
)
{
	KdPrint(("why::DriverUnLoad begin\n"));

	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT pDevice = DriverObject->DeviceObject;
	while (pDevice != NULL)
	{
		pDEVICE_EXTENSION pEx = pDevice->DeviceExtension;
		UNICODE_STRING symbolLink = pEx->strSymLinkName;
		status = IoDeleteSymbolicLink(&symbolLink);
		if (!NT_SUCCESS(status)) {
			KdPrint(("why::delete symlink fail\n"));
		}
		pDevice = pDevice->NextDevice;
		IoDeleteDevice(pEx->pDevice);
	}
	KdPrint(("why::DriverUnLoad end\n"));
	return status;
}

NTSTATUS
DriverCommonDispatch(
	_In_ struct _DEVICE_OBJECT *DeviceObject,
	_Inout_ struct _IRP *Irp
)
{
	NTSTATUS status = STATUS_SUCCESS;

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}

NTSTATUS CreateDevice(IN PDRIVER_OBJECT	pDriverObject)
{
	NTSTATUS status = STATUS_SUCCESS;
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

	return status;
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObj, IN PUNICODE_STRING pRegistryPath)
{
	KdPrint(("why::DriverEntry begin\n"));
	NTSTATUS status = STATUS_SUCCESS;
	pDriverObj->DriverUnload = DriverUnLoad;

	for (int i = 0; i < ARRAYSIZE(pDriverObj->MajorFunction); i++)
	{
		pDriverObj->MajorFunction[i] = DriverCommonDispatch;
	}

	status = CreateDevice(pDriverObj);
	ProbeTest();
	TryFinallyTest();
	KdPrint(("why::DriverEntry end\n"));
	return status;
}