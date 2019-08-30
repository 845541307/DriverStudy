#include <ntddk.h>
#define		DEVICE_NAME				TEXT("\\Device\\MyFileDevice")
#define		DEVICE_SYMLINKNAME		TEXT("\\??\\FileDevice")
#define		ARRAY_NUMBER 50
#define		BUFFER_SIZE 1024
#define		MYSTRING	TEXT("wanghy")
#define		FILENAME	TEXT("\\??\\C:\\why.log")

typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT	pDevice;
	UNICODE_STRING	strDeviceName;
	UNICODE_STRING	strSymLinkName;
}DEVICE_EXTENSION, *pDEVICE_EXTENSION;

VOID CreateFileTest()
{
	OBJECT_ATTRIBUTES objectArr;
	IO_STATUS_BLOCK ioStatus;
	HANDLE hFile;
	UNICODE_STRING	logFileStr = RTL_CONSTANT_STRING(FILENAME);
	//初始化objectAttributes
	InitializeObjectAttributes(&objectArr,
							   &logFileStr, 
							   OBJ_CASE_INSENSITIVE,
							   NULL,
							   NULL);

	//创建文件
	NTSTATUS status = ZwCreateFile(&hFile, 
								GENERIC_WRITE, 
								&objectArr, 
								&ioStatus,
								NULL,
								FILE_ATTRIBUTE_NORMAL,
								FILE_SHARE_READ,
								FILE_OPEN_IF,//文件存在时打开文件,不存在时创建文件
								FILE_SYNCHRONOUS_IO_NONALERT,
								NULL,
								0);

	if (NT_SUCCESS(status))
	{
		KdPrint(("why::Create File Success\n"));
	}
	else
	{
		KdPrint(("why::Create File Failed\n"));
	}

	ZwClose(hFile);
}

