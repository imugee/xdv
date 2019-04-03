#include "winapiexts.h"

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "corexts.lib")

WindowsApi::WindowsApi()
	: pid_(0), tid_(0), process_handle_(nullptr), debuggee_status_(DebuggeeStatusId::DEBUGGEE_STATUS_NEXT_HANDLE)
{
}

WindowsApi::~WindowsApi()
{
}

xdv::object::id WindowsApi::ObjectType()
{
	return xdv::object::id::XENOM_DEBUGGER_SYSTEM_OBJECT;
}

std::string WindowsApi::ObjectString()
{
	return "[Windows API]";
}

void WindowsApi::SetModuleName(std::string module)
{
}

std::string WindowsApi::ModuleName()
{
	return "";
}

std::map<unsigned long, std::string> WindowsApi::ProcessList()
{
	std::map<unsigned long, std::string> process_map;
	PROCESSENTRY32 process_block32 = { 0, };
	HANDLE snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (!snapshot_handle)
	{
		return process_map;
	}
	std::shared_ptr<void> handle_closer(snapshot_handle, CloseHandle);

	process_block32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(snapshot_handle, &process_block32))
	{
		return process_map;
	}

	do
	{
		unsigned long pid = process_block32.th32ProcessID;
		std::wstring wpn = process_block32.szExeFile;
		std::string pn(wpn.begin(), wpn.end());

		process_map[pid] = pn;
	} while (Process32Next(snapshot_handle, &process_block32));

	return process_map;
}

bool nametoid(wchar_t *process_name, unsigned long *pid)
{
	PROCESSENTRY32 process_block32 = { 0, };
	HANDLE snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (!snapshot_handle)
		return false;
	std::shared_ptr<void> handle_closer(snapshot_handle, CloseHandle);

	process_block32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(snapshot_handle, &process_block32))
	{
		return false;
	}

	do
	{
		if (wcsstr(process_block32.szExeFile, process_name))
		{
			*pid = process_block32.th32ProcessID;

			return true;
		}
	} while (Process32Next(snapshot_handle, &process_block32));

	return false;
}

unsigned long WindowsApi::WaitForProcess(std::string process_name)
{
	std::wstring wprocess_name(process_name.begin(), process_name.end());
	unsigned long pid = 0;
	while (1)
	{
		if (nametoid((wchar_t *)wprocess_name.c_str(), &pid))
		{
			break;
		}
	}

	return pid;
}

void ExceptionMonitor(void * ctx);
bool WindowsApi::Attach(unsigned long pid)
{
	if (XdvInstallRemoteEvent(pid) != 1)
	{
		std::thread *em = new std::thread(ExceptionMonitor, this);
	}

	return true;
}

bool WindowsApi::Open(char *path)
{
	return false;
}

bool WindowsApi::Open(unsigned long pid)
{
	SymSetOptions(SYMOPT_LOAD_LINES);

	pid_ = pid;
	process_handle_ = OpenProcess(MAXIMUM_ALLOWED, FALSE, pid);
	if (!process_handle_)
	{
		return false;
	}

	if (!SymInitialize(process_handle_, 0, TRUE))
	{
		return 0;
	}

	return true;
}

bool WindowsApi::Update() // 삭제해야함
{
	return false;
}

unsigned long WindowsApi::ProcessId()
{
	return pid_;
}

unsigned long long WindowsApi::Read(unsigned long long ptr, unsigned char *out_memory, unsigned long read_size)
{
	HANDLE ph = OpenProcess(MAXIMUM_ALLOWED, FALSE, pid_);
	if (!ph)
	{
		return 0;
	}
	std::shared_ptr<void> ph_closer(ph, CloseHandle);

	SIZE_T readn = 0;
	if (!ReadProcessMemory(ph, (void *)ptr, out_memory, read_size, &readn))
	{
		return 0;
	}

	return (unsigned long long)readn;
}

unsigned long long WindowsApi::Write(void * ptr, unsigned char *input_memory, unsigned long write_size)
{
	SIZE_T written = 0;
	if (!WriteProcessMemory(process_handle_, (void *)ptr, input_memory, write_size, &written))
	{
		return 0;
	}

	return (unsigned long long)written;
}

