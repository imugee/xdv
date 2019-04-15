#include "xdv_sdk.h"
#include "qxnm.h"
#include "GotoDialog.h"
#include "XenomFindDialog.h"

EXTS_FUNC(goto_dialog)
{
	GotoDialog gd;
	gd.setModal(true);
	gd.exec();

	return ullvar(gd.getPtr());
}

EXTS_FUNC(find_dialog) // argv[0] = ptr
{
	unsigned long long ptr = toullarg("ptr");
	if (ptr)
	{
		XenomFindDialog fbbd(ptr);
		fbbd.exec();

		ptr = fbbd.FindPattern();
		if (ptr)
		{
			return ullvar(ptr);
		}
	}

	return nullvar();
}
