#include "rmtexts.h"

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "corexts.lib")

RemoteApi::RemoteApi()
{
	debugger_ = (IDebugger *)XdvGetObjectByString("Windows Debugger");
	if (debugger_)
	{
		if (debugger_->ObjectType() != xdv::object::id::XENOM_DEBUGGER_SYSTEM_OBJECT)
		{
			debugger_ = nullptr;
		}
	}
}

RemoteApi::~RemoteApi()
{
}

xdv::object::id RemoteApi::ObjectType()
{
	return xdv::object::id::XENOM_DEBUGGER_SYSTEM_OBJECT;
}

std::string RemoteApi::ObjectString()
{
	return " VEH Engine, [DEBUGGER]";
}

void RemoteApi::SetModuleName(std::string module)
{
}

std::string RemoteApi::ModuleName()
{
	return "";
}

std::map<unsigned long, std::string> RemoteApi::ProcessList()
{
	std::map<unsigned long, std::string> pl;
	if (debugger_)
	{
		pl = debugger_->ProcessList();
	}
	return pl;
}

unsigned long RemoteApi::WaitForProcess(std::string process_name)
{
	unsigned long pid = 0;
	if (debugger_)
	{
		pid = debugger_->WaitForProcess(process_name);
	}

	return pid;
}

void ExceptionMonitor(void * ctx);
bool RemoteApi::Attach(unsigned long pid)
{
	DebugContextPtr dcp = (DebugContextPtr)XdvDebugSharedMemory();
	if (!dcp)
	{
		if (XdvInjectModule(L"vehexts.dll"))
		{
			std::thread * em = new std::thread(ExceptionMonitor, this);
			return true;
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool RemoteApi::Open(char *path)
{
	return false;
}

bool RemoteApi::Open(unsigned long pid)
{
	if (debugger_)
	{
		return debugger_->Open(pid);
	}

	return false;
}

bool RemoteApi::Update() // 삭제해야함
{
	return false;
}

unsigned long RemoteApi::ProcessId()
{
	unsigned long pid = 0;
	if (debugger_)
	{
		pid = debugger_->ProcessId();
	}

	return pid;
}

unsigned long long RemoteApi::Read(unsigned long long ptr, unsigned char *out_memory, unsigned long read_size)
{
	unsigned long long result = 0;
	if (debugger_)
	{
		result = debugger_->Read(ptr, out_memory, read_size);
	}

	return result;
}

unsigned long long RemoteApi::Write(void * ptr, unsigned char *input_memory, unsigned long write_size)
{
	unsigned long long result = 0;
	if (debugger_)
	{
		result = debugger_->Write(ptr, input_memory, write_size);
	}

	return result;
}

bool RemoteApi::Query(unsigned long long ptr, xdv::memory::type *memory_type)
{
	if (debugger_)
	{
		return debugger_->Query(ptr, memory_type);
	}

	return false;
}

void * RemoteApi::Alloc(void *ptr, unsigned long size, unsigned long allocation_type, unsigned long protect_type)
{
	if (debugger_)
	{
		return debugger_->Alloc(ptr, size, allocation_type, protect_type);
	}

	return nullptr;
}

bool RemoteApi::Select(unsigned long tid)
{
	if (debugger_)
	{
		return debugger_->Select(tid);
	}

	return false;
}

void RemoteApi::Threads(std::map<unsigned long, unsigned long long> &thread_info_map)
{
	if (debugger_)
	{
		debugger_->Threads(thread_info_map);
	}
}

bool RemoteApi::GetThreadContext(xdv::architecture::x86::context::type *context)
{
	DebugContextPtr dcp = (DebugContextPtr)XdvDebugSharedMemory();
	if (dcp)
	{
		*context = dcp->context;
		return true;
	}

	if (debugger_)
	{
		return debugger_->GetThreadContext(context);
	}

	return false;
}

bool RemoteApi::SetThreadContext(xdv::architecture::x86::context::type *context)
{
	DebugContextPtr dcp = (DebugContextPtr)XdvDebugSharedMemory();
	if (dcp)
	{
		dcp->context = *context;
		return true;
	}

	if (debugger_)
	{
		return debugger_->GetThreadContext(context);;
	}

	return false;
}

bool RemoteApi::SuspendThread(unsigned long tid)
{
	bool result = false;
	if (debugger_)
	{
		result = debugger_->SuspendThread(tid);
	}

	return result;
}

bool RemoteApi::ResumeThread(unsigned long tid)
{
	bool result = false;
	if (debugger_)
	{
		result = debugger_->ResumeThread(tid);
	}

	return result;
}

void RemoteApi::ResumeProcess()
{
	std::map<unsigned long, unsigned long long> thread_map;
	debugger_->Threads(thread_map);

	for (auto it : thread_map)
	{
		ResumeThread(it.first);
	}
}

std::string RemoteApi::Module(unsigned long long ptr)
{
	if (debugger_)
	{
		return debugger_->Module(ptr);
	}

	return "";
}

unsigned long RemoteApi::Symbol(unsigned long long ptr, unsigned long long *disp, char *symbol_str, unsigned long symbol_size)
{
	unsigned long result = 0;
	if (debugger_)
	{
		result = debugger_->Symbol(ptr, disp, symbol_str, symbol_size);
	}

	return result;
}

unsigned long RemoteApi::Symbol(unsigned long long ptr, char *symbol_str, unsigned long symbol_size)
{
	unsigned long result = 0;
	if (debugger_)
	{
		result = debugger_->Symbol(ptr, symbol_str, symbol_size);
	}

	return result;
}

unsigned long long RemoteApi::SymbolToPtr(char *symbol_str)
{
	unsigned long long result = 0;
	if (debugger_)
	{
		result = debugger_->SymbolToPtr(symbol_str);
	}

	return result;
}

bool RemoteApi::StackTrace(xdv::architecture::x86::frame::type *stack_frame, size_t size_of_stack_frame, unsigned long *stack_count)
{
	bool result = false;
	if (debugger_)
	{
		result = debugger_->StackTrace(stack_frame, size_of_stack_frame, stack_count);
	}

	return result;
}

bool RemoteApi::StackTraceEx(unsigned long long bp, unsigned long long ip, unsigned long long sp, xdv::architecture::x86::frame::type *stack_frame, size_t size_of_stack_frame, unsigned long *stack_count)
{
	bool result = false;
	if (debugger_)
	{
		DebugContextPtr dcp = (DebugContextPtr)XdvDebugSharedMemory();
		if (dcp)
		{
			result = debugger_->StackTraceEx(dcp->context.rbp, dcp->context.rip, dcp->context.rsp, stack_frame, size_of_stack_frame, stack_count);
		}
	}

	return result;
}

unsigned long long RemoteApi::GetPebAddress()
{
	unsigned long long result = 0;
	if (debugger_)
	{
		result = debugger_->GetPebAddress();
	}

	return result;
}

unsigned long long RemoteApi::GetTebAddress()
{
	unsigned long long result = 0;
	if (debugger_)
	{
		result = debugger_->GetTebAddress();
	}

	return result;
}

void RemoteApi::SetBreakPointTable()
{
	DebugContextPtr dcp = (DebugContextPtr)XdvDebugSharedMemory();
	if (dcp)
	{
		std::vector<unsigned long long> bps = GetBreakPointList();
		int i = 0;
		for (i = 0; (size_t)i < bps.size(); ++i)
		{
			if (i == 100)
			{
				break;
			}

			dcp->bp[i].id = GetBreakPointId(bps[i]);
			dcp->bp[i].ptr = bps[i];
			memcpy(dcp->bp[i].bytes, GetBpBackupDump(bps[i]), 16);
		}
		dcp->count = i;
	}
}

bool RemoteApi::StepInto(DebugCallbackT callback, void * cb_ctx)
{
	DebugContextPtr dcp = (DebugContextPtr)XdvDebugSharedMemory();
	if (dcp)
	{
		SetBreakPointTable();

		dcp->status = DebuggeeStatusId::DEBUGGEE_STATUS_STEP_INTO;
		XdvReturnEvent();
		ResumeProcess();

		return true;
	}

	return false;
}

bool RemoteApi::StepOver(DebugCallbackT callback, void * cb_ctx)
{
	DebugContextPtr dcp = (DebugContextPtr)XdvDebugSharedMemory();
	if (dcp)
	{
		SetBreakPointTable();

		dcp->status = DebuggeeStatusId::DEBUGGEE_STATUS_STEP_OVER;
		XdvReturnEvent();
		ResumeProcess();

		return true;
	}

	return false;
}

bool RemoteApi::RunningProcess()
{
	DebugContextPtr dcp = (DebugContextPtr)XdvDebugSharedMemory();
	if (dcp && dcp->context.rip)
	{
		SetBreakPointTable();

		dcp->status = DebuggeeStatusId::DEBUGGEE_STATUS_EXECUTION;
		XdvReturnEvent();
	}
	ResumeProcess();

	return true;
}

unsigned char * RemoteApi::GetBpBackupDump(unsigned long long ptr)
{
	unsigned char * result = nullptr;
	if (debugger_)
	{
		result = debugger_->GetBpBackupDump(ptr);
	}

	return result;
}

bool RemoteApi::SetBreakPoint(DebugBreakPointId id, unsigned long long ptr)
{
	bool result = false;
	if (debugger_)
	{
		result = debugger_->SetBreakPoint(id, ptr);
	}

	return result;
}

DebugBreakPointId RemoteApi::GetBreakPointId(unsigned long long ptr)
{
	DebugBreakPointId result = DebugBreakPointId::NO_BREAK_POINT_ID;
	if (debugger_)
	{
		result = debugger_->GetBreakPointId(ptr);
	}

	return result;
}

bool RemoteApi::RestoreBreakPoint(unsigned long long ptr)
{
	bool result = false;
	if (debugger_)
	{
		result = debugger_->RestoreBreakPoint(ptr);
	}

	return result;
}

void RemoteApi::ReInstallBreakPoint(unsigned long long ptr)
{
	if (debugger_)
	{
		debugger_->ReInstallBreakPoint(ptr);
	}
}

bool RemoteApi::DeleteBreakPoint(unsigned long long ptr)
{
	bool result = false;
	if (debugger_)
	{
		result = debugger_->DeleteBreakPoint(ptr);
	}

	return result;
}

void RemoteApi::RestoreAllBreakPoint()
{
	if (debugger_)
	{
		debugger_->RestoreAllBreakPoint();
	}
}

void RemoteApi::ReInstallAllBreakPoint()
{
	if (debugger_)
	{
		debugger_->ReInstallAllBreakPoint();
	}
}

std::vector<unsigned long long> RemoteApi::GetBreakPointList()
{
	std::vector<unsigned long long> v;
	if (debugger_)
	{
		v = debugger_->GetBreakPointList();
	}

	return v;
}

XENOM_ADD_INTERFACE()
{
	IObject * obj = __add_object(RemoteApi);
	if (obj)
	{
		return XdvGetHandleByObject(obj);
	}

	return 0;
}

void RemoteApi::SetStepOverPtr(unsigned long long ptr)
{
	step_over_ptr_ = ptr;
}

unsigned long long RemoteApi::GetStepOverPtr()
{
	return step_over_ptr_;
}
