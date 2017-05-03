/*----------------------------------------------------*/
/*-- Курсовой проект по курсу "Операционные системы"--*/
/*----------------------------------------------------*/

#include <Windows.h>
#include <locale.h>
#include <stdio.h>
#include <sys/stat.h>
#include <conio.h>
#include "DataWork.h"
#include "Interface.h"

/** Глобальные переменные**/
DWORD CountOfThreads = 0;
DWORD CountOfOperations = 0;
DWORD CountOfClosedThreads = 0;
DWORD BufferSize = 4;
DWORD* WriteQueue;
BOOL WritePermission = FALSE;
BYTE* Buffer;
CRITICAL_SECTION CriticalSection;
OVERLAPPED overlapped;
HANDLE hReadFile;
HANDLE hWriteFile;

VOID FillQueue(DWORD);

DWORD wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
	setlocale(LC_ALL, "Russian");
	struct stat FileInfo;
	HANDLE hEvent;

	InitializeCriticalSection(&CriticalSection);
	CountOfThreads = GetCountOfThreads();
	
	hEvent = CreateEvent(NULL, FALSE, TRUE, NULL); //Создание события с автоматическим сбросом
	overlapped.hEvent = hEvent;
	overlapped.Offset = 0;
	overlapped.OffsetHigh = 0;
	
	//Открытие файлов
	hReadFile = CreateFile("REPLACE.txt", GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
	if (hReadFile == INVALID_HANDLE_VALUE) {
		wprintf(L"Ошибка при открытии файла. Нажмите любую клавишу для продолжения...");
		_getch();
		CloseHandle(hReadFile);
		return -1;
	}
	
	hWriteFile = CreateFile("REPLACE2.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
	if (hWriteFile == INVALID_HANDLE_VALUE) {
		wprintf(L"Ошибка при открытии файла. Нажмите любую клавишу для продолжения...");
		_getch();
		CloseHandle(hWriteFile);
		CloseHandle(hReadFile);
		return -1;
	}
	
	stat("REPLACE", &FileInfo);
	FillQueue(FileInfo.st_size);

	Buffer = malloc(BufferSize * CountOfThreads * sizeof(BYTE)); //Выделение буферов для всех нитей
	for (DWORD i = 0; i < CountOfThreads; i++) {
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AsyncReadFile, i, 0, 0);
	}

	while (CountOfClosedThreads < CountOfThreads);
	wprintf(L"Работа программы завершена");
	free(Buffer);
	free(WriteQueue);
	CloseHandle(hWriteFile);
	CloseHandle(hReadFile);
	_getch();
	return 0;
}

VOID FillQueue(DWORD FileSize) {
	//Заполнение очереди инициализации контрольными значениями
	DWORD CountOfOperations = FileSize / BufferSize;
	if (FileSize % BufferSize) {
		CountOfOperations++;
	}

	WriteQueue = malloc(sizeof(DWORD) * (CountOfOperations+1));
	for (DWORD i = 0; i < CountOfOperations; i++) {
		WriteQueue[i] = -1;
	}
	WriteQueue[CountOfOperations] = -2;
}