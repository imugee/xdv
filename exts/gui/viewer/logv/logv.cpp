#include "xdv_sdk.h"

#pragma comment(lib, "corexts.lib")

xdv_handle _current_handle;
XENOM_ADD_INTERFACE()
{
	xvar var = XdvExe("!qxnm.add_viewer -name:Log -title:logv -callback:!logv.cblogv");
	_current_handle = handlevar(var);
	XdvExe("!qxnm.set_checkable -handle:%x", _current_handle);

	return _current_handle;
}

EXTS_FUNC(cblogv)	// argv[0] = status
					// argv[1] = handle
{
	return nullvar();
}