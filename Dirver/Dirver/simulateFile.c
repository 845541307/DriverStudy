//#include <wdf.h>
#include <ntddk.h>

#define		DEVICE_NAME				TEXT("\\Device\\MyFileDevice")
#define		DEVICE_SYMLINKNAME		TEXT("\\??\\FileDeviceLink")
#define		MAX_FILE_LENGTH			1024

//#define		BUFFERREAD_IO
#define		DIRECT_IO


typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT	pDevice;
	UNICODE_STRING	strDeviceName;
	UNICODE_STRING	strSymLinkName;
	PUCHAR			BufferFile;
	ULONG			ulFileLengthCurrent;
	ULONG			ulFileLengthMax;
}DEVICE_EXTENSION, *pDEVICE_EXTENSION;

NTSTATUS DriverUnLoad(IN PDRIVER_OBJECT pDriverObject)
{
	NTSTATUS  status = STATUS_SUCCESS;
	PDEVICE_OBJECT	pNextObj;
	pNextObj = pDriverObject->DeviceObject;
	while (pNextObj)
	{
		pDEVICE_EXTENSION pDeviceExten = pNextObj->DeviceExtension;
		if (pDeviceExten->BufferFile) {
			ExFreePool(pDeviceExten->BufferFile);
			pDeviceExten->BufferFile = NULL;
		}
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

#ifdef BUFFERREAD_IO
NTSTATUS ReadFileDispatch(
	IN PDEVICE_OBJECT pDeviceObj,
	IN PIRP pIrp)
{
	NTSTATUS	status = STATUS_SUCCESS;
	ULONG		ulReadLength = 0;
	LONGLONG	ulReadOffset = 0;
	PIO_STACK_LOCATION pIoStack;
	pDEVICE_EXTENSION pDeviceExten = pDeviceObj->DeviceExtension;

	pIoStack = IoGetCurrentIrpStackLocation(pIrp);
	ulReadLength = pIoStack->Parameters.Read.Length;
	ulReadOffset = pIoStack->Parameters.Read.ByteOffset.QuadPart;

	if (ulReadOffset + ulReadLength > pDeviceExten->ulFileLengthMax) {
		status = STATUS_FILE_INVALID;
		ulReadLength = 0;
		DbgPrint("why::read file length out of range\n");
	}
	else {
		memcpy(pIrp->AssociatedIrp.SystemBuffer,
			pDeviceExten->BufferFile + ulReadOffset, ulReadLength);
		DbgPrint("why::read file success\n");
	}

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = ulReadLength;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS WriteFileDispatch(
	IN PDEVICE_OBJECT pDeviceObj,
	IN PIRP pIrp)
{
	NTSTATUS		status = STATUS_SUCCESS;
	ULONG			ulWriteLength = 0;
	LONGLONG		ulWriteOffset = 0;
	PIO_STACK_LOCATION pStack;
	pDEVICE_EXTENSION pDeviceExten = pDeviceObj->DeviceExtension;

	pStack = IoGetCurrentIrpStackLocation(pIrp);
	ulWriteLength = pStack->Parameters.Write.Length;
	ulWriteOffset = pStack->Parameters.Write.ByteOffset.QuadPart;

	if (ulWriteOffset + ulWriteLength > pDeviceExten->ulFileLengthMax) {
		status = STATUS_FILE_INVALID;
		ulWriteLength = 0;
		DbgPrint("why::write file fail\n");
	}
	else {
		memcpy(pDeviceExten->BufferFile + ulWriteOffset,
			pIrp->AssociatedIrp.SystemBuffer, ulWriteLength);
		if (ulWriteOffset + ulWriteLength > pDeviceExten->ulFileLengthCurrent) {
			pDeviceExten->ulFileLengthCurrent = ulWriteOffset + ulWriteLength;
		}
		DbgPrint("why::write file success\n");
	}

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = ulWriteLength;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}


#endif // BUFFERREAD_IO

#ifdef DIRECT_IO
NTSTATUS ReadFileDispatch(
	IN PDEVICE_OBJECT pDeviceObj,
	IN PIRP pIrp)
{
	NTSTATUS	status = STATUS_SUCCESS;
	ULONG		ulReadLength = 0;
	LONGLONG	ulReadOffset = 0;
	ULONG		ulMdlReadLength = 0;
	PVOID		pMdlAddress = NULL;
	ULONG		ulMdlOffset = 0;
	PIO_STACK_LOCATION pIoStack;
	pDEVICE_EXTENSION pDeviceExten = pDeviceObj->DeviceExtension;

	pIoStack = IoGetCurrentIrpStackLocation(pIrp);
	ulReadLength = pIoStack->Parameters.Read.Length;
	ulReadOffset = pIoStack->Parameters.Read.ByteOffset.QuadPart;

	ulMdlReadLength = MmGetMdlByteCount(pIrp->MdlAddress);
	pMdlAddress = MmGetMdlBaseVa(pIrp->MdlAddress);
	ulMdlOffset = MmGetMdlByteOffset(pIrp->MdlAddress);

	KdPrint(("why::mdl_address:0X%08X\n", pMdlAddress));
	KdPrint(("why::mdl_length:%d\n", ulMdlReadLength));
	KdPrint(("why::mdl_offset:%d\n", ulMdlOffset));

	//MDL的长度应该和读长度相等，否则该操作应该设为不成功
	if ((ulReadOffset + ulReadLength > pDeviceExten->ulFileLengthMax) || (ulMdlReadLength != ulReadLength)) {
		status = STATUS_FILE_INVALID;
		ulReadLength = 0;
		DbgPrint("why::read file length out of range or mdl length error\n");
	}
	else {

		//用MmGetSystemAddressForMdlSafe得到MDL在内核模式下的映射
		PVOID pKernalAddress = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
		KdPrint(("why::kernel_address:0X%08X\n", pKernalAddress));
		memcpy(pKernalAddress,
			pDeviceExten->BufferFile + ulReadOffset, ulReadLength);
		DbgPrint("why::read file success\n");
	}

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = ulReadLength;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS WriteFileDispatch(
	IN PDEVICE_OBJECT pDeviceObj,
	IN PIRP pIrp)
{
	NTSTATUS		status = STATUS_SUCCESS;
	ULONG			ulWriteLength = 0;
	LONGLONG		ulWriteOffset = 0;
	ULONG		ulMdlReadLength = 0;
	PVOID		pMdlAddress = NULL;
	ULONG		ulMdlOffset = 0;
	PIO_STACK_LOCATION pStack;
	pDEVICE_EXTENSION pDeviceExten = pDeviceObj->DeviceExtension;

	pStack = IoGetCurrentIrpStackLocation(pIrp);
	ulWriteLength = pStack->Parameters.Write.Length;
	ulWriteOffset = pStack->Parameters.Write.ByteOffset.QuadPart;

	ulMdlReadLength = MmGetMdlByteCount(pIrp->MdlAddress);
	pMdlAddress = MmGetMdlBaseVa(pIrp->MdlAddress);
	ulMdlOffset = MmGetMdlByteOffset(pIrp->MdlAddress);

	KdPrint(("why::mdl_address:0X%08X\n", pMdlAddress));
	KdPrint(("why::mdl_length:%d\n", ulMdlReadLength));
	KdPrint(("why::mdl_offset:%d\n", ulMdlOffset));

	//MDL的长度应该和读长度相等，否则该操作应该设为不成功
	if ((ulWriteOffset + ulWriteLength > pDeviceExten->ulFileLengthMax) || (ulMdlReadLength != ulWriteLength)) {
		status = STATUS_FILE_INVALID;
		ulWriteLength = 0;
		DbgPrint("why::write file fail or mdl length error\n");
	}
	else {

		//用MmGetSystemAddressForMdlSafe得到MDL在内核模式下的映射
		PVOID pKernalAddress = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
		KdPrint(("why::kernel_address:0X%08X\n", pKernalAddress));
		memcpy(pDeviceExten->BufferFile + ulWriteOffset,
			pKernalAddress, ulWriteLength);
		if (ulWriteOffset + ulWriteLength > pDeviceExten->ulFileLengthCurrent) {
			pDeviceExten->ulFileLengthCurrent = ulWriteOffset + ulWriteLength;
		}
		DbgPrint("why::write file success\n");
	}

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = ulWriteLength;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}
#endif // DIRECT_IO

NTSTATUS GetFileLengthDispatch(
	IN PDEVICE_OBJECT pDeviceObj,
	IN PIRP pIrp)
{
	NTSTATUS				status = STATUS_SUCCESS;
	PIO_STACK_LOCATION		pStack = IoGetCurrentIrpStackLocation(pIrp);
	pDEVICE_EXTENSION		pDeviceExten = pDeviceObj->DeviceExtension;

	if (pStack->Parameters.QueryFile.FileInformationClass == FileStandardInformation) {
		PFILE_STANDARD_INFORMATION fileInfo =
			(PFILE_STANDARD_INFORMATION)pIrp->AssociatedIrp.SystemBuffer;
		fileInfo->EndOfFile = RtlConvertLongToLargeInteger(pDeviceExten->ulFileLengthCurrent);
		DbgPrint("why::get file length success\n");
	}

	DbgPrint("why::query file length is %d\n", pStack->Parameters.QueryFile.Length);
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = pStack->Parameters.QueryFile.Length;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
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
	if (!NT_SUCCESS(status)){
		DbgPrint("why::Create Device failed\n");
		return status;
	}

	status = IoCreateSymbolicLink(&strSymLinkName, &strDeviceName);
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(pDeviceObject);
		DbgPrint("why::Create SymLink failed\n");
		return status;
	}

#ifdef BUFFERREAD_IO
	pDeviceObject->Flags |= DO_BUFFERED_IO;
#endif // BUFFERREAD_IO

#ifdef DIRECT_IO
	pDeviceObject->Flags |= DO_DIRECT_IO;
#endif // DIRECT_IO

	
	deviceExten = (pDEVICE_EXTENSION)pDeviceObject->DeviceExtension;
	deviceExten->pDevice = pDeviceObject;
	deviceExten->strDeviceName = strDeviceName;
	deviceExten->strSymLinkName = strSymLinkName;
	deviceExten->BufferFile = (PUCHAR)ExAllocatePool(PagedPool, MAX_FILE_LENGTH);
	deviceExten->ulFileLengthCurrent = 0;
	deviceExten->ulFileLengthMax = MAX_FILE_LENGTH;

	return status;
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath)
{
	DbgPrint("why::DriverEntry\n");
	NTSTATUS status = STATUS_SUCCESS;
	pDriverObject->DriverUnload = DriverUnLoad;
	for (int i = 0; i < ARRAYSIZE(pDriverObject->MajorFunction); i++){
		pDriverObject->MajorFunction[i] = CommonDispatch;
	}

	pDriverObject->MajorFunction[IRP_MJ_READ] = ReadFileDispatch;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = WriteFileDispatch;
	pDriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] = GetFileLengthDispatch;

	status = CreateDevice(pDriverObject);

	return status;
}