#include "xdv_sdk.h"

#pragma comment(lib, "corexts.lib")

xdv_handle _current_handle;
XENOM_ADD_INTERFACE()
{
	xvar var = XdvExe("!qxnm.add_viewer -name:Stack -title:stack -callback:!stackv.cbstackv");
	_current_handle = handlevar(var);
	XdvExe("!qxnm.set_checkable -handle:%x", _current_handle);

	return _current_handle;
}

EXTS_FUNC(cbstackv)	// argv[0] = status
					// argv[1] = handle
{
	return nullvar();
}

EXTS_FUNC(printframe)	// argv[0] = status
						// ptr : unsigned long long
{
	unsigned long long ctx_ptr = toullarg("ctx");
	if (ctx_ptr)
	{
		xdv::architecture::x86::context::type * pctx = (xdv::architecture::x86::context::type *)ctx_ptr;
		xdv::architecture::x86::frame::type sft[1000];
		memset(sft, 0, sizeof(sft));
		unsigned long cnt = 0;
		std::string print;
		if (XdvStackTraceEx(XdvGetParserHandle(), pctx->rbp, pctx->rsp, pctx->rip, sft, sizeof(sft), &cnt))
		{
			for (unsigned long i = 0; i < cnt; ++i)
			{
				char symbol_name[1024] = { 0, };
				unsigned long name_out_size = 0;
				char print_string[1024] = { 0, };
				if (XdvGetSymbolString(XdvGetParserHandle(), sft[i].instruction_offset, symbol_name, sizeof(symbol_name)))
				{
					sprintf_s(print_string, sizeof(print_string), " %0*I64x   %0*I64x   %s\n"
						, 16, sft[i].frame_offset, 16, sft[i].instruction_offset, symbol_name);
				}
				else
				{
					sprintf_s(print_string, sizeof(print_string), " %0*I64x   %0*I64x   unknown\n"
						, 16, sft[i].frame_offset, 16, sft[i].instruction_offset);
				}

				print += print_string;
			}
			print += "\n";
			XdvPrintAndClear(_current_handle, print, false);
		}
	}
	else
	{
		xdv::architecture::x86::frame::type sft[1000];
		memset(sft, 0, sizeof(sft));
		unsigned long cnt = 0;
		std::string print;
		if (XdvStackTrace(XdvGetParserHandle(), sft, sizeof(sft), &cnt))
		{
			for (unsigned long i = 0; i < cnt; ++i)
			{
				char symbol_name[1024] = { 0, };
				unsigned long name_out_size = 0;
				char print_string[1024] = { 0, };
				if (XdvGetSymbolString(XdvGetParserHandle(), sft[i].instruction_offset, symbol_name, sizeof(symbol_name)))
				{
					sprintf_s(print_string, sizeof(print_string), " %0*I64x   %0*I64x   %s\n"
						, 16, sft[i].frame_offset, 16, sft[i].instruction_offset, symbol_name);
				}
				else
				{
					sprintf_s(print_string, sizeof(print_string), " %0*I64x   %0*I64x   unknown\n"
						, 16, sft[i].frame_offset, 16, sft[i].instruction_offset);
				}

				print += print_string;
			}
			print += "\n";
			XdvPrintAndClear(_current_handle, print, false);
		}
	}

	return nullvar();
}