#include <Windows.h>
#include <stdio.h>
#include "DataWork.h"
#include "Interface.h"
#include "QueueDefinitions.h"

extern DWORD CountOfOperations;
extern DWORD CountOfThreads;
extern DWORD BufferSize;
extern DWORD CountOfClosedThreads;
extern HANDLE hReadFile;
extern HANDLE hWriteFile;
extern BYTE* Buffer;
extern DWORD* WriteQueue;
extern CRITICAL_SECTION CriticalSection;

VOID AsyncReadFile(DWORD BlockNumber) {
	//Функция асинхронного чтения из файла
	DWORD BufferNumber = BlockNumber;
	DWORD BufferRead = 0;
	HANDLE hEvent;
	OVERLAPPED overlapped;
	BYTE* previousBuffer;

	hEvent = CreateEvent(NULL, FALSE, TRUE, NULL); //Создание события с автоматическим сбросом
	overlapped.hEvent = hEvent;
	overlapped.Offset = BufferSize*BufferNumber;
	overlapped.OffsetHigh = 0;

	//Первое чтение "за циклом"
	if (BlockNumber < CountOfOperations) {
		EnterCriticalSection(&CriticalSection);
		previousBuffer = (BYTE*)malloc(BufferSize * sizeof(BYTE)); //Объявление второго "временного" буфера
		LeaveCriticalSection(&CriticalSection);

		ReadFile(hReadFile, previousBuffer, BufferSize, NULL, &overlapped);
		if (!GetOverlappedResult(hReadFile, &overlapped, &BufferRead, TRUE)) {
			CloseHandle(hEvent);
			printf("Произошла ошибка при чтении файла. Нажмите любую клавишу для продолжения...");
			CloseHandle(hReadFile);
			free(Buffer);
		}
		overlapped.Offset += (BufferSize * CountOfThreads);
		BlockNumber += CountOfThreads;

		while (BlockNumber <= CountOfOperations) {
			memcpy(&Buffer[BufferNumber*BufferSize], previousBuffer, BufferSize); //Перенос прочитанной информации в общий буфер
			if (BlockNumber != CountOfOperations) { //Если последний блок уже прочитан, функция чтения не вызывается
				ReadFile(hReadFile, previousBuffer, BufferSize, NULL, &overlapped); //Асинхронное чтение следующего блока информации
			}
			ReverseData(Buffer[BufferNumber*BufferSize], BufferRead);
			WriteQueue[BufferNumber] = READY_FOR_WRITE_IN_FILE;
			if (BlockNumber != CountOfOperations) {
				if (!GetOverlappedResult(hReadFile, &overlapped, &BufferRead, TRUE)) {
					CloseHandle(hEvent);
					printf("Произошла ошибка при чтении файла. Нажмите любую клавишу для продолжения...");
					CloseHandle(hReadFile);
					free(Buffer);
				}
			}
			while (WriteQueue[BufferNumber] != WROTEN_IN_FILE); //Ожидание того, когда содержимое буфера будет записано в файл
			overlapped.Offset += (BufferSize * CountOfThreads);
			BlockNumber += CountOfThreads;
		}
		//Обработка последнего буфера
		memcpy(&Buffer[BufferNumber*BufferSize], previousBuffer, BufferSize);
		WriteQueue[BufferNumber] = READY_FOR_WRITE_IN_FILE;
		free(previousBuffer);
	}
	//Сигнал о том, что нить отработала
	EnterCriticalSection(&CriticalSection);
	CountOfClosedThreads++;
	LeaveCriticalSection(&CriticalSection);
}

BOOL ReverseData(BYTE* data, DWORD bufferSize) {
	//Функция обработки информации
	return TRUE;
}

VOID AsyncWriteFile() {
	//Функция записи информации в файл
	DWORD ActiveBufferBlock = -1;
	DWORD TotalRead = 0;
	DWORD HasRead = 0;
	DWORD* CountWrite;
	BOOL Result = FALSE;
	HANDLE hEvent;
	OVERLAPPED overlapped;

	//Подготовка массива для сохранения значений количества записи из каждого буфера
	CountWrite = (DWORD*)malloc(CountOfThreads * sizeof(DWORD));
	for (DWORD i = 0; i < CountOfThreads; i++) {
		CountWrite[i] = 0;
	}

	//Подготовка структуры overlapped для записи в файл
	hEvent = CreateEvent(NULL, FALSE, TRUE, NULL); //Создание события с автоматическим сбросом
	overlapped.hEvent = hEvent;
	overlapped.OffsetHigh = 0;

	while (CountOfClosedThreads < (CountOfThreads) ) {
		for (DWORD i = 0; i < CountOfThreads; i++) {
			if (WriteQueue[i] == READY_FOR_WRITE_IN_FILE) {
				ActiveBufferBlock = i;
				//printf("Master %d\n", ActiveBufferBlock);
				break;
			}
		}

		if (ActiveBufferBlock != (-1)) {
			DWORD OffsetInsideBlock = ActiveBufferBlock * BufferSize;
			DWORD OffsetBeginFile = CountOfThreads * BufferSize * CountWrite[ActiveBufferBlock];
			overlapped.Offset = OffsetInsideBlock + OffsetBeginFile;
			
			Result = WriteFile(hWriteFile, Buffer + OffsetInsideBlock, BufferSize, NULL, &overlapped);
			if (!Result) {
				DWORD err = GetLastError();
			}
			
			CountWrite[ActiveBufferBlock]++;
			WriteQueue[ActiveBufferBlock] = WROTEN_IN_FILE;
			ActiveBufferBlock = -1;
		}
	}
	EnterCriticalSection(&CriticalSection);
	CountOfClosedThreads++;
	printf("Master closed\n");
	LeaveCriticalSection(&CriticalSection);
}