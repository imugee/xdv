#include "xdv_sdk.h"

#pragma comment(lib, "corexts.lib")

unsigned long long _current_ptr = 0;
xdv_handle _current_handle;
XENOM_ADD_INTERFACE()
{
	xvar var = XdvExe("!qxnm.add_viewer -name:Hex -title:hex -type:event -callback:!hexv.cbhexv");
	_current_handle = handlevar(var);
	XdvExe("!qxnm.set_checkable -handle:%x", _current_handle);

	return _current_handle;
}

std::string PrintByte(unsigned long long ptr, int line);
EXTS_FUNC(hex)	// argv[0] = ptr
{
	unsigned long long ptr = toullarg("ptr");
	if (ptr == 0)
	{
		ptr = _current_ptr;
	}
	else
	{
		_current_ptr = ptr;
	}

	XdvPrintAndClear(_current_handle, PrintByte(ptr, 100), false);
	return nullvar();
}

EXTS_FUNC(cbhexv)	// argv[0] = status
					// argv[1] = string
{
	char * status =argof("status");
	if (hasarg("status", "up"))
	{
		_current_ptr -= 16;
		XdvPrintAndClear(_current_handle, PrintByte(_current_ptr, 100), false);
	}
	else if (hasarg("status", "down"))
	{
		_current_ptr += 16;
		XdvPrintAndClear(_current_handle, PrintByte(_current_ptr, 100), false);
	}
	else if (hasarg("status", "Find Pattern"))
	{
		unsigned long long ptr = toullarg("tag");
		xvar var = XdvExe("!qxnm.find_dialog -ptr:%I64x", ptr);
		ptr = ullvar(var);

		unsigned char dump[16] = { 0, };
		if (XdvReadMemory(XdvGetParserHandle(), ptr, dump, sizeof(dump)))
		{
			XdvExe("!hexv.hex -ptr:%I64x", ptr);
		}
	}
	else if (hasarg("status", "Jump"))
	{
		xvar var = XdvExe("!qxnm.goto_dialog");
		unsigned long long ptr = ullvar(var);
		if (ptr)
		{
			XdvExe("!hexv.hex -ptr:%I64x", ptr);
		}
	}

	return nullvar();
}

EXTS_FUNC(update) // first callback
{
	XdvExe("!qxnm.add_command -handle:%x -name:Jump -key:%x", _current_handle, xdv::key::Key_CTRL | xdv::key::id::Key_G);
	XdvExe("!qxnm.add_command -handle:%x -menu:Find -name:Find Pattern -key:%x", _current_handle, xdv::key::Key_CTRL | xdv::key::id::Key_F);

	return nullvar();
}

bool is_alphabet(char c)
{
	if (c >= 0x41 && c <= 0x7e)
		return true;

	return false;
}

std::string PrintByte(unsigned long long ptr, int line)
{
	_current_ptr = ptr;

	xdv_handle ih = XdvGetParserHandle();
	std::string print;
	for (int i = 0; i < line; ++i)
	{
		unsigned char dump[16] = { 0, };
		unsigned long long readn = XdvReadMemory(ih, ptr, dump, sizeof(dump));
		if (readn == 0)
		{
			break;
		}

		char ptr_str[32] = { 0, };
		sprintf_s(ptr_str, sizeof(ptr_str), " %0*I64x   ", 16, ptr);
		print += ptr_str;

		for (int j = 0; j < readn; ++j)
		{
			char string[10] = { 0, };
			sprintf_s(string, sizeof(string), "%02x ", dump[j]);

			print += string;
		}
		print += "       ";

		for (int j = 0; j < readn; ++j)
		{
			char string[10] = { 0, };
			if (!is_alphabet(dump[j]))
			{
				sprintf_s(string, sizeof(string), ".");
			}
			else
			{
				sprintf_s(string, sizeof(string), "%c", dump[j]);
			}

			print += string;
		}
		print += "\n";
		ptr += 16;
	}

	return print;
}
