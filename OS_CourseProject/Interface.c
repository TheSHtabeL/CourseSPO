#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include "Interface.h"
#include "DataWork.h"

VOID CheckSpeed(HANDLE sharedMemory) {
	//������� �������� � ������
}

VOID ConsoleUserInterface() {
	//����� ���������� � �������� ��������� �����
}

DWORD GetCountOfThreads() {
	DWORD count = 0;
	CHAR value;
	wprintf(L"������� ���������� �����, ������������ �������������� ����: ");

	while ( (!scanf_s("%d", &count)) != '\n' || (count <= 0) ) {
		wprintf(L"\n��� ����� ��������� ������. ���������� ��� ���.\n\n");
		_flushall();
		wprintf(L"������� ���������� �����, ������������ �������������� ����: ");
		_getch();
	}

	return 0;
}