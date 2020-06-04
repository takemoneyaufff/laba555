/*� �������� ������� ����� ��������� ������.��������������� ��� -
������� ������� �� ���� �������, ������� �������� �� ��������� ����� :
1) ������ �����(��������) ����������� ������� ��������� ������� -
��� ������ �����;
2) ����� - �������� ���������� ������ �����(��������) � ���, ��� ����� -
����� ����� ��������� � ����� ���� �������� ��������;
3) ����� - �������� �������� �� ������� ������ ���������� ����� � ���� -
������� ������� ���������� ���������� ������ � ����� ��������� �����;
4) ����� - �������� ���������� �������� � ���, ��� ������ �������� � �� -
������ ���� � ����� ���������� � ������ ���������� �����;
5) ��������� ����������� � �.1, ���� �� ���������� ������ ������.
� ���������� ������ ���� ����������� ������������(�����������)
������� ��������� ������ � ���� ��������������.
������� ������ - ������ ������ ���� �������� � ������������ ������ -
����, ���������� �� ����� ���������� ��������� � ��������� ����� ���� -
����� ��������� �����.*/
#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <process.h>
#include <string.h>
#define _CRT_SECURE_NO_WARNINGS
using namespace std;

HANDLE hFileToRead, hEventRead, hEventWrite, hFileToWrite, hMainEvent;
char* buffer;
DWORD nBytesToRead;
HINSTANCE hLib;

DWORD read_thread(LPVOID *) {
	DWORD nBytesRead;
	char fileName[3][20] = { "D:\\testlaba5\\1.txt\0", "D:\\testlaba5\\2.txt\0", "D:\\testlaba5\\3.txt\0" };
	hEventRead = CreateEvent(NULL, TRUE, FALSE, "event1");
	if (hEventRead == NULL) {
		printf("Error 2:%d", GetLastError());
		system("pause");
		exit(-1);
	}
	char*(*pFunction)(HANDLE, DWORD);
	(FARPROC&)pFunction = GetProcAddress(hLib, "readFile");
	for (int i = 0; i < 3; i++) {
		CloseHandle(hFileToRead);
		hFileToRead = CreateFile(fileName[i], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFileToRead == INVALID_HANDLE_VALUE) {
			printf("Error %d", GetLastError());
			system("pause");
			exit(-1);
		}
		nBytesToRead = GetFileSize(hFileToRead, NULL);
		buffer = (char*)calloc(nBytesToRead + 1, sizeof(char));
		buffer = pFunction(hFileToRead, nBytesToRead);
		SetEvent(hEventRead);
		WaitForSingleObject(hEventWrite, INFINITE);
		ResetEvent(hEventWrite);
	}
	WaitForSingleObject(hEventRead, INFINITE);
	CloseHandle(hEventRead);
	CloseHandle(hFileToRead);
	return 0;
}

DWORD write_thread(LPVOID *) {
	DWORD nBytesWrite, Offset = 0;
	if (hFileToWrite == INVALID_HANDLE_VALUE) {
		printf("Error 3:%d", GetLastError());
		system("pause");
		exit(-1);
	}
	DWORD(*pFunction)(HANDLE, DWORD, char*, DWORD);
	(FARPROC&)pFunction = GetProcAddress(hLib, "writeFile");
	hEventWrite = CreateEvent(NULL, TRUE, FALSE, "event2");
	for (int i = 0; i < 3; i++) {
		WaitForSingleObject(hEventRead, INFINITE);
		ResetEvent(hEventRead);
		Offset += pFunction(hFileToWrite, nBytesToRead, buffer, Offset);
		SetEvent(hEventWrite);
	}
	WaitForSingleObject(hEventWrite, INFINITE);
	CloseHandle(hFileToWrite);
	SetEvent(hMainEvent);
	return 0;
}

int main() {
	HANDLE rthread, wthread;
	unsigned long thread_ID;
	hFileToWrite = CreateFile("D:\\testlaba5\\result.txt", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	hMainEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	hLib = LoadLibrary(TEXT("myDLL.dll"));
	rthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)read_thread, NULL, 0, &thread_ID);
	wthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)write_thread, NULL, 0, &thread_ID);
	WaitForSingleObject(hMainEvent, INFINITE);
	CloseHandle(rthread);
	CloseHandle(wthread);
	free(buffer);
	FreeLibrary(hLib);
	system("pause");
	return 0;
}