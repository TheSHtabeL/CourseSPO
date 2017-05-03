#include <Windows.h>
#include <stdio.h>
#include "DataWork.h"
#include "Interface.h"

VOID AsyncReadFile(DWORD BlockNumber) {
	//Функция асинхронного чтения из файла
	DWORD BufferNumber = BlockNumber;
	DWORD BufferRead = 0;
	extern DWORD CountOfOperations;
	extern DWORD CountOfThreads;
	extern DWORD BufferSize;
	extern DWORD CountOfClosedThreads;
	extern HANDLE hReadFile;
	extern BYTE* Buffer;
	extern OVERLAPPED overlapped;
	extern CRITICAL_SECTION CriticalSection;

	while (BlockNumber < CountOfOperations) {
		ReadFile(hReadFile, &Buffer[BufferNumber], BufferSize, NULL, &overlapped);
		//Обработка

		if (!GetOverlappedResult(hReadFile, &overlapped, &BufferRead, TRUE)) {
			printf("Произошла ошибка при чтении файла. Нажмите любую клавишу для продолжения...");
			//CloseHandle(hEvent);
			CloseHandle(hReadFile);
			free(Buffer);
		}
		BlockNumber += CountOfThreads;
	}
	
	EnterCriticalSection(&CriticalSection);
	CountOfClosedThreads++;
	LeaveCriticalSection(&CriticalSection);
}

BOOL ReverseData(BYTE* data, DWORD bufferSize) {
	//Функция обработки информации
	return TRUE;
}

VOID AsyncWriteFile(BYTE* data, HANDLE sharedMemory) {
	//Функция записи информации в файл
}