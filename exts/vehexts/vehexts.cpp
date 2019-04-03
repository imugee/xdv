#include <windows.h>
#include <winternl.h>
#include <stdio.h>
#include <conio.h>
#include <mutex>

#include "xdv_sdk.h"
#include "include/distorm.h"
#include "include/mnemonics.h"

#pragma comment(lib, "corexts.lib")

bool Disassemble(void *code, size_t size, _DecodeType dt, _DInst *di);

void ReInstallBreakPoint(DebugContextPtr e, CONTEXT * ctx);
void RemoveBreakPoint(DebugContextPtr e, CONTEXT * ctx);

bool ProcessExecution(DebugContextPtr e, CONTEXT * ctx);
bool StepInto(DebugContextPtr e, CONTEXT * ctx);
bool StepOver(DebugContextPtr e, CONTEXT * ctx);
bool RemoveStepOver(unsigned long long ptr);

void SetContext(CONTEXT * ctx, xdv::architecture::x86::context::type xctx);

unsigned long long _step = 0;
unsigned long long _step_over = 0;
unsigned char _step_over_table[16] = { 0, };

std::mutex _vector_mutex;
xdv::architecture::x86::context::type SetContext(CONTEXT ctx);

//
//
LONG CALLBACK VectoredHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
	_vector_mutex.lock();

	if (XdvInstallRemoteEvent(GetCurrentProcessId()))
	{
		_vector_mutex.unlock();
		return EXCEPTION_CONTINUE_SEARCH;
	}

	CONTEXT * ctx = ExceptionInfo->ContextRecord;
	DebugContextPtr e = (DebugContextPtr)XdvDebugSharedMemory();
	if (!e)
	{
		_vector_mutex.unlock();
		return EXCEPTION_CONTINUE_SEARCH;
	}

	unsigned long long ptr = (unsigned long long)ExceptionInfo->ExceptionRecord->ExceptionAddress;
	ReInstallBreakPoint(e, ctx);
	RemoveStepOver(ptr);

	//printf("xdv:: call vector %08x %p\n", ExceptionInfo->ExceptionRecord->ExceptionCode, ExceptionInfo->ExceptionRecord->ExceptionAddress);
	e->context = SetContext(*(ExceptionInfo->ContextRecord));
	e->error_code = ExceptionInfo->ExceptionRecord->ExceptionCode;
	e->tid = GetCurrentThreadId();

	XdvExceptionEvent();
	XdvWaitForReturnEvent();
	SetContext(ExceptionInfo->ContextRecord, e->context);

	long status = EXCEPTION_CONTINUE_SEARCH;
	switch (e->status)
	{
	case DebuggeeStatusId::DEBUGGEE_STATUS_EXECUTION:
		if (ProcessExecution(e, ctx))
		{
			_step = ptr;
		}
		else if (_step)
		{
			_step = 0;
		}
		status = EXCEPTION_CONTINUE_EXECUTION;
		break;

	case DebuggeeStatusId::DEBUGGEE_STATUS_STEP_INTO:
		StepInto(e, ctx);
		_step = ptr;
		status = EXCEPTION_CONTINUE_EXECUTION;
		break;

	case DebuggeeStatusId::DEBUGGEE_STATUS_STEP_OVER:
		StepOver(e, ctx);
		_step = ptr;
		status = EXCEPTION_CONTINUE_EXECUTION;
		break;

	default:
		break;
	}

	XdvCloseRemoteEvent();
	_vector_mutex.unlock();

	return status;
}

//
bool InstallSoftwarePoint(unsigned long long ptr)
{
	unsigned long old = 0;
	if (!VirtualProtect((void *)ptr, 1, PAGE_EXECUTE_READWRITE, &old))
	{
		return false;
	}

	unsigned char lp[1] = { 0xcc };
	memcpy((void *)ptr, lp, 1);

	VirtualProtect((void *)ptr, 1, old, &old);

	return true;
}