bool WindowsApi::Query(unsigned long long ptr, xdv::memory::type *memory_type)
{
	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQueryEx(process_handle_, (void *)ptr, &mbi, sizeof(mbi)) != sizeof(mbi))
	{
		return false;
	}

	memory_type->AllocationBase = (unsigned long long)mbi.AllocationBase;
	memory_type->AllocationProtect = mbi.AllocationProtect;
	memory_type->BaseAddress = (unsigned long long)mbi.BaseAddress;
	memory_type->Protect = mbi.Protect;
	memory_type->RegionSize = (unsigned long long)mbi.RegionSize;
	memory_type->State = mbi.State;
	memory_type->Type = mbi.Type;

	return true;
}

void * WindowsApi::Alloc(void *ptr, unsigned long size, unsigned long allocation_type, unsigned long protect_type)
{
	return VirtualAllocEx(process_handle_, nullptr, size, allocation_type, protect_type);
}

bool WindowsApi::Select(unsigned long tid)
{
	if (tid == 0)
	{
		std::map<unsigned long, unsigned long long> thread_info_map;
		this->Threads(thread_info_map);
		auto it = thread_info_map.begin();
		tid_ = it->first;
	}
	else
	{
		tid_ = tid;
	}

	return true;
}

unsigned long WindowsApi::ThreadId()
{
	return tid_;
}

void WindowsApi::Threads(std::map<unsigned long, unsigned long long> &thread_info_map)
{
	HANDLE hs = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (!hs)
	{
		return;
	}
	std::shared_ptr<void> hs_closer(hs, CloseHandle);

	THREADENTRY32 thread_block32 = { 0, };
	thread_block32.dwSize = sizeof(THREADENTRY32);

	if (!Thread32First(hs, &thread_block32))
	{
		return;
	}

#define ThreadQuerySetWin32StartAddress		9

	typedef NTSTATUS(__stdcall *NtQueryInformationThreadT)(HANDLE, DWORD, PVOID, ULONG, PULONG);
	NtQueryInformationThreadT NtQueryInformationThread = (NtQueryInformationThreadT)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtQueryInformationThread");
	if (!NtQueryInformationThread)
	{
		return;
	}

	do
	{
		if (thread_block32.th32OwnerProcessID == pid_)
		{
			HANDLE ht = OpenThread(THREAD_QUERY_INFORMATION, FALSE, thread_block32.th32ThreadID);
			if (ht)
			{
				PVOID addr;
				if (NT_SUCCESS(NtQueryInformationThread(ht, ThreadQuerySetWin32StartAddress, &addr, sizeof(addr), NULL)))
				{
					thread_info_map.insert(std::pair<unsigned long, unsigned long long>(thread_block32.th32ThreadID, (unsigned long long)addr));
				}
				else
				{
					thread_info_map.insert(std::pair<unsigned long, unsigned long long>(thread_block32.th32ThreadID, 0));
				}
				CloseHandle(ht);
			}
		}
	} while (Thread32Next(hs, &thread_block32));
}

bool WindowsApi::GetThreadContext(xdv::architecture::x86::context::type *context)
{
	if (tid_ == 0)
	{
		std::map<unsigned long, unsigned long long> thread_info_map;
		this->Threads(thread_info_map);
		auto it = thread_info_map.begin();
		tid_ = it->first;
	}

	HANDLE ht = OpenThread(MAXIMUM_ALLOWED, FALSE, tid_);
	if (!ht)
	{
		return false;
	}
	std::shared_ptr<void> ht_closer(ht, CloseHandle);

	CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_ALL;
	if (!::GetThreadContext(ht, &ctx))
	{
		return false;
	}

#ifndef _WIN64
	context->rax = ctx.Eax;
	context->rbx = ctx.Ebx;
	context->rcx = ctx.Ecx;
	context->rdx = ctx.Edx;

	context->rdi = ctx.Edi;
	context->rsi = ctx.Esi;

	context->rbp = ctx.Ebp;
	context->rsp = ctx.Esp;

	context->rip = ctx.Eip;
#else
	context->rax = ctx.Rax;
	context->rbx = ctx.Rbx;
	context->rcx = ctx.Rcx;
	context->rdx = ctx.Rdx;

	context->rdi = ctx.Rdi;
	context->rsi = ctx.Rsi;

	context->rbp = ctx.Rbp;
	context->rsp = ctx.Rsp;

	context->rip = ctx.Rip;

	context->r8 = ctx.R8;
	context->r9 = ctx.R9;
	context->r10 = ctx.R10;
	context->r11 = ctx.R11;
	context->r12 = ctx.R12;
	context->r13 = ctx.R13;
	context->r14 = ctx.R14;
	context->r15 = ctx.R15;
#endif
	context->efl = ctx.EFlags;
	context->cs = ctx.SegCs;
	context->ds = ctx.SegDs;
	context->es = ctx.SegEs;
	context->fs = ctx.SegFs;
	context->gs = ctx.SegGs;
	context->ss = ctx.SegSs;

	context->dr0 = ctx.Dr0;
	context->dr1 = ctx.Dr1;
	context->dr2 = ctx.Dr2;
	context->dr3 = ctx.Dr3;
	context->dr6 = ctx.Dr6;
	context->dr7 = ctx.Dr7;

	return true;
}

