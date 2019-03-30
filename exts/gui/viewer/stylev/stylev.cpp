#include "xdv_sdk.h"

#pragma comment(lib, "corexts.lib")

XENOM_ADD_INTERFACE()
{
	return 0;
}

EXTS_FUNC(style)	// argv[0] = status
{
	xdv_handle cmdv_handle = XdvGetHandleByString("Command");
	xdv_handle dasmv_handle = XdvGetHandleByString("Disassemble");
	xdv_handle logv_handle = XdvGetHandleByString("Log");
	xdv_handle procv_handle = XdvGetHandleByString("Procedure");
	xdv_handle hex_handle = XdvGetHandleByString("Hex");
	xdv_handle reg_handle = XdvGetHandleByString("Registers");
	xdv_handle stack_handle = XdvGetHandleByString("Stack");
	xdv_handle thrd_handle = XdvGetHandleByString("Threads");
	xdv_handle segment_handle = XdvGetHandleByString("Segment");

	XdvExe("!qxnm.add_split -va:%x -area:left -vb:%x ", dasmv_handle, procv_handle);
	XdvExe("!qxnm.add_split -va:%x -area:right -vb:%x ", dasmv_handle, reg_handle);

	XdvExe("!qxnm.add_split -va:%x -area:bottom -vb:%x ", reg_handle, stack_handle);
	XdvExe("!qxnm.add_split -va:%x -area:bottom -vb:%x ", stack_handle, thrd_handle);

	XdvExe("!qxnm.add_split -va:%x -area:bottom -vb:%x ", dasmv_handle, hex_handle);
	XdvExe("!qxnm.add_split -va:%x -area:bottom -vb:%x ", dasmv_handle, hex_handle);

	XdvExe("!qxnm.add_tab -va:%x -area:bottom -vb:%x ", hex_handle, cmdv_handle);
	XdvExe("!qxnm.add_tab -va:%x -area:bottom -vb:%x ", cmdv_handle, segment_handle);
	XdvExe("!qxnm.add_tab -va:%x -area:bottom -vb:%x ", segment_handle, logv_handle);

	//XdvExe("!qxnm.add_tab -va:%x -area:left -vb:%x", cmdv_handle, logv_handle);

	return nullvar();
}