/*----------------------------------------------------*/
/*-- �������� ������ �� ����� "������������ �������"--*/
/*----------------------------------------------------*/

#include <Windows.h>
#include <locale.h>
#include <stdio.h>
#include <conio.h>
#include "DataWork.h"
#include "Interface.h"

/** ���������� ����������**/
DWORD CountOfThreads = 0;
DWORD BufferSize = 4;
BYTE* Buffer;
CRITICAL_SECTION CriticalSection;

DWORD wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
	setlocale(LC_ALL, "Russian");
	HANDLE hReadFile;
	HANDLE hWriteFile;

	InitializeCriticalSection(&CriticalSection);
	CountOfThreads = GetCountOfThreads();
	
	//�������� ������
	hReadFile = CreateFile("REPLACE.txt", GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
	if (hReadFile == INVALID_HANDLE_VALUE) {
		wprintf(L"������ ��� �������� �����. ������� ����� ������� ��� �����������...");
		_getch();
		CloseHandle(hReadFile);
		return -1;
	}
	
	hWriteFile = CreateFile("REPLACE2.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
	if (hWriteFile == INVALID_HANDLE_VALUE) {
		wprintf(L"������ ��� �������� �����. ������� ����� ������� ��� �����������...");
		_getch();
		CloseHandle(hWriteFile);
		CloseHandle(hReadFile);
		return -1;
	}

	Buffer = malloc(BufferSize * CountOfThreads * sizeof(BYTE)); //��������� ������� ��� ���� �����

	_getch();
	return 0;
}