VOID OpenFileTest()
{
	OBJECT_ATTRIBUTES objectArr;
	IO_STATUS_BLOCK ioStatus;
	HANDLE hFile;
	UNICODE_STRING	logFileStr = RTL_CONSTANT_STRING(FILENAME);

	//初始化objectAttributes
	InitializeObjectAttributes(&objectArr,
		&logFileStr,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	NTSTATUS status = ZwOpenFile(&hFile, 
								GENERIC_ALL, 
								&objectArr,
								&ioStatus,
								FILE_SHARE_READ,
								FILE_SYNCHRONOUS_IO_NONALERT);

	if (NT_SUCCESS(status))
	{
		KdPrint(("why::Open File Success\n"));
	}
	else
	{
		KdPrint(("why::Open File Failed\n"));
	}

	ZwClose(hFile);
}

VOID FileAttributesTest()
{
	OBJECT_ATTRIBUTES objectArr;
	IO_STATUS_BLOCK ioStatus;
	HANDLE hFile;
	UNICODE_STRING	logFileStr = RTL_CONSTANT_STRING(FILENAME);

	//初始化objectAttributes
	InitializeObjectAttributes(&objectArr,
		&logFileStr,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	NTSTATUS status = ZwOpenFile(&hFile,
		GENERIC_ALL,
		&objectArr,
		&ioStatus,
		FILE_SHARE_READ,
		FILE_SYNCHRONOUS_IO_NONALERT);

	if (NT_SUCCESS(status))
	{
		KdPrint(("why::Open File Success\n"));
	}
	else
	{
		KdPrint(("why::Open File Failed\n"));
	}

	FILE_STANDARD_INFORMATION fsi;
	//读取文件长度
	status = ZwQueryInformationFile(hFile,
									&ioStatus,
									&fsi,
									sizeof(FILE_STANDARD_INFORMATION),
									FileStandardInformation);
	if (NT_SUCCESS(status))
	{
		KdPrint(("why::Query File Info Success\n"));
		KdPrint(("why::file length:%u\n", fsi.EndOfFile.QuadPart));
	}
	else
	{
		KdPrint(("why::Query File Info Failed\n"));
	}

	//修改当前文件指针
	FILE_POSITION_INFORMATION fpi;
	fpi.CurrentByteOffset.QuadPart = 100i64;
	status = ZwSetInformationFile(hFile,
								&ioStatus,
								&fpi,
								sizeof(FILE_POSITION_INFORMATION),
								FilePositionInformation);
	if (NT_SUCCESS(status))
	{
		KdPrint(("update the file pointer successfully.\n"));
	}


	ZwClose(hFile);
}

VOID WriteFileTest()
{
	OBJECT_ATTRIBUTES objectArr;
	IO_STATUS_BLOCK ioStatus;
	HANDLE hFile;
	UNICODE_STRING	logFileStr = RTL_CONSTANT_STRING(FILENAME);

	//初始化objectAttributes
	InitializeObjectAttributes(&objectArr,
		&logFileStr,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	NTSTATUS status = ZwCreateFile(&hFile,
		GENERIC_WRITE,
		&objectArr,
		&ioStatus,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN_IF,//文件存在时打开文件,不存在时创建文件
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);

	if (NT_SUCCESS(status))
	{
		KdPrint(("why::Open File Success\n"));
	}
	else
	{
		KdPrint(("why::Open File Failed\n"));
	}

	PUCHAR pBuffer = (PUCHAR)ExAllocatePool(PagedPool, BUFFER_SIZE);
	RtlFillMemory(pBuffer, BUFFER_SIZE, 0);
	RtlCopyMemory(pBuffer, MYSTRING, 2 * wcslen(MYSTRING));
	KdPrint(("why::The program will write %d bytes\n", BUFFER_SIZE));
	status = ZwWriteFile(hFile,
						NULL,
						NULL,
						NULL,
						&ioStatus,
						pBuffer,
						BUFFER_SIZE,
						0,
						NULL);
	if (NT_SUCCESS(status))
	{
		KdPrint(("why::The program will write %d bytes\n", ioStatus.Information));
	}

	//追加数据
	LARGE_INTEGER num;
	num.QuadPart = 1024i64;
	RtlFillMemory(pBuffer, BUFFER_SIZE, 0);
	RtlCopyMemory(pBuffer, MYSTRING, 2 * wcslen(MYSTRING));
	KdPrint(("why::The program will write %d bytes\n", BUFFER_SIZE));
	status = ZwWriteFile(hFile,
		NULL,
		NULL,
		NULL,
		&ioStatus,
		pBuffer,
		BUFFER_SIZE,
		&num,
		NULL);
	if (NT_SUCCESS(status))
	{
		KdPrint(("why::The program will write %d bytes\n", ioStatus.Information));
	}

	ZwClose(hFile);
	ExFreePool(pBuffer);
}

VOID ReadFileTest()
{
	OBJECT_ATTRIBUTES objectArr;
	IO_STATUS_BLOCK ioStatus;
	HANDLE hFile;
	UNICODE_STRING logFileStr = RTL_CONSTANT_STRING(FILENAME);

	//初始化objectAttributes
	InitializeObjectAttributes(&objectArr,
		&logFileStr,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	NTSTATUS status = ZwCreateFile(&hFile,
		GENERIC_WRITE,
		&objectArr,
		&ioStatus,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN_IF,//文件存在时打开文件,不存在时创建文件
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("why::Open file failed\n"));
		return;
	}

	FILE_STANDARD_INFORMATION fsi;
	status = ZwQueryInformationFile(hFile,
									&ioStatus,
									&fsi,
									sizeof(FILE_STANDARD_INFORMATION),
									FileStandardInformation);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("why::QueryInformation File failed\n"));
		return;
	}
	KdPrint(("why::The program will Read %d bytes\n", fsi.EndOfFile.QuadPart));

	//分配读取文件内存
	PUCHAR pbuffer = (PUCHAR)ExAllocatePool(PagedPool, fsi.EndOfFile.QuadPart);
	status = ZwReadFile(hFile,
						NULL,
						NULL,
						NULL,
						&ioStatus,
						pbuffer,
						fsi.EndOfFile.QuadPart,
						0,
						NULL);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("why::read File failed\n"));
		return;
	}

	KdPrint(("why::The Read file  %d bytes\n", ioStatus.Information));
	KdPrint(("why::Read file content is %S\n", pbuffer));

	LARGE_INTEGER num;
	num.QuadPart = ioStatus.Information;
	//再次写入文件,看读取是否正确
	status = ZwWriteFile(hFile,
		NULL,
		NULL,
		NULL,
		&ioStatus,
		pbuffer,
		ioStatus.Information,
		&num,
		NULL);
	if (NT_SUCCESS(status))
	{
		KdPrint(("why::The program will write %d bytes\n", ioStatus.Information));
	}

	ZwClose(hFile);
	ExFreePool(pbuffer);
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
	//CreateFileTest();
	//OpenFileTest();
	//FileAttributesTest();
	WriteFileTest();
	ReadFileTest();
	KdPrint(("why::DriverEntry end\n"));
	return status;
}