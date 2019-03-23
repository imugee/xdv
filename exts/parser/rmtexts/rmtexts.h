#ifndef __DEFINE_REMOTE_API_EXTS_HEADER__
#define __DEFINE_REMOTE_API_EXTS_HEADER__

#include "xdv_sdk.h"

#include <windows.h>
#include <winternl.h>
#include <DbgHelp.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <strsafe.h>
#include <stdio.h>
#include <conio.h>

class RemoteApi : public IDebugger
{
private:
	IDebugger * debugger_;
	unsigned long long step_over_ptr_;

private:
	void ResumeProcess();
	void SetBreakPointTable();

public:
	void SetStepOverPtr(unsigned long long ptr);
	unsigned long long GetStepOverPtr();

public:
	RemoteApi();
	~RemoteApi();

public:
	virtual xdv::object::id ObjectType();
	virtual std::string ObjectString();
	virtual void SetModuleName(std::string module);
	virtual std::string ModuleName();

public:
	virtual std::map<unsigned long, std::string> ProcessList();
	virtual unsigned long WaitForProcess(std::string process_name);

	//
	virtual bool Attach(unsigned long pid);
	virtual bool Open(char *path);
	virtual bool Open(unsigned long pid);
	virtual bool Update();

	virtual unsigned long ProcessId();

	//
	virtual unsigned long long Read(unsigned long long ptr, unsigned char *out_memory, unsigned long read_size);
	virtual unsigned long long Write(void * ptr, unsigned char *input_memory, unsigned long write_size);
	virtual bool Query(unsigned long long ptr, xdv::memory::type *memory_type);
	virtual void * Alloc(void *ptr, unsigned long size, unsigned long allocation_type, unsigned long protect_type);

	//
	virtual bool Select(unsigned long tid);
	virtual void Threads(std::map<unsigned long, unsigned long long> &thread_info_map);

	virtual bool GetThreadContext(xdv::architecture::x86::context::type *context);
	virtual bool SetThreadContext(xdv::architecture::x86::context::type *context);

	virtual bool SuspendThread(unsigned long tid);
	virtual bool ResumeThread(unsigned long tid);

	//
	virtual std::string Module(unsigned long long ptr);
	virtual unsigned long Symbol(unsigned long long ptr, unsigned long long *disp, char *symbol_str, unsigned long symbol_size);
	virtual unsigned long Symbol(unsigned long long ptr, char *symbol_str, unsigned long symbol_size);
	virtual unsigned long long SymbolToPtr(char *symbol_str);

	//
	virtual bool StackTrace(xdv::architecture::x86::frame::type *stack_frame, size_t size_of_stack_frame, unsigned long *stack_count);
	virtual bool StackTraceEx(unsigned long long bp, unsigned long long ip, unsigned long long sp, xdv::architecture::x86::frame::type *stack_frame, size_t size_of_stack_frame, unsigned long *stack_count);

	//
	virtual unsigned long long GetPebAddress();
	virtual unsigned long long GetTebAddress();

	virtual bool StepInto(DebugCallbackT callback, void * cb_ctx);
	virtual bool StepOver(DebugCallbackT callback, void * cb_ctx);
	virtual bool RunningProcess();

	//
	virtual unsigned char * GetBpBackupDump(unsigned long long ptr);
	virtual DebugBreakPointId GetBreakPointId(unsigned long long ptr);
	virtual std::vector<unsigned long long> GetBreakPointList();
	virtual bool SetBreakPoint(DebugBreakPointId id, unsigned long long ptr);

	virtual bool RestoreBreakPoint(unsigned long long ptr);
	virtual void ReInstallBreakPoint(unsigned long long ptr);
	virtual bool DeleteBreakPoint(unsigned long long ptr);

	virtual void RestoreAllBreakPoint();
	virtual void ReInstallAllBreakPoint();
};

#endif