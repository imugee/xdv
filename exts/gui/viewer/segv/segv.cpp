#include "xdv_sdk.h"

#pragma comment(lib, "corexts.lib")

xdv_handle _current_handle;
XENOM_ADD_INTERFACE()
{
	xvar var = XdvExe("!qxnm.add_viewer -name:Segment -title:segment -callback:!segv.cbdasmv");
	_current_handle = handlevar(var);
	XdvExe("!qxnm.set_checkable -handle:%x", _current_handle);

	return _current_handle;
}

EXTS_FUNC(cbdasmv)	// argv = status
					// argv = string
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

EXTS_FUNC(update)
{
	XdvExe("!qxnm.express_color -handle:%x -expression:;[^\\n]* -color:#009327", _current_handle);
	return nullvar();
}

//
#include <windows.h>

std::string print_memory_info(xdv::memory::type mbi);
EXTS_FUNC(segall)
{
	xdv_handle parser_handle = XdvGetParserHandle();
	std::string result_str;
	unsigned long long ptr = 0;
	do
	{
		xdv::memory::type mbi;
		if (!XdvQueryMemory(parser_handle, ptr, &mbi))
		{
			break;
		}

		if (mbi.State == MEM_COMMIT)
		{
			char str[500] = { 0, };
			sprintf_s(str, sizeof(str), " %0*I64x	%0*I64x	", 16, mbi.BaseAddress, 16, mbi.RegionSize);

			result_str += str;
			result_str += print_memory_info(mbi);

			unsigned long long disp = 0;
			std::string mn = XdvGetModuleName(parser_handle, mbi.AllocationBase);
			if (mn.size())
			{
				memset(str, 0, sizeof(str));
				sprintf_s(str, sizeof(str), "; %s", mn.c_str());

				result_str += str;
				result_str += "\n";
			}
			else
			{
				result_str += "\n";
			}
		}

		ptr = mbi.BaseAddress + mbi.RegionSize;
	} while (1);

	XdvPrintAndClear(_current_handle, result_str, false);

	return nullvar();
}

std::string print_memory_info(xdv::memory::type mbi)
{
	std::string state_str;
	switch (mbi.State)
	{
	case MEM_COMMIT:
		state_str = "MEM_COMMIT";
		break;

	case MEM_RESERVE:
		state_str = "MEM_RESERVE";
		break;

	case MEM_DECOMMIT:
		state_str = "MEM_DECOMMIT";
		break;

	case MEM_RELEASE:
		state_str = "MEM_RELEASE";
		break;

	case MEM_FREE:
		state_str = "MEM_FREE";
		break;

	default:
		state_str = "UNKNOWN";
		break;
	}

	std::string type_str;
	switch (mbi.Type)
	{
	case MEM_PRIVATE:
		type_str = "MEM_PRIVATE";
		break;

	case MEM_MAPPED:
		type_str = "MEM_MAPPED";
		break;

	case MEM_IMAGE:
		type_str = "MEM_IMAGE";
		break;

	default:
		type_str = "UNKNOWN";
		break;
	}

	std::string protect_str;
	switch (mbi.Protect)
	{
	case PAGE_EXECUTE:
		protect_str = "PAGE_EXECUTE";
		break;

	case PAGE_EXECUTE_READ:
		protect_str = "PAGE_EXECUTE_READ";
		break;

	case PAGE_EXECUTE_READWRITE:
		protect_str = "PAGE_EXECUTE_READWRITE";
		break;

	case PAGE_EXECUTE_WRITECOPY:
		protect_str = "PAGE_EXECUTE_WRITECOPY";
		break;

	case PAGE_READONLY:
		protect_str = "PAGE_READONLY";
		break;

	case PAGE_READWRITE:
		protect_str = "PAGE_READWRITE";
		break;

	case PAGE_WRITECOPY:
		protect_str = "PAGE_WRITECOPY";
		break;

	default:
		protect_str = "UNKNOWN";
		break;
	}

	//
	std::string result_str;
	char str[500] = { 0, };
	size_t str_size = state_str.size();
	size_t align = 15;
	sprintf_s(str, sizeof(str), "%s%*c", state_str.c_str(), (int)(align - str_size), ' ');
	result_str += str;

	memset(str, 0, sizeof(str));
	str_size = protect_str.size();
	align = 25;
	sprintf_s(str, sizeof(str), "%s%*c", protect_str.c_str(), (int)(align - str_size), ' ');
	result_str += str;

	memset(str, 0, sizeof(str));
	str_size = type_str.size();
	align = 15;
	sprintf_s(str, sizeof(str), "%s%*c", type_str.c_str(), (int)(align - str_size), ' ');
	result_str += str;

	return result_str;
}
