#include "xdv_sdk.h"
#pragma comment(lib, "corexts.lib")

xdv_handle _current_handle;
XENOM_ADD_INTERFACE()
{
	xvar var = XdvExe("!qxnm.add_viewer -name:Command -title:cmd -type:cmd -callback:!cmdv.cbcmdv");
	_current_handle = handlevar(var);
	return _current_handle;
}

EXTS_FUNC(cbcmdv)	// argv[0] = status
					// argv[1] = handle
{
	if (hasarg("status", "pre"))
	{
		char * ptr_str = argof("str");
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