bool WindowsApi::SetThreadContext(xdv::architecture::x86::context::type *context)
{
	return false;
}

bool WindowsApi::SuspendThread(unsigned long tid)
{
	HANDLE ht = OpenThread(MAXIMUM_ALLOWED, FALSE, tid);
	if (!ht)
	{
		return false;
	}
	std::shared_ptr<void> ht_closer(ht, CloseHandle);

	unsigned long c = ::SuspendThread(ht);
	return true;
}

bool WindowsApi::ResumeThread(unsigned long tid)
{
	HANDLE ht = OpenThread(MAXIMUM_ALLOWED, FALSE, tid);
	if (!ht)
	{
		return false;
	}
	std::shared_ptr<void> ht_closer(ht, CloseHandle);

	::ResumeThread(ht);
	return true;
}

std::string WindowsApi::Module(unsigned long long ptr)
{
	HMODULE h_modules[1024];
	DWORD n;
	if (EnumProcessModules(process_handle_, h_modules, sizeof(h_modules), &n))
	{
		SIZE_T number_of_module = n / sizeof(HMODULE);
		LPMODULEINFO module_info = (LPMODULEINFO)malloc(sizeof(MODULEINFO) * number_of_module);
		ZeroMemory(module_info, sizeof(MODULEINFO) * number_of_module);
		std::shared_ptr<void> mi_closer(module_info, free);

		for (unsigned int i = 0; i < number_of_module; ++i)
		{
			if (!GetModuleInformation(process_handle_, h_modules[i], &module_info[i], sizeof(MODULEINFO)))
			{
				continue;
			}

			unsigned long long base = (unsigned long long)module_info[i].lpBaseOfDll;
			unsigned long long end = base + module_info[i].SizeOfImage;
			if (base <= ptr && end >= ptr)
			{
				wchar_t wn[100] = { 0, };
				if (GetModuleBaseName(process_handle_, (HMODULE)base, wn, sizeof(wn)))
				{
					std::wstring ws = wn;
					return std::string(ws.begin(), ws.end());
				}
			}
		}
	}

	return "";
}

#define MAX_SYMBOL_NAME		sizeof(IMAGEHLP_MODULE64) + (sizeof(SYMBOL_INFO) + 128 * 2)
unsigned long WindowsApi::Symbol(unsigned long long ptr, unsigned long long *disp, char *symbol_str, unsigned long symbol_size)
{
	IMAGEHLP_MODULE64 module_info = { 0, };
	CHAR buffer[sizeof(SYMBOL_INFO) + 128 * 2] = { 0, };
	PSYMBOL_INFO symbol_info = (PSYMBOL_INFO)buffer;

	char extra[MAX_SYMBOL_NAME] = { 0, };
	module_info.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
	symbol_info->SizeOfStruct = sizeof(SYMBOL_INFO);
	symbol_info->MaxNameLen = 128;

	if (SymGetModuleInfo64(process_handle_, ptr, &module_info))
	{
		StringCbCopyA(extra, MAX_SYMBOL_NAME, module_info.ModuleName);

		if (SymFromAddr(process_handle_, ptr, disp, symbol_info))
		{
			StringCbCatA(extra, MAX_SYMBOL_NAME, "!");
			StringCbCatA(extra, MAX_SYMBOL_NAME, symbol_info->Name);
		}
		else
		{
			*disp = ptr - module_info.BaseOfImage;
		}

		return 1;
	}

	return 0;
}

