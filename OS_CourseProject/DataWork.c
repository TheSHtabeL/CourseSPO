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
	DWORD BufferNumber = BlockNumber; //Номер нити
	DWORD BufferRead = 0;
	HANDLE hEvent;
	OVERLAPPED overlapped;
	BYTE* previousBuffer = (BYTE*)malloc(BufferSize * sizeof(BYTE));

	hEvent = CreateEvent(NULL, FALSE, TRUE, NULL); //Создание события с автоматическим сбросом
	overlapped.hEvent = hEvent;
	overlapped.Offset = BufferSize*BufferNumber;
	overlapped.OffsetHigh = 0;

	if (BlockNumber < CountOfOperations) { //Проверка на то, что как минимум один блок информации необходимо считать
		//Первое чтение "из-за цикла"
		ReadFile(hReadFile, previousBuffer, BufferSize, NULL, &overlapped);
		if (!GetOverlappedResult(hReadFile, &overlapped, &BufferRead, TRUE)) {
			//Ошибка чтения
		}
		overlapped.Offset += (BufferSize*CountOfThreads);
		BlockNumber += CountOfThreads;
		while (BlockNumber < CountOfOperations) {
			//Запись считанной информации в итоговый буфер
			EnterCriticalSection(&CriticalSection);
			memcpy(&Buffer[BufferNumber*BufferSize], previousBuffer, BufferSize);
			LeaveCriticalSection(&CriticalSection);
			//Чтение следующего блока
			ReadFile(hReadFile, previousBuffer, BufferSize, NULL, &overlapped);
			//Обработка информации
			ReverseData(&Buffer[BufferNumber*BufferSize], BufferRead);
			WriteQueue[BufferNumber] = BufferRead; //Уведомление записывающей нити о том, что запись в файл разрешена
			if (!GetOverlappedResult(hReadFile, &overlapped, &BufferRead, TRUE)) {
				//Ошибка чтения
			}
			while (WriteQueue[BufferNumber] != NO_IMPORTANT_INFORMATION); //Ждём, когда содержимое буфера будет перенесено в файл
			overlapped.Offset += (BufferSize*CountOfThreads);
			BlockNumber += CountOfThreads;
		}
		//Обработка последнего считанного буфера
		EnterCriticalSection(&CriticalSection);
		memcpy(&Buffer[BufferNumber*BufferSize], previousBuffer, BufferSize);
		LeaveCriticalSection(&CriticalSection);
		ReverseData(&Buffer[BufferNumber*BufferSize], BufferRead);
		WriteQueue[BufferNumber] = BufferRead; //Уведомление записывающей нити о том, что запись в файл разрешена
	}

	free(previousBuffer);
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
	DWORD SizeToWrite = 0;
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
			if (WriteQueue[i] != NO_IMPORTANT_INFORMATION) {
				ActiveBufferBlock = i;
				SizeToWrite = WriteQueue[i];
				break;
			}
		}

		if (ActiveBufferBlock != (-1)) {
			DWORD OffsetInsideBlock = ActiveBufferBlock * BufferSize;
			DWORD OffsetBeginFile = CountOfThreads * BufferSize * CountWrite[ActiveBufferBlock];
			overlapped.Offset = OffsetInsideBlock + OffsetBeginFile;
			
			Result = WriteFile(hWriteFile, Buffer + OffsetInsideBlock, SizeToWrite, NULL, &overlapped);
			if (!Result) {
				DWORD err = GetLastError();
			}
			
			CountWrite[ActiveBufferBlock]++;
			WriteQueue[ActiveBufferBlock] = NO_IMPORTANT_INFORMATION;
			ActiveBufferBlock = -1;
		}
	}
	EnterCriticalSection(&CriticalSection);
	CountOfClosedThreads++;
	printf("Master closed\n");
	LeaveCriticalSection(&CriticalSection);
}