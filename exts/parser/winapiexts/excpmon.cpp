#include "winapiexts.h"

bool RecoverExceptionPoint(DEBUG_EVENT e);
bool RecoverStepOverPtr(WindowsApi *wa, DEBUG_EVENT e, unsigned long long * step_over_ptr);
bool ReInstallBreakPoint(WindowsApi *wa);
//
bool ProcessExecution(WindowsApi *wa, DEBUG_EVENT e);
bool StepInto(WindowsApi *wa, DEBUG_EVENT e);
bool StepOver(WindowsApi *wa, DEBUG_EVENT e, unsigned long long * dest_ptr);
void ExceptionMonitor(void * ctx)
{
	WindowsApi * wa = (WindowsApi *)ctx;
	if (!wa)
	{
		return;
	}

	if (!DebugActiveProcess(wa->ProcessId()))
	{
		return;
	}

	unsigned long long step_over = 0;
	unsigned long long step_ptr = 0;
	do
	{
		DEBUG_EVENT e;
		if (!WaitForDebugEvent(&e, INFINITE))
		{
			break;
		}

		if (e.dwDebugEventCode != EXCEPTION_DEBUG_EVENT)
		{
			ContinueDebugEvent(e.dwProcessId, e.dwThreadId, DBG_CONTINUE);
			continue;
		}

		RecoverExceptionPoint(e);
		RecoverStepOverPtr(wa, e, &step_over);
		ReInstallBreakPoint(wa);

		wa->Select(e.dwThreadId);
		XdvSetDebugEvent();
		XdvWaitForReturnEvent();

		unsigned long status = DBG_EXCEPTION_NOT_HANDLED;
		switch (wa->GetDebugStatus())
		{
		case DebuggeeStatusId::DEBUGGEE_STATUS_EXECUTION:
			if (ProcessExecution(wa, e))
			{
				step_ptr = (unsigned long long)e.u.Exception.ExceptionRecord.ExceptionAddress;
				status = DBG_CONTINUE;
			}
			else if (step_ptr)
			{
				status = DBG_CONTINUE;
				step_ptr = 0;
			}
			break;

		case DebuggeeStatusId::DEBUGGEE_STATUS_STEP_INTO:
			if (StepInto(wa, e))
			{
				status = DBG_CONTINUE;
				step_ptr = (unsigned long long)e.u.Exception.ExceptionRecord.ExceptionAddress;
			}
			break;

		case DebuggeeStatusId::DEBUGGEE_STATUS_STEP_OVER:
			if (StepOver(wa, e, &step_over))
			{
				status = DBG_CONTINUE;
				step_ptr = (unsigned long long)e.u.Exception.ExceptionRecord.ExceptionAddress;
			}
			break;
		}

		ContinueDebugEvent(e.dwProcessId, e.dwThreadId, status);
	} while (true);
}

//
bool SetExceptionContext(unsigned long tid, unsigned long long ptr)
{
	HANDLE thread_handle = OpenThread(MAXIMUM_ALLOWED, FALSE, tid);
	if (!thread_handle)
	{
		false;
	}
	std::shared_ptr<void> thread_handle_closer(thread_handle, CloseHandle);

	CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_CONTROL;
	if (!GetThreadContext(thread_handle, &ctx))
	{
		false;
	}

#ifdef _WIN64
	ctx.Rip = ptr;
#else
	ctx.Eip = (unsigned long)ptr;
#endif

	if (!SetThreadContext(thread_handle, &ctx))
	{
		false;
	}

	return true;
}

bool RecoverExceptionPoint(DEBUG_EVENT e)
{
	unsigned long long ptr = (unsigned long long)e.u.Exception.ExceptionRecord.ExceptionAddress;
	if (!SetExceptionContext(e.dwThreadId, ptr))
	{
		return false;
	}

	return true;
}

bool ReInstallBreakPoint(WindowsApi *wa)
{
	wa->ReInstallAllBreakPoint();
	return true;
}

// debugging api
bool ProcessExecution(WindowsApi *wa, DEBUG_EVENT e)
{
	unsigned long long ptr = (unsigned long long)e.u.Exception.ExceptionRecord.ExceptionAddress;
	if (wa->GetBreakPointId(ptr) != DebugBreakPointId::NO_BREAK_POINT_ID)
	{
		if (StepInto(wa, e))
		{
			XdvReturnEvent();

			return true;
		}
	}

	return false;
}