bool InstallHardwarePoint(CONTEXT * ctx, unsigned long long ptr)
{
#ifndef _WIN64
	unsigned long point = (unsigned long)ptr;
#else
	unsigned long long point = ptr;
#endif

	if (ctx->Dr0 == 0)
	{
		ctx->Dr0 = point;
	}
	else if (ctx->Dr1 == 0)
	{
		ctx->Dr1 = point;
	}
	else if (ctx->Dr2 == 0)
	{
		ctx->Dr2 = point;
	}
	else if (ctx->Dr3 == 0)
	{
		ctx->Dr3 = point;
	}
	else
	{
		ctx->Dr0 = point;
	}

	ctx->Dr7 = (1 << 0) | (1 << 2) | (1 << 4) | (1 << 6);
	return true;
}

void ReInstallBreakPoint(DebugContextPtr e, CONTEXT * ctx)
{
	for (int i = 0; i < e->count; ++i)
	{
		switch (e->bp[i].id)
		{
		case DebugBreakPointId::SOFTWARE_BREAK_POINT_ID:
			InstallSoftwarePoint(e->bp[i].ptr);
			break;

		case DebugBreakPointId::HARDWARE_BREAK_POINT_ID:
			InstallHardwarePoint(ctx, e->bp[i].ptr);
			break;
		}
	}
}

void RemoveBreakPoint(DebugContextPtr e, CONTEXT * ctx)
{
	for (int i = 0; i < e->count; ++i)
	{
		switch (e->bp[i].id)
		{
		case DebugBreakPointId::SOFTWARE_BREAK_POINT_ID:
		{
			unsigned long old = 0;
			if (VirtualProtect((void *)e->bp[i].ptr, 16, PAGE_EXECUTE_READWRITE, &old))
			{
				memcpy((void *)e->bp[i].ptr, e->bp[i].bytes, 16);
				VirtualProtect((void *)e->bp[i].ptr, 16, old, &old);
			}

			break;
		}

		case DebugBreakPointId::HARDWARE_BREAK_POINT_ID:
			ctx->Dr0 = 0;
			ctx->Dr1 = 0;
			ctx->Dr2 = 0;
			ctx->Dr3 = 0;
			ctx->Dr6 = 0;
			ctx->Dr7 = 0;
			break;
		}
	}
}

//
bool ProcessExecution(DebugContextPtr e, CONTEXT * ctx)
{
#ifdef _WIN64
	unsigned long long ptr = ctx->Rip;
#else
	unsigned long long ptr = ctx->Eip;
#endif

	for (int i = 0; i < e->count; ++i)
	{
		if (ptr == e->bp[i].ptr)
		{
			StepInto(e, ctx);
			XdvReturnEvent();

			return true;
		}
	}

	return false;
}

bool StepInto(DebugContextPtr e, CONTEXT * ctx)
{
	RemoveBreakPoint(e, ctx);
	ctx->EFlags |= 0x100;
	return true;
}

bool StepOver(DebugContextPtr e, CONTEXT * ctx)
{
	RemoveBreakPoint(e, ctx);

#ifdef _WIN64
	unsigned long long ptr = ctx->Rip;
#else
	unsigned long long ptr = ctx->Eip;
#endif
	_DInst di;
#ifndef _WIN64
	_DecodeType dt = Decode32Bits;
#else
	_DecodeType dt = Decode64Bits;
#endif

	if (!Disassemble((void *)ptr, 64, dt, &di))
	{
		return StepInto(e, ctx);
	}

	if (di.opcode != I_CALL)
	{
		return StepInto(e, ctx);
	}

	auto next_ptr = ptr + di.size;
	memcpy((void *)_step_over_table, (void *)next_ptr, 16);
	if (!InstallSoftwarePoint(next_ptr))
	{
		return StepInto(e, ctx);
	}
	_step_over = next_ptr;

	return true;
}

bool RemoveStepOver(unsigned long long ptr)
{
	if (ptr == _step_over)
	{
		unsigned long old = 0;
		if (!VirtualProtect((void *)ptr, 16, PAGE_EXECUTE_READWRITE, &old))
		{
			return false;
		}

		memcpy((void *)ptr, _step_over_table, 16);
		VirtualProtect((void *)ptr, 16, old, &old);

		return true;
	}

	return false;
}