unsigned long WindowsApi::Symbol(unsigned long long ptr, char *symbol_str, unsigned long symbol_size)
{
	IMAGEHLP_MODULE64 module_info = { 0, };
	CHAR buffer[sizeof(SYMBOL_INFO) + 128 * 2] = { 0, };
	PSYMBOL_INFO symbol_info = (PSYMBOL_INFO)buffer;

	char extra[MAX_SYMBOL_NAME] = { 0, };
	module_info.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
	symbol_info->SizeOfStruct = sizeof(SYMBOL_INFO);
	symbol_info->MaxNameLen = 128;

	if (SymGetModuleInfo64(process_handle_, ptr, &module_info))
	{
		StringCbCopyA(extra, MAX_SYMBOL_NAME, module_info.ModuleName);

		unsigned long long disp = 0;
		if (SymFromAddr(process_handle_, ptr, &disp, symbol_info))
		{
			StringCbCatA(extra, MAX_SYMBOL_NAME, "!");
			StringCbCatA(extra, MAX_SYMBOL_NAME, symbol_info->Name);

			if (disp)
			{
				sprintf_s(symbol_str, symbol_size, "%s+0x%I64x", extra, disp);
			}
			else
			{
				sprintf_s(symbol_str, symbol_size, "%s", extra);
			}
		}
		else
		{
			disp = ptr - module_info.BaseOfImage;
			if (disp)
			{
				sprintf_s(symbol_str, symbol_size, "%s+0x%I64x", module_info.ModuleName, disp);
			}
			else
			{
				sprintf_s(symbol_str, symbol_size, "%s", module_info.ModuleName);
			}
		}

		return 1;
	}

	return 0;
}

