/*
* eXtension Disassembly viewer
*/

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include "resource.h"

#include "xdv_sdk.h"
#pragma comment(lib, "corexts.lib")

#if 0
int main(int argc, char * argv[])
{
	if (argc < 2)
	{
		XdvLoadModule("qt\\Qt5Core");
		XdvLoadModule("qt\\Qt5Gui");
		XdvLoadModule("qt\\Qt5Widgets");
		XdvLoadModule("qt\\qwindows");
		if (XdvLoadModule("qxnm"))
		{
			ShowWindow(GetConsoleWindow(), SW_HIDE);
			return (int)ullvar(XdvExe("!qxnm.xenom"));
		}
		else
		{
			printf("> not found qxnm\n"), _getch();
		}
	}
	else
	{
		if (strstr(argv[1], "-hide"))
		{
			return (int)ullvar(XdvExe(".xenom"));
		}
	}

	return 0;
}
#else
int __stdcall WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nShowCmd
)
{
	XdvLoadModule("qt\\Qt5Core");
	XdvLoadModule("qt\\Qt5Gui");
	XdvLoadModule("qt\\Qt5Widgets");
	XdvLoadModule("qt\\qwindows");
	if (XdvLoadModule("qxnm"))
	{
		ShowWindow(GetConsoleWindow(), SW_HIDE);
		return (int)ullvar(XdvExe("!qxnm.xenom"));
	}
	else
	{
		printf("> not found qxnm\n"), _getch();
	}
}
#endif