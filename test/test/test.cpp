#include <windows.h>
#include <stdio.h>
#include <conio.h>

typedef void(*FuncT)();
FuncT _func = nullptr;
LONG CALLBACK VectoredHandlerA(PEXCEPTION_POINTERS ExceptionInfo)
{
	printf("test:: call vector A=>%p, %08x\n"
		, ExceptionInfo->ExceptionRecord->ExceptionAddress
		, ExceptionInfo->ExceptionRecord->ExceptionCode);

	if (_func == ExceptionInfo->ExceptionRecord->ExceptionAddress)
	{
#ifndef _WIN64
		ExceptionInfo->ContextRecord->Eip += 1;
#else
		ExceptionInfo->ContextRecord->Rip += 1;
#endif
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

void main()
{
	AddVectoredExceptionHandler(0x0, VectoredHandlerA);
	unsigned char table[2] = { 0xcc, 0xc3};
	void * table_ptr = table;

	unsigned long old = 0;
	VirtualProtect(table, 2, PAGE_EXECUTE_READWRITE, &old);

	_func = (FuncT)table_ptr;
	int i = 0;
	while (1)
	{
		printf("test:: %d\n", i++);
		//_func();
		printf("\n");

		Sleep(1000);
	}
	_getch();
}