// DirverClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <winioctl.h>

#define CTL_START_TIMER 	CTL_CODE(FILE_DEVICE_UNKNOWN,\
							 0x8000,\
							 METHOD_BUFFERED,\
							 FILE_ANY_ACCESS)


#define CTL_STOP_TIMER 	CTL_CODE(FILE_DEVICE_UNKNOWN,\
							 0x8002,\
							 METHOD_BUFFERED,\
							 FILE_ANY_ACCESS)

int main()
{
	HANDLE hDevice =
		CreateFile(L"\\\\.\\DPCTimerDevice",
			GENERIC_READ | GENERIC_WRITE,
			0,		// share mode none
			NULL,	// no security
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);		// no template

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		printf("Failed to obtain file handle to device: "
			"%s with Win32 error code: %d\n",
			"MyWDMDevice", GetLastError());
		return 1;
	}

	UCHAR InputBuffer[10];
	UCHAR OutputBuffer[10];
	//将输入缓冲区全部置成0XBB
	memset(InputBuffer, 0xBB, 10);
	DWORD dwOutput;
	//输入缓冲区作为输入，输出缓冲区作为输出
	DWORD dwTimerInter = 2;
	BOOL bRet;
	bRet = DeviceIoControl(hDevice, CTL_START_TIMER, &dwTimerInter, sizeof(DWORD), NULL, NULL, &dwOutput, NULL);
	Sleep(10 * 1000);
	bRet = DeviceIoControl(hDevice, CTL_STOP_TIMER, NULL, NULL, NULL, NULL, &dwOutput, NULL);
	CloseHandle(hDevice);
	std::cout << "Hello World!\n";
	system("pause");
	return 0;

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
