#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include "Interface.h"
#include "DataWork.h"

#define ONE_SECOND 1000

extern DWORD CountOfClosedThreads;
extern DWORD CountOfThreads;
extern DWORD ReadFileSize;
extern DWORD WriteFileSize;
extern DWORD BufferSize;
extern volatile DWORD TickPackets;
DWORD CurrentTime = 0;
DWORD PrevTime = 0;

VOID ConsoleUserInterface() {
	//����� ���������� � �������� ��������� �����
	DOUBLE StartTime = 0.00f;
	DOUBLE TempTime = 0.00f;
	DWORD Sec = 0;
	DWORD Min = 0;
	DWORD PrevPercent = 0;
	DWORD Percent = 0;
	DOUBLE Speed = 0.00f;
	DOUBLE TempSpeed = 0.00f;
	HANDLE hConsole;
	COORD OutputCoordinates = { 0, 1 };

	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	StartTime = GetTickCount();
	system("cls");
	wprintf(L"/*-- ��� ���������� �����������: --*/\n");
	while (CountOfClosedThreads < (CountOfThreads + 1)) {
		TempSpeed = CheckSpeed();

		if (TempSpeed != (-1)) {
			Speed = TempSpeed;
		}

		Percent = GetPercent();
		if (PrevPercent < Percent) {
			PrevPercent = Percent;
		}

		TempTime = GetTickCount() - StartTime;
		TempTime /= ONE_SECOND;
		Min = 0;
		Sec = TempTime;
		while (Sec >= 60) {
			Min++;
			Sec -= 60;
		}

		SetConsoleCursorPosition(hConsole, OutputCoordinates);
		wprintf(L"������� ����������: %d%%\n", Percent);
		wprintf(L"������� �������� �����������: %.2f ��/���   \n", Speed);
		wprintf(L"����� ���������� �����������: %d ���, %d ���   ", Min, Sec);
	}
	wprintf(L"\n����������� ���������. ������� ����� ������� ��� ������ �� ���������...");
	CloseHandle(hConsole);
}

DWORD GetPercent() {
	return WriteFileSize / (ReadFileSize / 100);
}

DOUBLE CheckSpeed() {
	//������� �������� � ������
	DWORD TickTime = 0;
	DOUBLE Speed = 0.00f;
	CurrentTime = GetTickCount();
	TickTime = CurrentTime - PrevTime;

	if (TickTime > ONE_SECOND) {
		PrevTime = CurrentTime;
		Speed = BufferSize * TickPackets / 1000000.00;
		TickPackets = 0;
		return Speed;
	}
	else {
		return -1;
	}
}