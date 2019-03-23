#include "xcn_sdk.h"

#include <windows.h>
#include <dbghelp.h>

std::string print_memory_info(xcn::memory::type mbi);
EXTS_FUNC(segall)
{
	xcn_handle parser_handle = XcnGetParserHandle();
	std::string result_str;
	unsigned long long ptr = 0;
	do
	{
		xcn::memory::type mbi;
		if (!XcnQueryMemory(parser_handle, ptr, &mbi))
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
			IMAGEHLP_MODULEW64 module_info;
			if (XcnGetModuleName(parser_handle, mbi.AllocationBase, &module_info, sizeof(module_info)))
			{
				memset(str, 0, sizeof(str));
				sprintf_s(str, sizeof(str), "; %ls", module_info.ImageName);

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

	char * handle = XcnValue(argv, argc, "handle", nullptr);
	if (handle)
	{
		char *end = nullptr;
		xcn_handle vh = strtoul(handle, &end, 16);
		XcnPrintAndClear(vh, result_str, false);
		XcnPrintLog("xcn:: %x viewer=>print segall", vh);
	}

#if 0
	xcn_handle vh = xcn::get::viewer::handle();
	xcn::print::viewer(vh, result_str);
	xcn::print::log("xcn:: %x viewer run", vh);
#endif

	return nullvar();
}

std::string print_memory_info(xcn::memory::type mbi)
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
