#include <ntddk.h>
#define		DEVICE_NAME				TEXT("\\Device\\MyLookasideDevice")
#define		DEVICE_SYMLINKNAME		TEXT("\\??\\LookasideDevice")
#define ARRAY_NUMBER 50
#define BUFFER_SIZE 1024
typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT	pDevice;
	UNICODE_STRING	strDeviceName;
	UNICODE_STRING	strSymLinkName;
}DEVICE_EXTENSION, *pDEVICE_EXTENSION;

typedef struct _MYDATASTRUCT
{
	CHAR buffer[64];
} MYDATASTRUCT, *PMYDATASTRUCT;

VOID RtlTest()
{
	PUCHAR pBuffer = (PUCHAR)ExAllocatePool(PagedPool, BUFFER_SIZE);
	//用零填充内存
	RtlZeroMemory(pBuffer, BUFFER_SIZE);

	PUCHAR pBuffer2 = (PUCHAR)ExAllocatePool(PagedPool, BUFFER_SIZE);
	//用固定字节填充内存
	RtlFillMemory(pBuffer2, BUFFER_SIZE, 0xAA);

	//内存拷贝
	RtlCopyMemory(pBuffer, pBuffer2, BUFFER_SIZE);

	//判断内存是否一致
	ULONG ulRet = RtlCompareMemory(pBuffer, pBuffer2, BUFFER_SIZE);
	if (ulRet == BUFFER_SIZE)
	{
		KdPrint(("The two blocks are same.\n"));
	}
	if (RtlEqualMemory(pBuffer, pBuffer2, BUFFER_SIZE) != 0)
	{
		KdPrint(("The two blocks are same.\n"));
	}
}

VOID LookasideTest()
{
	KdPrint(("why::Lookaside begin\n"));
	//初始化Lookaside对象
	PAGED_LOOKASIDE_LIST pageList;
	ExInitializePagedLookasideList(&pageList, NULL, NULL, 0, sizeof(MYDATASTRUCT), '1234', 0);
	
	PMYDATASTRUCT MyObjectArray[ARRAY_NUMBER];
	//模拟频繁申请内存
	for (int i = 0; i < ARRAY_NUMBER; i++)
	{
		MyObjectArray[i] = (PMYDATASTRUCT)ExAllocateFromPagedLookasideList(&pageList);
	}

	//模拟频繁回收内存
	for (int j = 0; j < ARRAY_NUMBER; j++)
	{
		ExFreeToPagedLookasideList(&pageList, MyObjectArray[j]);
		MyObjectArray[j] = NULL;
	}

	//删除Lookaside对象
	ExDeletePagedLookasideList(&pageList);
	KdPrint(("why::Lookaside end\n")); 
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
	LookasideTest();
	KdPrint(("why::DriverEntry end\n"));
	return status;
}