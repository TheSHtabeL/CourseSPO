/*----------------------------------------------------*/
/*-- Курсовой проект по курсу "Операционные системы"--*/
/*----------------------------------------------------*/

#pragma warning(disable : 4996)  

#include <Windows.h>
#include <locale.h>
#include <stdio.h>
#include <sys/stat.h>
#include <conio.h>
#include "DataWork.h"
#include "Interface.h"
#include "QueueDefinitions.h"

/** Глобальные переменные**/
DWORD CountOfThreads = 0;
DWORD CountOfOperations = 0;
DWORD CountOfClosedThreads = 0;
DWORD BufferSize = 131072;
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
	TCHAR ReadFileName[50];
	TCHAR WriteFileName[50];

	InitializeCriticalSection(&CriticalSection);
	//CountOfThreads = GetCountOfThreads();
	
	hEvent = CreateEvent(NULL, FALSE, TRUE, NULL); //Создание события с автоматическим сбросом
	overlapped.hEvent = hEvent;
	overlapped.Offset = 0;
	overlapped.OffsetHigh = 0;
	
	wprintf(L"Введите количество нитей: ");
	scanf("%d", &CountOfThreads);
	wprintf(L"Введите имя файла для чтения: ");
	scanf("%s", ReadFileName);
	wprintf(L"Введите имя файла для записи: ");
	scanf("%s", WriteFileName);
	//Открытие файлов
	hReadFile = CreateFile(ReadFileName, GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
	if (hReadFile == INVALID_HANDLE_VALUE) {
		wprintf(L"Ошибка при открытии файла. Нажмите любую клавишу для продолжения...");
		_getch();
		CloseHandle(hReadFile);
		return -1;
	}
	
	hWriteFile = CreateFile(WriteFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
	if (hWriteFile == INVALID_HANDLE_VALUE) {
		wprintf(L"Ошибка при открытии файла. Нажмите любую клавишу для продолжения...");
		_getch();
		CloseHandle(hWriteFile);
		CloseHandle(hReadFile);
		return -1;
	}	
	
	stat(ReadFileName, &FileInfo);
	FillQueue(FileInfo.st_size);

	Buffer = (BYTE*)malloc(BufferSize * CountOfThreads * sizeof(BYTE)); //Выделение буферов для всех нитей
	//Создание нитей
	for (DWORD i = 0; i < CountOfThreads; i++) {
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AsyncReadFile, i, 0, 0);
	}
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AsyncWriteFile, NULL, 0, 0);

	while (CountOfClosedThreads < (CountOfThreads+1));
	EnterCriticalSection(&CriticalSection);
	LeaveCriticalSection(&CriticalSection);
	free(Buffer);
	free(WriteQueue);
	CloseHandle(hWriteFile);
	CloseHandle(hReadFile);
	_getch();
	return 0;
}

VOID FillQueue(DWORD FileSize) {
	//Заполнение очереди инициализации контрольными значениями
	CountOfOperations = FileSize / BufferSize;
	DWORD Test;
	if (FileSize % BufferSize) {
		CountOfOperations++;
	}

	WriteQueue = (DWORD*)malloc(sizeof(DWORD) * (CountOfThreads+1));
	for (DWORD i = 0; i < CountOfThreads; i++) {
		WriteQueue[i] = NO_IMPORTANT_INFORMATION;
		Test = WriteQueue[i];
	}
}