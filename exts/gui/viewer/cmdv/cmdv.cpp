#include "xdv_sdk.h"
#pragma comment(lib, "corexts.lib")

#include <Windows.h>
xdv_handle _current_handle;
XENOM_ADD_INTERFACE()
{
	xvar var = XdvExe("!qxnm.addv -name:Command -title:cmd -type:cmda -callback:!cmdv.cbcmdv");
	_current_handle = handlevar(var);
	return _current_handle;
}

EXTS_FUNC(cbcmdv)	// argv[0] = status
					// argv[1] = handle
{
	char * status = XdvValue(argv, argc, "status", nullptr);
	char * handle = XdvValue(argv, argc, "handle", nullptr);
	if (strstr(status, "pre"))
	{
		char * ptr_str = XdvValue(argv, argc, "str", nullptr);
		if (ptr_str)
		{
			XdvExe("!dasmv.dasm -ptr:%s", ptr_str);
			XdvExe("!hexv.hex -ptr:%s", ptr_str);
			XdvExe("!thrdv.threads");
			XdvExe("!cpuv.printctx");
			XdvExe("!stackv.printframe");
		}
	}

	return nullvar();
}