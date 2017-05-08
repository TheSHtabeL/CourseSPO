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
	DWORD BufferNumber = BlockNumber; //����� ����
	DWORD BufferRead = 0;
	HANDLE hEvent;
	OVERLAPPED overlapped;
	BYTE* previousBuffer = (BYTE*)malloc(BufferSize * sizeof(BYTE));

	hEvent = CreateEvent(NULL, FALSE, TRUE, NULL); //�������� ������� � �������������� �������
	overlapped.hEvent = hEvent;
	overlapped.Offset = BufferSize*BufferNumber;
	overlapped.OffsetHigh = 0;

	if (BlockNumber < CountOfOperations) { //�������� �� ��, ��� ��� ������� ���� ���� ���������� ���������� �������
		//������ ������ "��-�� �����"
		ReadFile(hReadFile, previousBuffer, BufferSize, NULL, &overlapped);
		if (!GetOverlappedResult(hReadFile, &overlapped, &BufferRead, TRUE)) {
			//������ ������
		}
		overlapped.Offset += (BufferSize*CountOfThreads);
		BlockNumber += CountOfThreads;
		while (BlockNumber < CountOfOperations) {
			//������ ��������� ���������� � �������� �����
			EnterCriticalSection(&CriticalSection);
			memcpy(&Buffer[BufferNumber*BufferSize], previousBuffer, BufferSize);
			LeaveCriticalSection(&CriticalSection);
			//������ ���������� �����
			ReadFile(hReadFile, previousBuffer, BufferSize, NULL, &overlapped);
			//��������� ����������
			ReverseData(&Buffer[BufferNumber*BufferSize], BufferRead);
			WriteQueue[BufferNumber] = BufferRead; //����������� ������������ ���� � ���, ��� ������ � ���� ���������
			if (!GetOverlappedResult(hReadFile, &overlapped, &BufferRead, TRUE)) {
				//������ ������
			}
			while (WriteQueue[BufferNumber] != NO_IMPORTANT_INFORMATION); //���, ����� ���������� ������ ����� ���������� � ����
			overlapped.Offset += (BufferSize*CountOfThreads);
			BlockNumber += CountOfThreads;
		}
		//��������� ���������� ���������� ������
		EnterCriticalSection(&CriticalSection);
		memcpy(&Buffer[BufferNumber*BufferSize], previousBuffer, BufferSize);
		LeaveCriticalSection(&CriticalSection);
		ReverseData(&Buffer[BufferNumber*BufferSize], BufferRead);
		WriteQueue[BufferNumber] = BufferRead; //����������� ������������ ���� � ���, ��� ������ � ���� ���������
	}

	free(previousBuffer);
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
	DWORD SizeToWrite = 0;
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