#include <Windows.h>
#include "DataWork.h"
#include "Interface.h"

VOID AsyncReadFile(DWORD BlockNumber) {
	//������� ������������ ������ �� �����
	DWORD BufferNumber = BlockNumber;
	extern CountOfOperations;
	extern CountOfThreads;
	extern HANDLE hReadFile;

	while (BlockNumber < CountOfOperations) {
			

		BlockNumber += CountOfThreads;
	}
}

BOOL ReverseData(BYTE* data, DWORD bufferSize) {
	//������� ��������� ����������
	return TRUE;
}

VOID AsyncWriteFile(BYTE* data, HANDLE sharedMemory) {
	//������� ������ ���������� � ����
	return TRUE;
}