//
xdv::architecture::x86::context::type SetContext(CONTEXT ctx)
{
	xdv::architecture::x86::context::type xctx;
#ifdef _WIN64
	xctx.rax = ctx.Rax;
	xctx.rbx = ctx.Rbx;
	xctx.rcx = ctx.Rcx;
	xctx.rdx = ctx.Rdx;

	xctx.rdi = ctx.Rdi;
	xctx.rsi = ctx.Rsi;

	xctx.rbp = ctx.Rbp;
	xctx.rsp = ctx.Rsp;

	xctx.rip = ctx.Rip;

	xctx.efl = ctx.EFlags;
	xctx.cs = ctx.SegCs;
	xctx.ds = ctx.SegDs;
	xctx.es = ctx.SegEs;
	xctx.fs = ctx.SegFs;
	xctx.gs = ctx.SegGs;
	xctx.ss = ctx.SegSs;

	xctx.dr0 = ctx.Dr0;
	xctx.dr1 = ctx.Dr1;
	xctx.dr2 = ctx.Dr2;
	xctx.dr3 = ctx.Dr3;
	xctx.dr6 = ctx.Dr6;
	xctx.dr7 = ctx.Dr7;
#else
	xctx.rax = ctx.Eax;
	xctx.rbx = ctx.Ebx;
	xctx.rcx = ctx.Ecx;
	xctx.rdx = ctx.Edx;

	xctx.rdi = ctx.Edi;
	xctx.rsi = ctx.Esi;

	xctx.rbp = ctx.Ebp;
	xctx.rsp = ctx.Esp;

	xctx.rip = ctx.Eip;

	xctx.efl = ctx.EFlags;
	xctx.cs = ctx.SegCs;
	xctx.ds = ctx.SegDs;
	xctx.es = ctx.SegEs;
	xctx.fs = ctx.SegFs;
	xctx.gs = ctx.SegGs;
	xctx.ss = ctx.SegSs;

	xctx.dr0 = ctx.Dr0;
	xctx.dr1 = ctx.Dr1;
	xctx.dr2 = ctx.Dr2;
	xctx.dr3 = ctx.Dr3;
	xctx.dr6 = ctx.Dr6;
	xctx.dr7 = ctx.Dr7;
#endif

	return xctx;
}

void SetContext(CONTEXT * ctx, xdv::architecture::x86::context::type xctx)
{
#ifdef _WIN64
	ctx->Rax = xctx.rax;
	ctx->Rbx = xctx.rbx;
	ctx->Rcx = xctx.rcx;
	ctx->Rdx = xctx.rdx;

	ctx->Rdi = xctx.rdi;
	ctx->Rsi = xctx.rsi;

	ctx->Rbp = xctx.rbp;
	ctx->Rsp = xctx.rsp;

	ctx->EFlags = xctx.efl;

	ctx->Dr0 = xctx.dr0;
	ctx->Dr1 = xctx.dr1;
	ctx->Dr2 = xctx.dr2;
	ctx->Dr3 = xctx.dr3;
	ctx->Dr6 = xctx.dr6;
	ctx->Dr7 = xctx.dr7;

#else
	ctx->Eax = (unsigned long)xctx.rax;
	ctx->Ebx = (unsigned long)xctx.rbx;
	ctx->Ecx = (unsigned long)xctx.rcx;
	ctx->Edx = (unsigned long)xctx.rdx;

	ctx->Edi = (unsigned long)xctx.rdi;
	ctx->Esi = (unsigned long)xctx.rsi;

	ctx->Ebp = (unsigned long)xctx.rbp;
	ctx->Esp = (unsigned long)xctx.rsp;

	ctx->EFlags = xctx.efl;

	ctx->Dr0 = (unsigned long)xctx.dr0;
	ctx->Dr1 = (unsigned long)xctx.dr1;
	ctx->Dr2 = (unsigned long)xctx.dr2;
	ctx->Dr3 = (unsigned long)xctx.dr3;
	ctx->Dr6 = (unsigned long)xctx.dr6;
	ctx->Dr7 = (unsigned long)xctx.dr7;
#endif
}

bool Disassemble(void *code, size_t size, _DecodeType dt, _DInst *di)
{
	unsigned int dc;
	_CodeInfo ci;

	ci.code = (unsigned char *)code;
	ci.codeLen = (int)size;
	ci.codeOffset = (_OffsetType)code;
	ci.dt = dt;
	ci.features = DF_NONE;

	if (distorm_decompose64(&ci, di, 1, &dc) == DECRES_INPUTERR)
	{
		return false;
	}

	if (dc != 1)
	{
		return false;
	}

	return true;
}
