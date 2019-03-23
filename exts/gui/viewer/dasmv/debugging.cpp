#include "xdv_sdk.h"

typedef struct _tag_break_point_type
{
	xdv::breakpoint::id id;
	unsigned char bytes[16];
}break_point, *break_point_ptr;

std::map<unsigned long long, break_point_ptr> GetBreakPointMap();
EXTS_FUNC(set_swbp)	// argv[0] = ptr
{
	unsigned long long ptr = XdvToUll(argv, argc, "ptr");
	if (ptr == 0)
	{
		return nullvar();
	}

	unsigned char * backup = (unsigned char *)malloc(16);
	memset(backup, 0, 16);
	if (XdvReadMemory(XdvGetParserHandle(), ptr, backup, 16) == 0)
	{
		return nullvar();
	}

	unsigned char dump[16] = { 0, };
	size_t insn_size = 0;
	if (XdvAssemble(XdvGetArchitectureHandle(), dump, &insn_size, "int3"))
	{
		XdvWriteMemory(XdvGetParserHandle(), (void *)ptr, dump, (unsigned long)insn_size);
		return ptrvar(backup);
	}

	return nullvar();
}

EXTS_FUNC(debuggeerun)
{
	// 현재 EIP를 확인하고, BP에서 멈춘 경우, Step(install bp where is next insn and remove bp)을 수행한다.
	DebugContextPtr dcp = (DebugContextPtr)XdvDebugSharedMemory();
	if (dcp)
	{
		dcp->status = 0;

		XdvReturnEvent();
	}

	return nullvar();
}

EXTS_FUNC(stepinto)
{
	unsigned long long ptr = XdvToUll(argv, argc, "ptr");
	if (ptr == 0)
	{
		return nullvar();
	}

	return nullvar();
}

EXTS_FUNC(checkpoint)
{
	unsigned long long ptr = XdvToUll(argv, argc, "ptr");
	if (ptr == 0)
	{
		return nullvar();
	}

	std::map<unsigned long long, break_point_ptr> break_point_map = GetBreakPointMap();
	std::map<unsigned long long, break_point_ptr>::iterator it = break_point_map.find(ptr);
	if (it != break_point_map.end())
	{
		return ullvar(1);
	}

	return ullvar(0);
}

// ------------------------------------------------
//
EXTS_FUNC(install_pc)
{
	unsigned long long ptr = XdvToUll(argv, argc, "ptr");
	if (ptr == 0)
	{
		return nullvar();
	}

	unsigned char lp[3] = { 0x90, 0xEB, 0xFD };
	if (XdvWriteMemory(XdvGetParserHandle(), (void *)ptr, lp, 3))
	{
		return ullvar(1);
	}

	return ullvar(0);
}

EXTS_FUNC(pc_check)
{
	unsigned long long ptr = XdvToUll(argv, argc, "ptr");
	if (ptr == 0)
	{
		return nullvar();
	}

	do
	{
		XdvUpdateDebuggee(XdvGetParserHandle());
		xdv::architecture::x86::context::type ctx;
		if (XdvGetThreadContext(XdvGetParserHandle(), &ctx))
		{
			if (ctx.rip == ptr || ctx.rip == ptr + 1)
			{
				break;
			}
		}
	} while (true);

	return ullvar(1);
}

EXTS_FUNC(restorepoint)
{
	unsigned long long ptr = XdvToUll(argv, argc, "ptr");
	if (ptr == 0)
	{
		return nullvar();
	}

	std::map<unsigned long long, break_point_ptr> break_point_map = GetBreakPointMap();
	std::map<unsigned long long, break_point_ptr>::iterator it = break_point_map.find(ptr);
	if (it != break_point_map.end())
	{
		if (XdvWriteMemory(XdvGetParserHandle(), (void *)ptr, it->second->bytes, 16))
		{
			xdv::architecture::x86::context::type ctx;
			if (XdvGetThreadContext(XdvGetParserHandle(), &ctx))
			{
				ctx.rip = ptr;
				if (XdvSetThreadContext(XdvGetParserHandle(), &ctx))
				{
					return ullvar(1);
				}
			}
		}
	}

	return ullvar(0);
}