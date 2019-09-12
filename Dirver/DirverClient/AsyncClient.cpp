//// DirverClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
////
//
//#include "pch.h"
//#include <iostream>
//#include <windows.h>
//#include <stdio.h>
//#include <winioctl.h>
//
//#define BUFFSIZE		1024
//
//int main()
//{
//	HANDLE hDevice =
//		CreateFile(L"\\\\.\\IrpAsyncDevice",
//			GENERIC_READ | GENERIC_WRITE,
//			0,		// share mode none
//			NULL,	// no security
//			OPEN_EXISTING,
//			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
//			NULL);		// no template
//
//	if (hDevice == INVALID_HANDLE_VALUE)
//	{
//		printf("Failed to obtain file handle to device: "
//			"%s with Win32 error code: %d\n",
//			"MyWDMDevice", GetLastError());
//		return 1;
//	}
//
//	UCHAR buffer[BUFFSIZE] = {0};
//	DWORD readSize = 0;
//	OVERLAPPED overLap = {0};
//	overLap.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
//	OVERLAPPED overLap1 = {0};
//	overLap1.hEvent = CreateEvent(NULL ,FALSE, FALSE, NULL);
//	BOOL ret = ReadFile(hDevice, buffer, BUFFSIZE, &readSize, &overLap);
//	if (!ret && GetLastError() == ERROR_IO_PENDING)
//	{
//		printf("readfile is async\n");
//	}
//	ret = ReadFile(hDevice, buffer, BUFFSIZE, &readSize, &overLap1);
//	if (!ret && GetLastError() == ERROR_IO_PENDING)
//	{
//		printf("readfile is async\n");
//	}
//
//	Sleep(2 * 1000);
//	CancelIo(hDevice);
//	//WaitForSingleObject(overLap.hEvent, INFINITE);
//	//WaitForSingleObject(overLap1.hEvent, INFINITE);
//	CloseHandle(hDevice); //IRP_MJ_CLEANUP
//	std::cout << "Hello World!\n";
//	system("pause");
//	return 0;
//   
//}
//
//// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
//// Debug program: F5 or Debug > Start Debugging menu
//
//// Tips for Getting Started: 
////   1. Use the Solution Explorer window to add/manage files
////   2. Use the Team Explorer window to connect to source control
////   3. Use the Output window to see build output and other messages
////   4. Use the Error List window to view errors
////   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
////   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
