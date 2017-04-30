#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include "Interface.h"
#include "DataWork.h"

VOID CheckSpeed(HANDLE sharedMemory) {
	//Функция подсчёта и вывода
}

VOID ConsoleUserInterface() {
	//Вывод информации о процессе обработки файла
}

DWORD GetCountOfThreads() {
	DWORD count = 0;
	CHAR value;
	wprintf(L"Введите количество нитей, одновременно обрабатывающих файл: ");

	while ( (!scanf_s("%d", &count)) != '\n' || (count <= 0) ) {
		wprintf(L"\nПри вводе произошла ошибка. Попробуйте ещё раз.\n\n");
		_flushall();
		wprintf(L"Введите количество нитей, одновременно обрабатывающих файл: ");
		_getch();
	}

	return 0;
}