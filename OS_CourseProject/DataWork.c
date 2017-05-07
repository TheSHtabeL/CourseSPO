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
	//������� ������������ ������ �� �����
	DWORD BufferNumber = BlockNumber;
	DWORD BufferRead = 0;
	HANDLE hEvent;
	OVERLAPPED overlapped;
	BYTE* previousBuffer;

	hEvent = CreateEvent(NULL, FALSE, TRUE, NULL); //�������� ������� � �������������� �������
	overlapped.hEvent = hEvent;
	overlapped.Offset = BufferSize*BufferNumber;
	overlapped.OffsetHigh = 0;

	//������ ������ "�� ������"
	if (BlockNumber < CountOfOperations) {
		EnterCriticalSection(&CriticalSection);
		previousBuffer = (BYTE*)malloc(BufferSize * sizeof(BYTE)); //���������� ������� "����������" ������
		LeaveCriticalSection(&CriticalSection);

		ReadFile(hReadFile, previousBuffer, BufferSize, NULL, &overlapped);
		if (!GetOverlappedResult(hReadFile, &overlapped, &BufferRead, TRUE)) {
			CloseHandle(hEvent);
			printf("��������� ������ ��� ������ �����. ������� ����� ������� ��� �����������...");
			CloseHandle(hReadFile);
			free(Buffer);
		}
		overlapped.Offset += (BufferSize * CountOfThreads);
		BlockNumber += CountOfThreads;

		while (BlockNumber <= CountOfOperations) {
			memcpy(&Buffer[BufferNumber*BufferSize], previousBuffer, BufferSize); //������� ����������� ���������� � ����� �����
			if (BlockNumber != CountOfOperations) { //���� ��������� ���� ��� ��������, ������� ������ �� ����������
				ReadFile(hReadFile, previousBuffer, BufferSize, NULL, &overlapped); //����������� ������ ���������� ����� ����������
			}
			ReverseData(Buffer[BufferNumber*BufferSize], BufferRead);
			WriteQueue[BufferNumber] = READY_FOR_WRITE_IN_FILE;
			if (BlockNumber != CountOfOperations) {
				if (!GetOverlappedResult(hReadFile, &overlapped, &BufferRead, TRUE)) {
					CloseHandle(hEvent);
					printf("��������� ������ ��� ������ �����. ������� ����� ������� ��� �����������...");
					CloseHandle(hReadFile);
					free(Buffer);
				}
			}
			while (WriteQueue[BufferNumber] != WROTEN_IN_FILE); //�������� ����, ����� ���������� ������ ����� �������� � ����
			overlapped.Offset += (BufferSize * CountOfThreads);
			BlockNumber += CountOfThreads;
		}
		//��������� ���������� ������
		memcpy(&Buffer[BufferNumber*BufferSize], previousBuffer, BufferSize);
		WriteQueue[BufferNumber] = READY_FOR_WRITE_IN_FILE;
		free(previousBuffer);
	}
	//������ � ���, ��� ���� ����������
	EnterCriticalSection(&CriticalSection);
	CountOfClosedThreads++;
	LeaveCriticalSection(&CriticalSection);
}

BOOL ReverseData(BYTE* data, DWORD bufferSize) {
	//������� ��������� ����������
	return TRUE;
}

VOID AsyncWriteFile() {
	//������� ������ ���������� � ����
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