#include <ntddk.h>
#define		DEVICE_NAME				TEXT("\\Device\\MyRegeditDevice")
#define		DEVICE_SYMLINKNAME		TEXT("\\??\\RegeditDevice")
#define		ARRAY_NUMBER 50
#define		BUFFER_SIZE 1024
#define		MYSTRING	TEXT("wanghy")
#define		REGEDIT_ROOTPATH			TEXT("\\REGISTRY\\MACHINE\\SOFTWARE\\why")
#define		REGEDIT_SUBPATH				TEXT("subItem")
#define		REGEDIT_SETPATH				TEXT("\\REGISTRY\\MACHINE\\SOFTWARE\\why\\subItem")
#define		KEYNAME						TEXT("test")
#define		RTL_SETPATH					TEXT("\\REGISTRY\\MACHINE\\SOFTWARE\\why\\subItem2")

typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT	pDevice;
	UNICODE_STRING	strDeviceName;
	UNICODE_STRING	strSymLinkName;
}DEVICE_EXTENSION, *pDEVICE_EXTENSION;

VOID CreateRegedit()
{
	OBJECT_ATTRIBUTES	objArr;
	OBJECT_ATTRIBUTES	subObjAtt;
	UNICODE_STRING		regRootPathStr = RTL_CONSTANT_STRING(REGEDIT_ROOTPATH);
	UNICODE_STRING		regSubPathStr = RTL_CONSTANT_STRING(REGEDIT_SUBPATH);
	HANDLE				hKey = NULL;
	HANDLE				hSubKey = NULL;
	ULONG	ulResult = 0;

	InitializeObjectAttributes(&objArr,
							  &regRootPathStr,
								OBJ_CASE_INSENSITIVE,//对大小写敏感 
								NULL,
								NULL);

	NTSTATUS status = ZwCreateKey(&hKey,
								KEY_ALL_ACCESS,
								&objArr,
								0,
								NULL,
								REG_OPTION_NON_VOLATILE,
								&ulResult);
	if (NT_SUCCESS(status))
	{
		//判断是被新创建，还是已经被创建
		if (ulResult == REG_CREATED_NEW_KEY)
		{
			KdPrint(("why::The register item is created\n"));
		}
		else if (ulResult == REG_OPENED_EXISTING_KEY)
		{
			KdPrint(("why::The register item has been created,and now is opened\n"));
		}
	}


	//创建或打开子项
	InitializeObjectAttributes(&subObjAtt,
							&regSubPathStr,
							OBJ_CASE_INSENSITIVE,//对大小写敏感 
							hKey,
							NULL);
	status = ZwCreateKey(&hSubKey,
						KEY_ALL_ACCESS,
						&subObjAtt,
						0,
						NULL,
						REG_OPTION_NON_VOLATILE,
						&ulResult);

	if (NT_SUCCESS(status))
	{
		//判断是被新创建，还是已经被创建
		if (ulResult == REG_CREATED_NEW_KEY)
		{
			KdPrint(("why::The sub register item is created\n"));
		}
		else if (ulResult == REG_OPENED_EXISTING_KEY)
		{
			KdPrint(("why::The sub register item has been created,and now is opened\n"));
		}
	}

	ZwClose(hSubKey);
	ZwClose(hKey);
}

VOID SetRegedit()
{
	OBJECT_ATTRIBUTES objArr;
	UNICODE_STRING		regPath = RTL_CONSTANT_STRING(REGEDIT_SETPATH);
	UNICODE_STRING		keyName = RTL_CONSTANT_STRING(KEYNAME);
	ULONG				ulResult = 0;
	HANDLE				hKey = NULL;

	InitializeObjectAttributes(&objArr,
							  &regPath,
							  OBJ_CASE_INSENSITIVE,
							  NULL,
							  NULL);

	NTSTATUS status = ZwOpenKey(&hKey,
			 KEY_ALL_ACCESS,
			 &objArr);

	if (NT_SUCCESS(status))
	{
		KdPrint(("why::Open key success\n"));
	}

	PWCHAR	keyValue = L"why";
	ULONG	keyValueLen = 2 * wcslen(keyValue) + 2;
	status = ZwSetValueKey(hKey,
						 &keyName,
						 0,
						 REG_SZ,
						 keyValue,
						 keyValueLen);


	if (NT_SUCCESS(status))
	{
		KdPrint(("why::set key success\n"));
	}

	ZwClose(hKey);
}

