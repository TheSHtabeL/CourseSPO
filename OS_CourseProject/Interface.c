#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include "Interface.h"
#include "DataWork.h"

extern DWORD CountOfClosedThreads;
extern DWORD CountOfThreads;

VOID CheckSpeed() {
	//������� �������� � ������
}

VOID ConsoleUserInterface() {
	//����� ���������� � �������� ��������� �����
	while (CountOfClosedThreads < (CountOfThreads + 1)) {
		system("cls");
	}
}