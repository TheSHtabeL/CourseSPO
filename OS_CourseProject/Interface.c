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
	//Вывод информации о процессе обработки файла
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
	wprintf(L"/*-- Ход выполнения копирования: --*/\n");
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
		wprintf(L"Процент выполнения: %d%%\n", Percent);
		wprintf(L"Текущая скорость копирования: %.2f Мб/сек   \n", Speed);
		wprintf(L"Время выполнения копирования: %d мин, %d сек   ", Min, Sec);
	}
	wprintf(L"\nКопирование завершено. Нажмите любую клавишу для выхода из программы...");
	CloseHandle(hConsole);
}

DWORD GetPercent() {
	return WriteFileSize / (ReadFileSize / 100);
}

DOUBLE CheckSpeed() {
	//Функция подсчёта и вывода
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