unsigned long long WindowsApi::SymbolToPtr(char *symbol_str)
{
	char memory[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
	PSYMBOL_INFO sym = reinterpret_cast<PSYMBOL_INFO>(memory);
	sym->NameLen = MAX_SYM_NAME;
	sym->SizeOfStruct = sizeof(SYMBOL_INFO);
	if (SymFromName(process_handle_, symbol_str, sym))
	{
		return sym->Address;
	}

	return 0;
}

bool WindowsApi::StackTrace(xdv::architecture::x86::frame::type *stack_frame, size_t size_of_stack_frame, unsigned long *stack_count)
{
	HANDLE th = OpenThread(MAXIMUM_ALLOWED, FALSE, tid_);
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

	int i = 0;
	int cnt = (int)(size_of_stack_frame / sizeof(xdv::architecture::x86::frame::type));
	for (i = 0; i < cnt; ++i)
	{
		STACKFRAME64 debug_stack_frame;
		debug_stack_frame.AddrPC.Mode = AddrModeFlat;
		debug_stack_frame.AddrStack.Mode = AddrModeFlat;
		debug_stack_frame.AddrFrame.Mode = AddrModeFlat;
#ifdef _WIN64
		debug_stack_frame.AddrPC.Offset = ctx.Rip;
		debug_stack_frame.AddrStack.Offset = ctx.Rsp;
		debug_stack_frame.AddrFrame.Offset = ctx.Rbp;

		if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, process_handle_, th, &debug_stack_frame, &ctx, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
		{
			break;
		}
#else
		debug_stack_frame.AddrPC.Offset = ctx.Eip;
		debug_stack_frame.AddrStack.Offset = ctx.Esp;
		debug_stack_frame.AddrFrame.Offset = ctx.Ebp;

		if (!StackWalk64(IMAGE_FILE_MACHINE_I386, process_handle_, th, &debug_stack_frame, &ctx, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
		{
			break;
		}
#endif

		stack_frame[i].frame_number = i;
		stack_frame[i].frame_offset = debug_stack_frame.AddrFrame.Offset;
		stack_frame[i].func_table_entry = (unsigned long long)debug_stack_frame.FuncTableEntry;
		stack_frame[i].instruction_offset = debug_stack_frame.AddrPC.Offset;
		stack_frame[i].return_offset = debug_stack_frame.AddrReturn.Offset;
		stack_frame[i].stack_offset = debug_stack_frame.AddrStack.Offset;
		stack_frame[i].bool_virtual = debug_stack_frame.Virtual;

		for (int j = 0; j < 4; ++j)
		{
			stack_frame[i].params[j] = debug_stack_frame.Params[j];
		}

		for (int j = 0; j < 3; ++j)
		{
			stack_frame[i].reserved[j] = debug_stack_frame.Reserved[j];
		}
	}

	if (i)
	{
		*stack_count = i;
		return true;
	}

	return false;
}

bool WindowsApi::StackTraceEx(unsigned long long bp, unsigned long long ip, unsigned long long sp, xdv::architecture::x86::frame::type *stack_frame, size_t size_of_stack_frame, unsigned long *stack_count)
{
	HANDLE th = OpenThread(MAXIMUM_ALLOWED, FALSE, tid_);
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

	int i = 0;
	int cnt = (int)(size_of_stack_frame / sizeof(xdv::architecture::x86::frame::type));
	for (i = 0; i < cnt; ++i)
	{
		STACKFRAME64 debug_stack_frame;
		debug_stack_frame.AddrPC.Mode = AddrModeFlat;
		debug_stack_frame.AddrStack.Mode = AddrModeFlat;
		debug_stack_frame.AddrFrame.Mode = AddrModeFlat;
#ifdef _WIN64
		debug_stack_frame.AddrPC.Offset = ip;
		debug_stack_frame.AddrStack.Offset = sp;
		debug_stack_frame.AddrFrame.Offset = bp;

		if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, process_handle_, th, &debug_stack_frame, &ctx, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
		{
			break;
		}
#else
		debug_stack_frame.AddrPC.Offset = ip;
		debug_stack_frame.AddrStack.Offset = sp;
		debug_stack_frame.AddrFrame.Offset = bp;

		if (!StackWalk64(IMAGE_FILE_MACHINE_I386, process_handle_, th, &debug_stack_frame, &ctx, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
		{
			break;
		}
#endif

		stack_frame[i].frame_number = i;
		stack_frame[i].frame_offset = debug_stack_frame.AddrFrame.Offset;
		stack_frame[i].func_table_entry = (unsigned long long)debug_stack_frame.FuncTableEntry;
		stack_frame[i].instruction_offset = debug_stack_frame.AddrPC.Offset;
		stack_frame[i].return_offset = debug_stack_frame.AddrReturn.Offset;
		stack_frame[i].stack_offset = debug_stack_frame.AddrStack.Offset;
		stack_frame[i].bool_virtual = debug_stack_frame.Virtual;

		for (int j = 0; j < 4; ++j)
		{
			stack_frame[i].params[j] = debug_stack_frame.Params[j];
		}

		for (int j = 0; j < 3; ++j)
		{
			stack_frame[i].reserved[j] = debug_stack_frame.Reserved[j];
		}
	}

	if (i)
	{
		*stack_count = i;
		return true;
	}

	return false;
}

unsigned long long WindowsApi::GetPebAddress()
{
	return 0;
}

unsigned long long WindowsApi::GetTebAddress()
{
	return 0;
}

bool WindowsApi::StepInto(DebugCallbackT callback, void * cb_ctx)
{
	debuggee_status_ = DebuggeeStatusId::DEBUGGEE_STATUS_STEP_INTO;
	XdvReturnEvent();

	return true;
}

bool WindowsApi::StepOver(DebugCallbackT callback, void * cb_ctx)
{
	debuggee_status_ = DebuggeeStatusId::DEBUGGEE_STATUS_STEP_OVER;
	XdvReturnEvent();

	return true;
}

bool WindowsApi::RunningProcess()
{
	debuggee_status_ = DebuggeeStatusId::DEBUGGEE_STATUS_EXECUTION;
	XdvReturnEvent();
	return true;
}

unsigned char * WindowsApi::GetBpBackupDump(unsigned long long ptr)
{
	std::map<unsigned long long, break_point_ptr>::iterator it = break_point_map_.find(ptr);
	if (it != break_point_map_.end())
	{
		return it->second->bytes;
	}

	return nullptr;
}

bool WindowsApi::SetBreakPoint(DebugBreakPointId id, unsigned long long ptr)
{
	switch (id)
	{
	case DebugBreakPointId::SUSPEND_BREAK_POINT_ID:
		return this->InstallSuspendBreakPoint(ptr);

	case DebugBreakPointId::SOFTWARE_BREAK_POINT_ID:
		return this->InstallSoftwareBreakPoint(ptr);

	case DebugBreakPointId::HARDWARE_BREAK_POINT_ID:
		return this->InstallHardwareBreakPoint(ptr);
	}

	return false;
}

DebugBreakPointId WindowsApi::GetBreakPointId(unsigned long long ptr)
{
	auto it = break_point_map_.find(ptr);
	if (it != break_point_map_.end())
	{
		return it->second->id;
	}

	return DebugBreakPointId::NO_BREAK_POINT_ID;
}

bool WindowsApi::RestoreBreakPoint(unsigned long long ptr)
{
	auto it = break_point_map_.find(ptr);
	if (it != break_point_map_.end())
	{
		if (it->second->id == DebugBreakPointId::HARDWARE_BREAK_POINT_ID)
		{
			std::map<unsigned long, unsigned long long> thread_map;
			Threads(thread_map);

			unsigned long cnt = 0;
			for (auto it : thread_map)
			{
				HANDLE thread_handle = OpenThread(MAXIMUM_ALLOWED, FALSE, it.first);
				if (!thread_handle)
				{
					return false;
				}
				std::shared_ptr<void> handle_closer(thread_handle, CloseHandle);

				CONTEXT ctx = { 0, };
				ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

				if (ctx.Dr0 == 0)
				{
					ctx.Dr0 = 0;
				}
				else if (ctx.Dr1 == 0)
				{
					ctx.Dr1 = 0;
				}
				else if (ctx.Dr2 == 0)
				{
					ctx.Dr2 = 0;
				}
				else if (ctx.Dr3 == 0)
				{
					ctx.Dr3 = 0;
				}
				ctx.Dr7 = (1 << 0) | (1 << 2) | (1 << 4) | (1 << 6);

				::SetThreadContext(thread_handle, &ctx);
			}
		}
		else if (XdvWriteMemory(XdvGetParserHandle(), (void *)ptr, it->second->bytes, 16))
		{
			return true;
		}
	}

	return false;
}

void WindowsApi::ReInstallBreakPoint(unsigned long long ptr)
{
	auto it = break_point_map_.find(ptr);
	if (it != break_point_map_.end())
	{
		DebugBreakPointId id = it->second->id;
		DeleteBreakPoint(ptr);
		SetBreakPoint(id, ptr);
	}
}

bool WindowsApi::DeleteBreakPoint(unsigned long long ptr)
{
	if (RestoreBreakPoint(ptr))
	{
		break_point_map_.erase(ptr);
		return true;
	}

	return false;
}

void WindowsApi::RestoreAllBreakPoint()
{
	for (auto it : break_point_map_)
	{
		RestoreBreakPoint(it.first);
	}
}

void WindowsApi::ReInstallAllBreakPoint()
{
	for (auto it : break_point_map_)
	{
		SetBreakPoint(it.second->id, it.first);
	}
}

std::vector<unsigned long long> WindowsApi::GetBreakPointList()
{
	std::vector<unsigned long long> v;
	for (auto it : break_point_map_)
	{
		v.push_back(it.first);
	}

	return v;
}

XENOM_ADD_INTERFACE()
{
	IObject * obj = __add_object(WindowsApi);
	if (obj)
	{
		return XdvGetHandleByObject(obj);
	}

	return 0;
}

//
bool InstallSuspendPoint(unsigned long long ptr)
{
	unsigned char lp[3] = { 0x90, 0xEB, 0xFD };
	if (XdvWriteMemory(XdvGetParserHandle(), (void *)ptr, lp, 3))
	{
		return true;
	}

	return false;
}

void SuspendCallback(unsigned long long ptr);
bool WindowsApi::InstallSuspendBreakPoint(unsigned long long ptr)
{
	break_point_ptr bp_ptr = new break_point;
	bp_ptr->id = DebugBreakPointId::SUSPEND_BREAK_POINT_ID;
	if (XdvReadMemory(XdvGetParserHandle(), ptr, bp_ptr->bytes, 16) == 0)
	{
		return false;
	}

	if (InstallSuspendPoint(ptr))
	{
		break_point_map_.insert(std::pair<unsigned long long, break_point_ptr>(ptr, bp_ptr));
		std::thread * bp = new std::thread(SuspendCallback, ptr);
		return true;
	}

	return false;
}

//
bool InstallSoftwarePoint(unsigned long long ptr)
{
	unsigned char lp[1] = { 0xcc };
	if (XdvWriteMemory(XdvGetParserHandle(), (void *)ptr, lp, 1))
	{
		return true;
	}

	return false;
}

bool WindowsApi::InstallSoftwareBreakPoint(unsigned long long ptr)
{
	break_point_ptr bp_ptr = new break_point;
	bp_ptr->id = DebugBreakPointId::SOFTWARE_BREAK_POINT_ID;
	if (XdvReadMemory(XdvGetParserHandle(), ptr, bp_ptr->bytes, 16) == 0)
	{
		return false;
	}

	if (InstallSoftwarePoint(ptr))
	{
		break_point_map_.insert(std::pair<unsigned long long, break_point_ptr>(ptr, bp_ptr));
		return true;
	}

	return false;
}

//
bool WindowsApi::InstallHardwareBreakPoint(unsigned long long ptr)
{
	break_point_ptr bp_ptr = new break_point;
	bp_ptr->id = DebugBreakPointId::HARDWARE_BREAK_POINT_ID;
	if (XdvReadMemory(XdvGetParserHandle(), ptr, bp_ptr->bytes, 16) == 0)
	{
		return false;
	}

	std::map<unsigned long, unsigned long long> thread_map;
	Threads(thread_map);

	unsigned long cnt = 0;
	for (auto it : thread_map)
	{
		HANDLE thread_handle = OpenThread(MAXIMUM_ALLOWED, FALSE, it.first);
		if (!thread_handle)
		{
			return false;
		}
		std::shared_ptr<void> handle_closer(thread_handle, CloseHandle);

		CONTEXT ctx = { 0, };
		ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		if (!::GetThreadContext(thread_handle, &ctx))
		{
			return false;
		}

#ifdef _WIN64
		if (ctx.Dr0 == 0)
		{
			ctx.Dr0 = ptr;
		}
		else if (ctx.Dr1 == 0)
		{
			ctx.Dr1 = ptr;
		}
		else if (ctx.Dr2 == 0)
		{
			ctx.Dr2 = ptr;
		}
		else if (ctx.Dr3 == 0)
		{
			ctx.Dr3 = ptr;
		}
		else
		{
			ctx.Dr0 = ptr;
		}
#else
		if (ctx.Dr0 == 0)
		{
			ctx.Dr0 = (unsigned long)ptr;
		}
		else if (ctx.Dr1 == 0)
		{
			ctx.Dr1 = (unsigned long)ptr;
		}
		else if (ctx.Dr2 == 0)
		{
			ctx.Dr2 = (unsigned long)ptr;
		}
		else if (ctx.Dr3 == 0)
		{
			ctx.Dr3 = (unsigned long)ptr;
		}
		else
		{
			ctx.Dr0 = (unsigned long)ptr;
		}
#endif
		ctx.Dr7 = (1 << 0) | (1 << 2) | (1 << 4) | (1 << 6);
		if (::SetThreadContext(thread_handle, &ctx))
		{
			++cnt;
		}
	}

	if (cnt)
	{
		break_point_map_.insert(std::pair<unsigned long long, break_point_ptr>(ptr, bp_ptr));
		return true;
	}

	return false;
}

void WindowsApi::SetStepOverPtr(unsigned long long ptr)
{
	step_over_ptr_ = ptr;
}

unsigned long long WindowsApi::GetStepOverPtr()
{
	return step_over_ptr_;
}

DebuggeeStatusId WindowsApi::GetDebugStatus()
{
	return debuggee_status_;
}

void WindowsApi::SetDebugStatus(DebuggeeStatusId id)
{
	debuggee_status_ = id;
}