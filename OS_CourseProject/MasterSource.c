/*-----------------------------------------------------*/
/*-- �������� ������ �� ����� "������������ �������" --*/
/*-----------------------------------------------------*/

#include <Windows.h>
#include <locale.h>
#include <stdio.h>
#include <sys/stat.h>
#include <conio.h>
#include "DataWork.h"
#include "Interface.h"
#include "QueueDefinitions.h"

/** ���������� ����������**/
DWORD CountOfThreads = 0; //���������� �������� ����� (��� ����� �������)
DWORD CountOfOperations = 0;
DWORD CountOfClosedThreads = 0;
DWORD BufferSize = 131072;
DWORD ReadFileSize;
DWORD WriteFileSize = 0;
volatile DWORD TickPackets = 0;
DWORD* WriteQueue;
BYTE* Buffer;
CRITICAL_SECTION CriticalSection;
CRITICAL_SECTION CriticalSectionConsole;
OVERLAPPED overlapped;
HANDLE hReadFile;
HANDLE hWriteFile;

VOID FillQueue(DWORD);

DWORD wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
	setlocale(LC_ALL, "Russian");
	struct stat FileInfo;
	HANDLE hEvent;
	TCHAR ReadFileName[50];
	TCHAR WriteFileName[50];

	//���������� ���������� ����������
	InitializeCriticalSection(&CriticalSection);
	InitializeCriticalSection(&CriticalSectionConsole);
	
	hEvent = CreateEvent(NULL, FALSE, TRUE, NULL); //�������� ������� � �������������� �������
	overlapped.hEvent = hEvent;
	overlapped.Offset = 0;
	overlapped.OffsetHigh = 0;
	
	wprintf(L"������� ���������� �����: ");
	if (!scanf("%d", &CountOfThreads)){
		wprintf(L"��������� ������. ������� ����� ������� ��� ������ �� ���������...");
		_getch();
		CloseHandle(hEvent);
		return -1;
	}
	wprintf(L"������� ��� ����� ��� ������: ");
	if (!scanf("%s", ReadFileName)){
		wprintf(L"��������� ������. ������� ����� ������� ��� ������ �� ���������...");
		_getch();
		CloseHandle(hEvent);
		return -1;
	}
	wprintf(L"������� ��� ����� ��� ������: ");
	if(!scanf("%s", WriteFileName)){
		wprintf(L"��������� ������. ������� ����� ������� ��� ������ �� ���������...");
		_getch();
		CloseHandle(hEvent);
		return -1;
	}
	//�������� ������
	hReadFile = CreateFile(ReadFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (hReadFile == INVALID_HANDLE_VALUE) {
		wprintf(L"������ ��� �������� �����. ������� ����� ������� ��� �����������...");
		_getch();
		CloseHandle(hReadFile);
		CloseHandle(hEvent);
		return -1;
	}
	
	hWriteFile = CreateFile(WriteFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
	if (hWriteFile == INVALID_HANDLE_VALUE) {
		wprintf(L"������ ��� �������� �����. ������� ����� ������� ��� �����������...");
		_getch();
		CloseHandle(hWriteFile);
		CloseHandle(hReadFile);
		CloseHandle(hEvent);
		return -1;
	}	
	
	//�������� ������ �����
	stat(ReadFileName, &FileInfo);
	FillQueue(FileInfo.st_size);
	ReadFileSize = FileInfo.st_size;

	Buffer = (BYTE*)malloc(BufferSize * CountOfThreads * sizeof(BYTE)); //��������� ������� ��� ���� �����
	//�������� �����
	for (DWORD i = 0; i < CountOfThreads; i++) {
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AsyncReadFile, i, 0, 0); //�������� ���� ������
	}
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AsyncWriteFile, NULL, 0, 0); //�������� ���� ������
	ConsoleUserInterface(); //����� ���������� � �������� ����������� ����� �� �����

	//������� ������� � �������� �������� ������������
	free(Buffer);
	free(WriteQueue);
	CloseHandle(hWriteFile);
	CloseHandle(hReadFile);
	CloseHandle(hEvent);
	_getch();
	return 0;
}

VOID FillQueue(DWORD FileSize) {
	//���������� ������� ������������� ������������ ����������
	CountOfOperations = FileSize / BufferSize;
	DWORD Test;
	if (FileSize % BufferSize) {
		CountOfOperations++; //���������� ���������� �������� �� ������ ������� �� �����
	}

	//������ ���� ��������� DWORD � ���������� �������, ����� ������� ���������� �������������
	//����� ��������� � ������� ������
	WriteQueue = (DWORD*)malloc(sizeof(DWORD) * (CountOfThreads+1));
	for (DWORD i = 0; i < CountOfThreads; i++) {
		WriteQueue[i] = NO_IMPORTANT_INFORMATION;
	}
}