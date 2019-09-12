//// DirverClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
////
//
#include "pch.h"
//#include <iostream>
//#include <windows.h>
//#include <stdio.h>
//#include <winioctl.h>
//
//#define		BUFFERREAD_IO
//#define		IO_CONTROL
//
//
//#ifdef BUFFERREAD_IO
//#define CTL_COMM_EVENT	CTL_CODE(FILE_DEVICE_UNKNOWN,\
//							 0x8000,\
//							 METHOD_BUFFERED,\
//							 FILE_ANY_ACCESS)
//#endif
//
//DWORD WaitKernelEventThd(
//	LPVOID lpThreadParameter
//	)
//{
//	HANDLE hEvent = *(HANDLE*)lpThreadParameter;
//	DWORD status = WaitForSingleObject(hEvent, 5 * 1000);
//	if (status == WAIT_OBJECT_0)
//	{
//		printf("wait for kernel event success\n");
//	}
//	else if (status == WAIT_TIMEOUT)
//	{
//		printf("wait for kernel event timeout\n");
//	}
//	return 0;
//}
//
//int main()
//{
//	HANDLE hDevice =
//		CreateFile(L"\\\\.\\CommEventDevice",
//			GENERIC_READ | GENERIC_WRITE,
//			0,		// share mode none
//			NULL,	// no security
//			OPEN_EXISTING,
//			FILE_ATTRIBUTE_NORMAL,
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
//	HANDLE hEvent = NULL;
//	HANDLE hThd = NULL;
//	DWORD  dwThdId = 0;
//	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
//	hThd = CreateThread(NULL,
//		0,
//		WaitKernelEventThd,
//		(LPVOID)&hEvent,
//		0,
//		&dwThdId);
//
//
//#ifdef IO_CONTROL
//	UCHAR InputBuffer[10];
//	UCHAR OutputBuffer[10];
//	//将输入缓冲区全部置成0XBB
//	memset(InputBuffer, 0xBB, 10);
//	DWORD dwOutput;
//	//输入缓冲区作为输入，输出缓冲区作为输出
//
//	BOOL bRet;
//	bRet = DeviceIoControl(hDevice, CTL_COMM_EVENT, &hEvent, sizeof(hEvent), &OutputBuffer, 10, &dwOutput, NULL);
//	if (bRet)
//	{
//		printf("Output buffer:%d bytes\n", dwOutput);
//		for (int i = 0; i < (int)dwOutput; i++)
//		{
//			printf("%02X ", OutputBuffer[i]);
//		}
//		printf("\n");
//	}
//	else
//	{
//		printf("Device Io control call failed, error is %d\n", GetLastError());
//	}
//#endif
//
//	WaitForSingleObject(hThd, INFINITE);
//
//	CloseHandle(hEvent);
//	CloseHandle(hThd);
//	CloseHandle(hDevice);
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
