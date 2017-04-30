/*----------------------------------------------------*/
/*-- Курсовой проект по курсу "Операционные системы"--*/
/*----------------------------------------------------*/

#include <Windows.h>
#include <locale.h>
#include <conio.h>
#include "DataWork.h"
#include "Interface.h"

DWORD wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
	setlocale(LC_ALL, "Russian");
	GetCountOfThreads();

	_getch();
	return 0;
}