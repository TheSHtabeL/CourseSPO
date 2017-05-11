#include <Windows.h>
#include <stdio.h>
#include "DataWork.h"
#include "Interface.h"
#include "QueueDefinitions.h"

extern volatile DWORD TickPackets;
extern DWORD CountOfOperations;
extern DWORD CountOfThreads;
extern DWORD BufferSize;
extern DWORD CountOfClosedThreads;
extern DWORD WriteFileSize;
extern HANDLE hReadFile;
extern HANDLE hWriteFile;
extern BYTE* Buffer;
extern DWORD* WriteQueue;
extern CRITICAL_SECTION CriticalSection;
//extern CRITICAL_SECTION CriticalSectionConsole;


VOID AsyncReadFile(DWORD BlockNumber) {
	//Функция асинхронного чтения из файла
	DWORD BufferNumber = BlockNumber; //Номер нити
	DWORD BufferRead = 0;
	HANDLE hEvent;
	OVERLAPPED overlapped;
	BYTE* previousBuffer = (BYTE*)malloc(BufferSize * sizeof(BYTE));
	BYTE* reverseBuffer = (BYTE*)malloc(BufferSize * sizeof(BYTE));
	//Переменные, используемые только в случае ошибки
	HANDLE hConsole;
	COORD OutputCoordinates = { 0, 5 }; //x = 0, y = 5 - Координаты шестой строки в консоли

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
		memcpy(reverseBuffer, previousBuffer, BufferSize);
		overlapped.Offset += (BufferSize*CountOfThreads);
		BlockNumber += CountOfThreads;
		while (BlockNumber < CountOfOperations) {
			//Чтение следующего блока
			ReadFile(hReadFile, previousBuffer, BufferSize, NULL, &overlapped);

			//Обработка информации
			ReverseData(reverseBuffer, BufferSize);
			memcpy(&Buffer[BufferNumber*BufferSize], reverseBuffer, BufferSize); //Передаём информацию в буфер для записи
			WriteQueue[BufferNumber] = BufferRead; //Уведомление записывающей нити о том, что запись в файл разрешена

			if (!GetOverlappedResult(hReadFile, &overlapped, &BufferRead, TRUE)) {
				//Ошибка чтения
				CountOfClosedThreads++;
				free(Buffer);
				free(previousBuffer);
				CloseHandle(hEvent);
				return;
			}

			//Запись считанной информации во временный буфер
			memcpy(reverseBuffer, previousBuffer, BufferSize);
			while (WriteQueue[BufferNumber] != NO_IMPORTANT_INFORMATION); //Ждём, когда содержимое буфера будет перенесено в файл
			overlapped.Offset += (BufferSize*CountOfThreads);
			BlockNumber += CountOfThreads;
		}
		//Обработка последнего считанного буфера
		ReverseData(reverseBuffer, BufferRead);
		memcpy(&Buffer[BufferNumber*BufferSize], reverseBuffer, BufferSize);
		WriteQueue[BufferNumber] = BufferRead; //Уведомление записывающей нити о том, что запись в файл разрешена
	}

	free(previousBuffer);
	free(reverseBuffer);

	//Сигнал о том, что нить отработала
	EnterCriticalSection(&CriticalSection);
	CountOfClosedThreads++;
	LeaveCriticalSection(&CriticalSection);
}

VOID ReverseData(BYTE* data, DWORD bufferSize) {
	//Функция обработки информации
	BYTE temp;
	DWORD count = bufferSize - 1; //Символ без пары не будет обрабатываться

	for (DWORD i = 0; i < count; i+=2) { //Меняем местами нечётные и чётные байты
		temp = data[i];
		data[i] = data[i + 1];
		data[i + 1] = temp;
	}

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
			if (WriteQueue[i] != NO_IMPORTANT_INFORMATION) { //Ищем буфер, в который читающая нить записал блок информации
				ActiveBufferBlock = i;
				SizeToWrite = WriteQueue[i];
				break;
			}
		}

		if (ActiveBufferBlock != (-1)) { //Проверяем, найдена ли информация в буфере
			DWORD OffsetInsideBlock = ActiveBufferBlock * BufferSize;
			DWORD OffsetBeginFile = CountOfThreads * BufferSize * CountWrite[ActiveBufferBlock];
			overlapped.Offset = OffsetInsideBlock + OffsetBeginFile; //Смещение на место, в который нужно вписать информацию
			
			WriteFile(hWriteFile, Buffer + OffsetInsideBlock, SizeToWrite, NULL, &overlapped);			
			WriteFileSize += SizeToWrite;
			TickPackets++; //Количество записанных буферов (периодически переменная очищается)
			CountWrite[ActiveBufferBlock]++; //Индивидуальные счётчики читающих нитей
			WriteQueue[ActiveBufferBlock] = NO_IMPORTANT_INFORMATION;
			ActiveBufferBlock = -1;
		}
	}

	EnterCriticalSection(&CriticalSection);
	CountOfClosedThreads++;
	LeaveCriticalSection(&CriticalSection);
}