bool StepInto(WindowsApi *wa, DEBUG_EVENT e)
{
	wa->RestoreAllBreakPoint();

	HANDLE th = OpenThread(MAXIMUM_ALLOWED, FALSE, e.dwThreadId);
	if (!th)
	{
		return false;
	}
	std::shared_ptr<void> th_closer(th, CloseHandle);

	CONTEXT ctx = { 0, };
	ctx.ContextFlags = CONTEXT_ALL;
	if (!::GetThreadContext(th, &ctx))
	{
		return false;
	}

	ctx.EFlags |= 0x100;
	if (!::SetThreadContext(th, &ctx))
	{
		return false;
	}

	return true;
}

bool StepOver(WindowsApi *wa, DEBUG_EVENT e, unsigned long long * dest_ptr)
{
	HANDLE th = OpenThread(MAXIMUM_ALLOWED, FALSE, e.dwThreadId);
	if (!th)
	{
		return false;
	}
	std::shared_ptr<void> th_closer(th, CloseHandle);

	CONTEXT ctx = { 0, };
	ctx.ContextFlags = CONTEXT_ALL;
	if (!::GetThreadContext(th, &ctx))
	{
		return false;
	}

	wa->RestoreAllBreakPoint();

	unsigned char dump[16] = { 0, };
#ifdef _WIN64
	unsigned long long ptr = ctx.Rip;
#else
	unsigned long long ptr = ctx.Eip;
#endif
	auto readn = wa->Read(ptr, dump, 16);
	if (readn == 0)
	{
		return StepInto(wa, e);
	}

	if (!XdvIsCallCode(XdvGetArchitectureHandle(), ptr, dump))
	{
		return StepInto(wa, e);
	}

	xdv::architecture::x86::type x86ctx;
	if (!XdvDisassemble(XdvGetArchitectureHandle(), ptr, dump, &x86ctx))
	{
		return StepInto(wa, e);
	}

	auto next_ptr = ptr + x86ctx.instruction_size;
	if (!wa->SetBreakPoint(DebugBreakPointId::SOFTWARE_BREAK_POINT_ID, next_ptr))
	{
		return StepInto(wa, e);
	}

	*dest_ptr = next_ptr;

	return true;
}

bool RecoverStepOverPtr(WindowsApi *wa, DEBUG_EVENT e, unsigned long long * step_over_ptr)
{
	unsigned long long ptr = (unsigned long long)e.u.Exception.ExceptionRecord.ExceptionAddress;
	if (ptr == *step_over_ptr)
	{
		*step_over_ptr = 0;
		return wa->DeleteBreakPoint(ptr);
	}

	return false;
}

// -------------------------------------
// Suspend Point
void SuspendCallback(unsigned long long ptr)
{
	do
	{
		bool result = false;
		std::map<unsigned long, unsigned long long> thread_map;
		XdvThreads(XdvGetParserHandle(), thread_map);
		for (auto it : thread_map)
		{
			if (XdvSelectThread(XdvGetParserHandle(), it.first))
			{
				xdv::architecture::x86::context::type ctx;
				if (XdvGetThreadContext(XdvGetParserHandle(), &ctx))
				{
					if (ctx.rip == ptr || ctx.rip == ptr + 1)
					{
						XdvSelectThread(XdvGetParserHandle(), it.first);
						result = true;
						break;
					}
				}
			}
		}

		if (result)
		{
			break;
		}

		std::chrono::seconds dura_sec(1);
		std::this_thread::sleep_for(dura_sec);
	} while (true);

	XdvSuspendProcess(XdvGetParserHandle());
	xdv::architecture::x86::context::type ctx;
	if (XdvGetThreadContext(XdvGetParserHandle(), &ctx))
	{
		ctx.rip = ptr;
		XdvSetThreadContext(XdvGetParserHandle(), &ctx);
	}
	XdvRestoreBreakPoint(XdvGetParserHandle(), ptr);

	XdvSetDebugEvent();
}
