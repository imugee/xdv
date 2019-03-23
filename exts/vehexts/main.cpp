#include <windows.h>
#include <winternl.h>
#include <stdio.h>
#include <conio.h>
#include <mutex>

#include "xdv_sdk.h"
#pragma comment(lib, "corexts_static.lib")
#ifdef _WIN64
#pragma comment(lib, "xdvlib_x64.lib")
#else
#pragma comment(lib, "xdvlib.lib")
#endif

//
LONG CALLBACK VectoredHandler(PEXCEPTION_POINTERS ExceptionInfo);
void * GetLdrpVectorHandlerList();
PVOID RtlpAddVectoredExceptionHandler(ULONG FirstHandler, PVECTORED_EXCEPTION_HANDLER VectoredHandler, void * LdrpVectorHandlerList);
BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		void * entry = RtlpAddVectoredExceptionHandler(1, VectoredHandler, GetLdrpVectorHandlerList());
		if (!entry)
		{
			printf("xdv:: veh f\n");
		}
	}

	return 1;
}

