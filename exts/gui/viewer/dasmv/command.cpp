#include "xdv_sdk.h"

unsigned long long GetCurrentPtr();
void SetCurrentPtr(unsigned long long ptr);
xdv_handle GetCurrentHandle();

unsigned long long CodeAndRemarkString(unsigned long long ptr, std::string &str);
EXTS_FUNC(codesize)	// argv[0] = ptr
{
	unsigned long long ptr = toullarg("ptr");
	if (ptr)
	{
		std::string str;
		unsigned long long size = CodeAndRemarkString(ptr, str);

		return ullvar(size);
	}

	return ullvar(0);
}

void NavigationString(unsigned long long ptr, std::string &str);
EXTS_FUNC(navistr)	// argv ptr
					// argv buffer
{
	unsigned long long ptr = toullarg("ptr");
	if (ptr == 0)
	{
		return nullvar();
	}

	std::string * buf = (std::string *)toptrarg("buf");
	if (!buf)
	{
		return nullvar();
	}

	if (ptr)
	{
		std::string str;
		NavigationString(ptr, str);

		*buf += str;
	}

	return nullvar();
}

EXTS_FUNC(codestr)	// argv[0] = ptr
{
	unsigned long long ptr = toullarg("ptr");
	if (ptr == 0)
	{
		return nullvar();
	}

	std::string * buf = (std::string *)toptrarg("buf");
	if (!buf)
	{
		return nullvar();
	}

	if (ptr)
	{
		std::string str;
		CodeAndRemarkString(ptr, str);

		*buf += str;
	}

	return nullvar();
}

EXTS_FUNC(dasmptr)	// argv[0] = status
{
	return ullvar(GetCurrentPtr());
}

unsigned long long Disassemble(unsigned long long ptr, std::string &str);
EXTS_FUNC(dasm)	// argv[0] = ptr
{
	xdv_handle current_handle = GetCurrentHandle();
	unsigned long long current_ptr = GetCurrentPtr();
	unsigned long long ptr = toullarg("ptr");
	if (ptr == 0)
	{
		ptr = current_ptr;
	}
	else
	{
		SetCurrentPtr(ptr);
	}

	std::string str;
	unsigned char dump[16] = { 0, };
	unsigned long long read = XdvReadMemory(XdvGetParserHandle(), ptr, dump, sizeof(dump));
	if (read == 0)
	{
		return nullvar();
	}

	XdvExe("!procv.analyze -ptr:%I64x", ptr);
	for (int i = 0; i < 80; ++i)
	{
		unsigned long long size = Disassemble(ptr, str);
		if (size == 0)
		{
			++ptr;
			continue;
		}

		ptr += size;
		if (i + 1 != 80)
		{
			str += "\n";
		}
	}

	XdvPrintAndClear(current_handle, str.c_str(), false);
	return nullvar();
}
