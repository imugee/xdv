#include "xdv_sdk.h"

#pragma comment(lib, "corexts.lib")

xdv_handle _current_handle;
XENOM_ADD_INTERFACE()
{
	xvar var = XdvExe("!qxnm.add_viewer -name:Registers -title:cpu -callback:!cpuv.cbcpuv");
	_current_handle = handlevar(var);
	XdvExe("!qxnm.set_checkable -handle:%x", _current_handle);

	return _current_handle;
}

EXTS_FUNC(cbcpuv)
{
	return nullvar();
}

std::string regtostr(std::string name, unsigned long long val);
xdv::architecture::x86::context::type _thread_ctx;
EXTS_FUNC(getctx)
{
	return ptrvar(&_thread_ctx);
}

EXTS_FUNC(printctx)	// argv[0] = tid, unsigned long type
					// argv[0] = ctx, xdv::architecture::x86::context::type * type
{
	unsigned long long ctx_ptr = toullarg("ctx");
	if (ctx_ptr)
	{
		xdv::architecture::x86::context::type * pctx = (xdv::architecture::x86::context::type *)ctx_ptr;
		_thread_ctx = *pctx;
	}
	else if (argof("tid"))
	{
		unsigned long tid = (unsigned long)toullarg("tid");
		if (tid)
		{
			XdvSelectThread(XdvGetParserHandle(), tid);
			if (!XdvGetThreadContext(XdvGetParserHandle(), &_thread_ctx))
			{
				return nullvar();
			}
		}
	}
	else
	{
		if (!XdvGetThreadContext(XdvGetParserHandle(), &_thread_ctx))
		{
			return nullvar();
		}
	}

	std::string regs;
	switch (XdvGetObjectByHandle(XdvGetArchitectureHandle())->ObjectType())
	{
	case xdv::object::id::XENOM_X86_ARCHITECTURE_OBJECT:
	case xdv::object::id::XENOM_X86_ANALYZER_OBJECT:
		regs += regtostr(" EAX", _thread_ctx.rax) + "\n";
		regs += regtostr(" EBX", _thread_ctx.rbx) + "\n";
		regs += regtostr(" ECX", _thread_ctx.rcx) + "\n";
		regs += regtostr(" EDX", _thread_ctx.rdx) + "\n\n";

		regs += regtostr(" ESP", _thread_ctx.rsp) + "\n";
		regs += regtostr(" EBP", _thread_ctx.rbp) + "\n\n";

		regs += regtostr(" EDI", _thread_ctx.rdi) + "\n";
		regs += regtostr(" ESI", _thread_ctx.rsi) + "\n\n";

		regs += regtostr(" EIP", _thread_ctx.rip) + "\n";
		regs += regtostr(" EFL", _thread_ctx.efl) + "\n\n";

		regs += regtostr(" CS", _thread_ctx.cs) + "\n";
		regs += regtostr(" DS", _thread_ctx.ds) + "\n";
		regs += regtostr(" ES", _thread_ctx.es) + "\n";
		regs += regtostr(" FS", _thread_ctx.fs) + "\n";
		regs += regtostr(" GS", _thread_ctx.gs) + "\n";
		regs += regtostr(" SS", _thread_ctx.ss) + "\n\n";

		regs += regtostr(" DR0", _thread_ctx.dr0) + "\n";
		regs += regtostr(" DR1", _thread_ctx.dr1) + "\n";
		regs += regtostr(" DR2", _thread_ctx.dr2) + "\n";
		regs += regtostr(" DR3", _thread_ctx.dr3) + "\n";
		regs += regtostr(" DR6", _thread_ctx.dr6) + "\n";
		regs += regtostr(" DR7", _thread_ctx.dr7) + "\n";
		break;

	case xdv::object::id::XENOM_X64_ARCHITECTURE_OBJECT:
	case xdv::object::id::XENOM_X64_ANALYZER_OBJECT:
		regs += regtostr(" RAX", _thread_ctx.rax) + "\n";
		regs += regtostr(" RBX", _thread_ctx.rbx) + "\n";
		regs += regtostr(" RCX", _thread_ctx.rcx) + "\n";
		regs += regtostr(" RDX", _thread_ctx.rdx) + "\n\n";

		regs += regtostr(" RSP", _thread_ctx.rsp) + "\n";
		regs += regtostr(" RBP", _thread_ctx.rbp) + "\n\n";

		regs += regtostr(" RDI", _thread_ctx.rdi) + "\n";
		regs += regtostr(" RSI", _thread_ctx.rsi) + "\n\n";

		regs += regtostr(" RIP", _thread_ctx.rip) + "\n";
		regs += regtostr(" EFL", _thread_ctx.efl) + "\n\n";

		regs += regtostr(" CS", _thread_ctx.cs) + "\n";
		regs += regtostr(" DS", _thread_ctx.ds) + "\n";
		regs += regtostr(" ES", _thread_ctx.es) + "\n";
		regs += regtostr(" FS", _thread_ctx.fs) + "\n";
		regs += regtostr(" GS", _thread_ctx.gs) + "\n";
		regs += regtostr(" SS", _thread_ctx.ss) + "\n\n";

		regs += regtostr(" DR0", _thread_ctx.dr0) + "\n";
		regs += regtostr(" DR1", _thread_ctx.dr1) + "\n";
		regs += regtostr(" DR2", _thread_ctx.dr2) + "\n";
		regs += regtostr(" DR3", _thread_ctx.dr3) + "\n";
		regs += regtostr(" DR6", _thread_ctx.dr6) + "\n";
		regs += regtostr(" DR7", _thread_ctx.dr7) + "\n";
		break;
	}

	XdvPrintAndClear(_current_handle, regs, false);

	return nullvar();
}

std::string regtostr(std::string name, unsigned long long val)
{
	std::string line;
	line = name + "   ";
	char reg[100] = { 0, };
	sprintf_s(reg, sizeof(reg), "%I64x", val);

	return line + reg;
}