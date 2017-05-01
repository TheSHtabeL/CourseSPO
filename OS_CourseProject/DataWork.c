#include <Windows.h>
#include "DataWork.h"
#include "Interface.h"

VOID AsyncReadFile(DWORD BlockNumber) {
	//Функция асинхронного чтения из файла
	DWORD BufferNumber = BlockNumber;
	extern CountOfOperations;
	extern CountOfThreads;
	extern HANDLE hReadFile;

	while (BlockNumber < CountOfOperations) {
			

		BlockNumber += CountOfThreads;
	}
}

BOOL ReverseData(BYTE* data, DWORD bufferSize) {
	//Функция обработки информации
	return TRUE;
}

VOID AsyncWriteFile(BYTE* data, HANDLE sharedMemory) {
	//Функция записи информации в файл
	return TRUE;
}