VOID QueryRegTest()
{
	OBJECT_ATTRIBUTES objArr;
	UNICODE_STRING		regPath = RTL_CONSTANT_STRING(REGEDIT_SETPATH);
	UNICODE_STRING		keyName = RTL_CONSTANT_STRING(KEYNAME);
	HANDLE				hKey = NULL;

	InitializeObjectAttributes(&objArr,
		&regPath,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	NTSTATUS status = ZwOpenKey(&hKey,
		KEY_ALL_ACCESS,
		&objArr);

	if (NT_SUCCESS(status))
	{
		KdPrint(("why::Open key success\n"));
	}

	ULONG	ulDataSize = 0;
	status = ZwQueryValueKey(hKey,
							&keyName,
							KeyValuePartialInformation,
							NULL,
							0,
							&ulDataSize);
	if (status == STATUS_OBJECT_NAME_NOT_FOUND || ulDataSize == 0)
	{
		ZwClose(hKey);
		KdPrint(("why::The item is not exist, data size is %d, status is %d\n", 
					ulDataSize, status));
		return;
	}
	KdPrint(("why::Query key success, need data length is %d\n", ulDataSize));

	PKEY_VALUE_PARTIAL_INFORMATION pvpi =
		(PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePool(PagedPool, ulDataSize);

	status = ZwQueryValueKey(hKey,
							&keyName,
							KeyValuePartialInformation,
							pvpi,
							ulDataSize,
							&ulDataSize);

	if (!NT_SUCCESS(status))
	{
		ExFreePool(pvpi);
		ZwClose(hKey);
		KdPrint(("Read regsiter error\n"));
		return;
	}

	if (pvpi->Type == REG_SZ)
	{
		KdPrint(("why::read reg key content is %S\n", pvpi->Data));
	}


	ExFreePool(pvpi);
	ZwClose(hKey);
}

VOID RTLRegedit()
{
	//保证传入RTL_SETPATH路径均为存在
	NTSTATUS status = RtlCreateRegistryKey(RTL_REGISTRY_ABSOLUTE, RTL_SETPATH);
	if (NT_SUCCESS(status))
	{
		KdPrint(("why::Rtl create key success\n"));
	}
	status = RtlCheckRegistryKey(RTL_REGISTRY_ABSOLUTE, RTL_SETPATH);
	if (NT_SUCCESS(status))
	{
		KdPrint(("why::The item is exist\n"));
	}

	PWCHAR	strKeyValue = TEXT("test");
	ULONG	ulKeyValueSize = 2 * wcslen(strKeyValue) + 2;
	status = RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
									RTL_SETPATH,
									KEYNAME,
									REG_SZ,
									strKeyValue,
									ulKeyValueSize);

	if (NT_SUCCESS(status))
	{
		KdPrint(("why::Write the REG_SZ value succuessfully\n"));
	}

	RTL_QUERY_REGISTRY_TABLE paramTable[2];
	RtlZeroMemory(paramTable, sizeof(paramTable));
	PWCHAR	strContent = (PWCHAR)ExAllocatePool(PagedPool, BUFFER_SIZE);
	RtlZeroMemory(strContent, BUFFER_SIZE);
	WCHAR strDef = L'a';
	paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
	paramTable[0].Name = KEYNAME;
	paramTable[0].EntryContext = strContent;
	paramTable[0].DefaultType = REG_SZ;
	paramTable[0].DefaultLength = 2;
	paramTable[0].DefaultData = &strDef;

	status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
									RTL_SETPATH,
									paramTable,
									NULL,
									NULL);

	if (NT_SUCCESS(status))
	{
		KdPrint(("why::Query the REG_SZ value succuessfully, data is %wZ\n", strContent));
	}
	
	ExFreePool(strContent);

	//删除子键
	status = RtlDeleteRegistryValue(RTL_REGISTRY_ABSOLUTE,
		RTL_SETPATH,
		KEYNAME);
	if (NT_SUCCESS(status))
	{
		KdPrint(("why::delete the value successfully\n"));
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

	CreateRegedit();
	//SetRegedit();
	//QueryRegTest();
	//RTLRegedit();
	KdPrint(("why::DriverEntry end\n"));
	return status;
}