#ifndef DATAWORK_H
#define DATAWORK_H

BOOL AsyncReadFile(DWORD);
BOOL ReverseData(BYTE*, DWORD);
BOOL AsyncWriteFile(BYTE*, HANDLE);

#endif