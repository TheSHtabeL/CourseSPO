#include <Windows.h>
#include <stdio.h>
#include "DataWork.h"
#include "Interface.h"

VOID AsyncReadFile(DWORD BlockNumber) {
	//������� ������������ ������ �� �����
	DWORD BufferNumber = BlockNumber;
	DWORD BufferRead = 0;
	HANDLE hEvent;
	OVERLAPPED overlapped;

	extern DWORD CountOfOperations;
	extern DWORD CountOfThreads;
	extern DWORD BufferSize;
	extern DWORD CountOfClosedThreads;
	extern HANDLE hReadFile;
	extern BYTE* Buffer;
	extern DWORD* WriteQueue;
	extern CRITICAL_SECTION CriticalSection;

	hEvent = CreateEvent(NULL, FALSE, TRUE, NULL); //�������� ������� � �������������� �������
	overlapped.hEvent = hEvent;
	overlapped.Offset = BufferSize*BufferNumber;
	overlapped.OffsetHigh = 0;

	while (BlockNumber < CountOfOperations) {
		//EnterCriticalSection(&CriticalSection);
		ReadFile(hReadFile, &Buffer[BufferNumber*BufferSize], BufferSize, NULL, &overlapped);
		//���������

		if (!GetOverlappedResult(hReadFile, &overlapped, &BufferRead, TRUE)) {
			printf("��������� ������ ��� ������ �����. ������� ����� ������� ��� �����������...");
			//CloseHandle(hEvent);
			CloseHandle(hReadFile);
			free(Buffer);
		}
		overlapped.Offset += (BufferSize * CountOfThreads);
		//LeaveCriticalSection(&CriticalSection);

		printf("Thread %d\n", BufferNumber);
		WriteQueue[BufferNumber] = 1;
		while (WriteQueue[BufferNumber] != 0);
		BlockNumber += CountOfThreads;
	}
	
	EnterCriticalSection(&CriticalSection);
	CountOfClosedThreads++;
	printf("Thread %d closed\n", BufferNumber);
	LeaveCriticalSection(&CriticalSection);
}

BOOL ReverseData(BYTE* data, DWORD bufferSize) {
	//������� ��������� ����������
	return TRUE;
}

VOID AsyncWriteFile() {
	//������� ������ ���������� � ����
	extern DWORD CountOfClosedThreads;
	extern DWORD CountOfThreads;
	extern DWORD CountOfOperations;
	extern DWORD* WriteQueue;
	extern HANDLE hWriteFile;
	extern BYTE* Buffer;
	extern DWORD BufferSize;
	extern CRITICAL_SECTION CriticalSection;
	DWORD ActiveBufferBlock = -1;
	DWORD TotalRead = 0;
	DWORD HasRead = 0;
	DWORD* CountWrite;
	BOOL Result = FALSE;
	HANDLE hEvent;
	OVERLAPPED overlapped;

	//���������� ������� ��� ���������� �������� ���������� ������ �� ������� ������
	CountWrite = (DWORD*)malloc(CountOfThreads * sizeof(DWORD));
	for (DWORD i = 0; i < CountOfThreads; i++) {
		CountWrite[i] = 0;
	}

	//���������� ��������� overlapped ��� ������ � ����
	hEvent = CreateEvent(NULL, FALSE, TRUE, NULL); //�������� ������� � �������������� �������
	overlapped.hEvent = hEvent;
	overlapped.OffsetHigh = 0;

	while (CountOfClosedThreads < (CountOfThreads) ) {
		for (DWORD i = 0; i < CountOfThreads; i++) {
			if (WriteQueue[i] == 1) {
				ActiveBufferBlock = i;
				printf("Master %d\n", ActiveBufferBlock);
				break;
			}
		}

		if (ActiveBufferBlock != (-1)) {
			//overlapped.Offset = BufferSize * (ActiveBufferBlock + CountWrite[ActiveBufferBlock]);
			DWORD OffsetInsideBlock = ActiveBufferBlock * BufferSize;
			DWORD OffsetBeginFile = CountOfThreads * BufferSize * CountWrite[ActiveBufferBlock];
			overlapped.Offset = OffsetInsideBlock + OffsetBeginFile;
			Result = WriteFile(hWriteFile, Buffer + OffsetInsideBlock, BufferSize, NULL, &overlapped);
			if (!Result) {
				DWORD err = GetLastError();
			}
			CountWrite[ActiveBufferBlock]++;
			WriteQueue[ActiveBufferBlock] = 0;
			ActiveBufferBlock = -1;
		}
	}
	EnterCriticalSection(&CriticalSection);
	CountOfClosedThreads++;
	printf("Master closed\n");
	LeaveCriticalSection(&CriticalSection);
}