#include "xdv_sdk.h"
#include <windows.h>

#pragma comment(lib, "corexts.lib")

BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
)
{
	XdvLoadModule("qt\\Qt5Core");
	XdvLoadModule("qt\\Qt5Gui");
	XdvLoadModule("qt\\Qt5Widgets");
	XdvLoadModule("qt\\qwindows");
	if (XdvLoadModule("qxnm"))
	{
		return (int)ullvar(XdvExe("!qxnm.xenom"));
	}
	else
	{
		MessageBox(nullptr, L"fail..", L"", MB_OK);
	}

	return 1;
}