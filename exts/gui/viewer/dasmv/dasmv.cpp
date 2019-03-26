#include "xdv_sdk.h"

#pragma comment(lib, "corexts.lib")

unsigned long long _current_ptr = 0;
xdv_handle _current_handle;
XENOM_ADD_INTERFACE()
{
	xvar var = XdvExe("!qxnm.addv -name:Disassemble -title:code -type:event -callback:!dasmv.cbdasmv");
	_current_handle = handlevar(var);
	XdvExe("!qxnm.chkable -handle:%x", _current_handle);

	return _current_handle;
}

// ------------------------------------------------------
//
std::multimap<unsigned long long, unsigned long long> * _ref_map_ptr = nullptr;
std::map<unsigned long long, xdv::architecture::x86::block::id> * _proc_map_ptr = nullptr;
std::vector<unsigned long long> _trace_vector;
std::set<unsigned long long> _break_point_set;
void NavigationString(unsigned long long ptr, std::string &str)
{
	if (!_ref_map_ptr)
	{
		xvar var = XdvExe("!procv.reftable");
		_ref_map_ptr
			= (std::multimap<unsigned long long, unsigned long long> *)ptrvar(var);
	}

	if (!_proc_map_ptr)
	{
		xvar var = XdvExe("!procv.proctable");
		_proc_map_ptr
			= (std::map<unsigned long long, xdv::architecture::x86::block::id> *)ptrvar(var);
	}

	xdv_handle ah = XdvGetArchitectureHandle();
	xdv_handle ih = XdvGetParserHandle();

	std::set<unsigned long long>::iterator bi = _break_point_set.find(ptr);
	std::multimap<unsigned long long, unsigned long long>::iterator ri = _ref_map_ptr->find(ptr);
	std::map<unsigned long long, xdv::architecture::x86::block::id>::iterator pi = _proc_map_ptr->find(ptr);
	if (ri != _ref_map_ptr->end())
	{
		char symbol[256] = { 0, };
		if (XdvGetSymbolString(ih, ptr, symbol, sizeof(symbol)))
		{
			if (pi != _proc_map_ptr->end())
			{
				switch (pi->second)
				{
				case xdv::architecture::x86::block::id::X86_CODE_BLOCK:
					str += "\n; =========================== subroutine ===========================";
					break;

				default:
					break;
				}
			}

			str += "\n; .sym : ";
			str += symbol;
		}

		unsigned char dump[16] = { 0, };
		unsigned long long readn = XdvReadMemory(ih, ri->second, dump, sizeof(dump));
		if (readn == 0)
		{
			return;
		}

		str += "\n; .xref ";

		std::pair<std::multimap<unsigned long long, unsigned long long>::iterator
			, std::multimap<unsigned long long, unsigned long long>::iterator> proc_table = _ref_map_ptr->equal_range(ptr);
		std::multimap<unsigned long long, unsigned long long>::iterator range_it = proc_table.first;
		if (proc_table.first != proc_table.second)
		{
			str += "\n; ";
		}
		else
		{
			str += "\n";
		}

		int i = 0;
		for (range_it; range_it != proc_table.second; ++range_it, ++i)
		{
			if (i == 16)
			{
				str += "\n; ";
				i = 0;
			}

			char xref_str[500] = { 0, };
			sprintf_s(xref_str, sizeof(xref_str), "0x%I64x ", range_it->second);
			str += xref_str;
		}

		str += "\n";
	}
	else if (pi != _proc_map_ptr->end())
	{
		char symbol[256] = { 0, };
		if (!XdvGetSymbolString(ih, ptr, symbol, sizeof(symbol)))
		{
			sprintf_s(symbol, sizeof(symbol), "unknown");
		}

		switch (pi->second)
		{
		case xdv::architecture::x86::block::id::X86_CODE_BLOCK:
			str += "\n; =========================== subroutine ===========================";
			break;

		default:
			break;
		}

		str += "\n; .sym : ";
		str += symbol;
		str += "\n";
	}

	xdv::architecture::x86::context::type *pctx = (xdv::architecture::x86::context::type *)ptrvar(XdvExe("!cpuv.getctx"));
	if (ptr && pctx && pctx->rip == ptr)
	{
		str += "; ======= current point\n";
	}
	//else if (XdvGetThreadContext(XdvGetParserHandle(), &ctx) // => cpuv에서 값을 가져와야함
	//	&& ctx.rip == ptr)
	//{
	//	str += " ; ======= current point\n";
	//}
	else if (bi != _break_point_set.end())
	{
		str += "; ======= break point\n";
	}
}

// ------------------------------------------------------
//
unsigned long long CodeAndRemarkString(unsigned long long ptr, std::string &str)
{
	xdv_handle ah = XdvGetArchitectureHandle();
	xdv_handle ih = XdvGetParserHandle();

	unsigned char * dump = nullptr;
	unsigned char cdump[16] = { 0, };
	unsigned long long readn = XdvReadMemory(ih, ptr, cdump, sizeof(cdump));
	if (readn == 0)
	{
		return 0;
	}

	std::set<unsigned long long>::iterator bi = _break_point_set.find(ptr);
	if (bi != _break_point_set.end())
	{
		dump = XdvGetBpBackupDump(ih, ptr);
	}
	else
	{
		dump = cdump;
	}

	//
	// assemble mn & symbol & string
	char mn[200] = { 0, };
	unsigned long long r = XdvDisassemble(ah, ptr, dump, mn, sizeof(mn));
	if (r == 0)
	{
		return 0;
	}

	//
	//
	char asm_mn[3072] = { 0, };
	sprintf_s(asm_mn, sizeof(asm_mn), "%s", mn);

	//
	// remark
	unsigned long align = (unsigned long)(100 - strlen(mn));
	std::vector<unsigned long long> ov;
	bool ovr = XdvGetOperandValues(ah, ih, ptr, dump, ov);

	if (ov.size())
	{
		for (size_t i = 0; i < ov.size(); ++i)
		{
			unsigned char str[1024] = { 0, };
			readn = XdvReadMemory(ih, ov[i], str, sizeof(str));
			if (readn == 0)
			{
				continue;
			}

			char symbol[1000] = { 0, };
			memset(asm_mn, 0, sizeof(asm_mn));
			if (XdvGetSymbolString(ih, ov[i], symbol, sizeof(symbol)))
			{
				sprintf_s(asm_mn, sizeof(asm_mn), "%s%*c; 0x%I64x, %s", mn, align, ' ', ov[0], symbol);
			}
			else
			{
				sprintf_s(asm_mn, sizeof(asm_mn), "%s%*c; 0x%I64x, %s", mn, align, ' ', ov[0], "<unknown>");
			}

			if (!ovr)
			{
				std::string ascii;
				if (XdvIsAscii(str, sizeof(str), ascii))
				{
					memset(asm_mn, 0, sizeof(asm_mn));
					sprintf_s(asm_mn, sizeof(asm_mn), "%s%*c; \"%s\"", mn, align, ' ', ascii.c_str());
				}

				std::string unicode;
				if (XdvIsUnicode(str, sizeof(str), unicode))
				{
					memset(asm_mn, 0, sizeof(asm_mn));
					sprintf_s(asm_mn, sizeof(asm_mn), "%s%*c; L\"%s\"", mn, align, ' ', unicode.c_str());
				}
			}
		}
	}

	//
	// add str
	str += asm_mn;

	//
	//
	bool is_jxx = false;
	if (XdvIsJumpCode(ah, ptr, dump, &is_jxx) || XdvIsRetCode(ah, ptr, dump))
	{
		unsigned long long next = ptr + r;
		std::multimap<unsigned long long, unsigned long long>::iterator ri = _ref_map_ptr->find(next);
		std::map<unsigned long long, xdv::architecture::x86::block::id>::iterator pi = _proc_map_ptr->find(next);

		if (ri == _ref_map_ptr->end() && pi == _proc_map_ptr->end())
		{
			str += "\n";
		}
	}

	return r;
}

EXTS_FUNC(codesize)	// argv[0] = ptr
{
	unsigned long long ptr = XdvToUll(argv, argc, "ptr");
	if (ptr)
	{
		std::string str;
		unsigned long long size = CodeAndRemarkString(ptr, str);

		return ullvar(size);
	}

	return ullvar(0);
}

EXTS_FUNC(navistr)	// argv[0] = ptr
{
	unsigned long long ptr = XdvToUll(argv, argc, "ptr");
	if (ptr == 0)
	{
		return nullvar();
	}

	char * nv = nullptr;
	if (ptr)
	{
		std::string str;
		NavigationString(ptr, str);
		if (str.size())
		{
			nv = (char *)malloc(str.size() + 10);
			if (nv)
			{
				memset(nv, 0, str.size() + 10);
				memcpy(nv, str.c_str(), str.size());
			}
		}
	}

	return ptrvar(nv);
}

EXTS_FUNC(codestr)	// argv[0] = ptr
{
	unsigned long long ptr = XdvToUll(argv, argc, "ptr");
	if (ptr == 0)
	{
		return nullvar();
	}

	char * cstr = nullptr;
	if (ptr)
	{
		std::string str;
		CodeAndRemarkString(ptr, str);
		if (str.size())
		{
			cstr = (char *)malloc(str.size());
			if (cstr)
			{
				memset(cstr, 0, str.size() + 10);
				memcpy(cstr, str.c_str(), str.size());
			}
		}
	}

	return ptrvar(cstr);
}


// ------------------------------------------------------
//
unsigned long long Disassemble(unsigned long long ptr, std::string &str)
{
	//
	// print code navigation
	NavigationString(ptr, str);
	return CodeAndRemarkString(ptr, str);
}

// ------------------------------------------------------
//
EXTS_FUNC(dasmptr)	// argv[0] = status
{
	return ullvar(_current_ptr);
}

EXTS_FUNC(dasm)	// argv[0] = ptr
{
	unsigned long long ptr = XdvToUll(argv, argc, "ptr");
	if (ptr == 0)
	{
		ptr = _current_ptr;
	}
	else
	{
		_current_ptr = ptr;
	}

	std::string str;
	unsigned char dump[16] = { 0, };
	unsigned long long read = XdvReadMemory(XdvGetParserHandle(), ptr, dump, sizeof(dump));
	if (read == 0)
	{
		return nullvar();
	}

	XdvExe("!procv.analyze -ptr:%I64x", _current_ptr);
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

	XdvPrintAndClear(_current_handle, str.c_str(), false);
	return nullvar();
}

void Update()
{
	XdvExe("!procv.analyze -ptr:%I64x", _current_ptr);

	std::string str;
	unsigned long long ptr = _current_ptr;
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

	XdvPrintAndClear(_current_handle, str.c_str(), false);
}

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
						result = true;
						break;
					}
				}
			}
		}
		//XdvResumeProcess(XdvGetParserHandle());

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

	if (XdvGetThreadContext(XdvGetParserHandle(), &ctx))
	{
		XdvExe("!cpuv.printctx");

		XdvExe("!dasmv.dasm -ptr:%I64x", ctx.rip);
		XdvExe("!hexv.hex -ptr:%I64x", ctx.rip);
		XdvExe("!thrdv.threads");
		XdvExe("!stackv.printframe");
	}

	_break_point_set.erase(ptr);
	XdvDeleteBreakPoint(XdvGetParserHandle(), ptr);
}

// ------------------------------------------------------
//
void TraceFromEntryPoint(unsigned long long ptr);
void TraceFromCurrentPoint(unsigned long long ptr);
void TrackTrace(unsigned long long ptr);
void FindReferenceString(unsigned long long ptr);
void FindIntermodularCall(unsigned long long ptr);
EXTS_FUNC(cbdasmv)	// argv = status
					// argv = string
{
	char * status = XdvValue(argv, argc, "status", nullptr);
	char * handle = XdvValue(argv, argc, "handle", nullptr);
	if (strstr(status, "up"))
	{
		_current_ptr -= 3;
		Update();
	}
	else if (strstr(status, "down"))
	{
		_current_ptr += 3;
		Update();
	}
	else if (strstr(status, "pre"))
	{
		unsigned long long ptr = XdvToUll(argv, argc, "str");
		if (ptr)
		{
			unsigned char dump[16] = { 0, };
			if (XdvReadMemory(XdvGetParserHandle(), ptr, dump, sizeof(dump)))
			{
				_trace_vector.push_back(ptr);
			}
		}
	}
	else if (strstr(status, "dc"))
	{
		char * ptr_str = XdvValue(argv, argc, "str", nullptr);
		if (strlen(ptr_str))
		{
			ptr_str = strstr(ptr_str, "0x");
			if (ptr_str)
			{
				XdvExe("!dasmv.dasm -ptr:%s", ptr_str);
			}
		}
	}
	else if (strstr(status, "backspace"))
	{
		unsigned long long ptr = _trace_vector[_trace_vector.size() - 1];
		if (ptr)
		{
			XdvExe("!dasmv.dasm -ptr:%I64x", ptr);
		}
	}
	else if (strstr(status, "space"))
	{
		char * ptr_str = XdvValue(argv, argc, "str", nullptr);
		ptr_str = strstr(ptr_str, "0x");
		if (ptr_str)
		{
			XdvExe("!dasmv.dasm -ptr:%s", ptr_str);
			Update();
		}
	}
	// ------------------------------------------------------
	// context menu
	else if (strstr(status, "Jump"))
	{
		xvar var = XdvExe("!qxnm.gotodialog");
		unsigned long long ptr = ullvar(var);
		if (ptr)
		{
			XdvExe("!dasmv.dasm -ptr:%I64x", ptr);
		}
	}
	else if (strstr(status, "Trace from the entry point"))
	{
		unsigned long long ptr = XdvToUll(argv, argc, "tag");
		if (ptr)
		{
			TraceFromEntryPoint(ptr);
		}
	}
	else if (strstr(status, "Trace from the current point"))
	{
		unsigned long long ptr = XdvToUll(argv, argc, "tag");
		if (ptr)
		{
			TraceFromCurrentPoint(ptr);
		}
	}
	else if (strstr(status,"Track trace"))
	{
		unsigned long long ptr = XdvToUll(argv, argc, "tag");
		if (ptr)
		{
			TrackTrace(ptr);
		}
	}
	else if (strstr(status, "Find reference string"))
	{
		unsigned long long ptr = XdvToUll(argv, argc, "tag");
		if (ptr)
		{
			FindReferenceString(ptr);
		}
	}
	else if (strstr(status, "Find intermodular call"))
	{
		unsigned long long ptr = XdvToUll(argv, argc, "tag");
		if (ptr)
		{
			FindIntermodularCall(ptr);
		}
	}
	else if (strstr(status, "Suspend point"))
	{
		unsigned long long ptr = XdvToUll(argv, argc, "tag");
		if (ptr)
		{
			if (XdvSetBreakPoint(XdvGetParserHandle(), DebugBreakPointId::SUSPEND_BREAK_POINT_ID, ptr))
			{
				_break_point_set.insert(ptr);
				Update();
				std::thread * bp = new std::thread(SuspendCallback, ptr);
			}
		}
	}
	else if (strstr(status, "Software break point"))
	{
		unsigned long long ptr = XdvToUll(argv, argc, "tag");
		if (ptr)
		{
			XdvSuspendProcess(XdvGetParserHandle());
			if (XdvSetBreakPoint(XdvGetParserHandle(), DebugBreakPointId::SOWFTWARE_BREAK_POINT_ID, ptr))
			{
				_break_point_set.insert(ptr);
				Update();
			}
			XdvResumeProcess(XdvGetParserHandle());
		}
	}
	else if (strstr(status, "Hardware break point"))
	{
		unsigned long long ptr = XdvToUll(argv, argc, "tag");
		if (ptr)
		{
			if (XdvSetBreakPoint(XdvGetParserHandle(), DebugBreakPointId::HARDWARE_BREAK_POINT_ID, ptr))
			{
				_break_point_set.insert(ptr);
				Update();
			}
		}
	}
	else if (strstr(status, "Delete break point"))
	{
		unsigned long long ptr = XdvToUll(argv, argc, "tag");
		if (ptr)
		{
			if (XdvDeleteBreakPoint(XdvGetParserHandle(), ptr))
			{
				_break_point_set.erase(ptr);
				Update();
			}
		}
	}

	return nullvar();
}

EXTS_FUNC(update) // first callback
{
	Update();

	XdvExe("!qxnm.ctxmenu -handle:%x -name:Jump -key:%x -nco:Goto.ico", _current_handle, xdv::key::id::Key_G);

	XdvExe("!qxnm.ctxmenu -handle:%x -menu:Tag -name:Trace from the entry point -key:%x", _current_handle, xdv::key::id::Key_E);
	XdvExe("!qxnm.ctxmenu -handle:%x -menu:Tag -name:Trace from the current point -key:%x", _current_handle, xdv::key::id::Key_P);
	XdvExe("!qxnm.ctxmenu -handle:%x -menu:Tag -name:Track trace -key:%x", _current_handle, xdv::key::id::Key_T);

	XdvExe("!qxnm.ctxmenu -handle:%x -menu:Find -name:Find reference string -nco:FindString.ico", _current_handle);
	XdvExe("!qxnm.ctxmenu -handle:%x -menu:Find -name:Find intermodular call -nco:FIndProc.ico", _current_handle);

	XdvExe("!qxnm.ctxmenu -handle:%x -menu:Break point -name:Suspend point -nco:SuspendBreakPoint.ico", _current_handle);
	XdvExe("!qxnm.ctxmenu -handle:%x -menu:Break point -name:Software break point -nco:SoftwareBreakPoint.ico", _current_handle);
	XdvExe("!qxnm.ctxmenu -handle:%x -menu:Break point -name:Hardware break point -nco:HardwareBreakPoint.ico", _current_handle);
	XdvExe("!qxnm.ctxmenu -handle:%x -menu:Break point -name:Delete break point -nco:DeleteBreakPoint.ico", _current_handle);

	return nullvar();
}

// ------------------------------------------------------
//
void TraceFromEntryPoint(unsigned long long ptr)
{
	xvar var = XdvExe("!qxnm.addv -name:Tag -title:tag %I64x -type:txta -callback:non", ptr);
	xdv_handle tag_handle = handlevar(var);
	XdvExe("!qxnm.chkable -handle:%x", tag_handle);

	IViewer *v = (IViewer *)XdvGetObjectByHandle(tag_handle);
	if (!v)
	{
		return;
	}

	xdv_handle ah = XdvGetArchitectureHandle();
	xdv_handle ih = XdvGetParserHandle();
	unsigned long long entry = XdvFindEntryPoint(ah, ih, ptr);
	if (entry != 0)
	{
		ptr = entry;
	}

	std::set<unsigned long long> trace_set;
	XdvAnalyze(ah, ih, ptr, trace_set);
	if (trace_set.size() == 0)
	{
		return;
	}

	std::set<unsigned long long>::iterator it = trace_set.begin();
	std::string dasm_str;
	for (it; it != trace_set.end(); ++it)
	{
		std::string str;
		Disassemble(*it, str);
		dasm_str += str;

		unsigned char dump[16] = { 0, };
		unsigned long long readn = XdvReadMemory(ih, *it, dump, sizeof(dump));
		if (readn == 0)
		{
			break;
		}

		if (!XdvIsRetCode(ah, *it, dump))
		{
			dasm_str += "\n";
		}
	}

	v->AddViewer();
	XdvPrintViewer(tag_handle, dasm_str);

	XdvExe("!qxnm.addtabv -va:%x -area:left -vb:%x", _current_handle, tag_handle);
	XdvExe("!qxnm.raise -handle:%x", tag_handle);
}

void TraceFromCurrentPoint(unsigned long long ptr)
{
	xvar var = XdvExe("!qxnm.addv -name:Tag -title:tag %I64x -type:txta -callback:non", ptr);
	xdv_handle tag_handle = handlevar(var);
	XdvExe("!qxnm.chkable -handle:%x", tag_handle);

	IViewer *v = (IViewer *)XdvGetObjectByHandle(tag_handle);
	if (!v)
	{
		return;
	}

	xdv_handle ah = XdvGetArchitectureHandle();
	xdv_handle ih = XdvGetParserHandle();
	std::set<unsigned long long> trace_set;
	XdvAnalyze(ah, ih, ptr, trace_set);
	if (trace_set.size() == 0)
	{
		return;
	}

	std::set<unsigned long long>::iterator it = trace_set.begin();
	std::string dasm_str;
	for (it; it != trace_set.end(); ++it)
	{
		std::string str;
		Disassemble(*it, str);
		dasm_str += str;

		unsigned char dump[16] = { 0, };
		unsigned long long readn = XdvReadMemory(ih, *it, dump, sizeof(dump));
		if (readn == 0)
		{
			break;
		}

		if (!XdvIsRetCode(ah, *it, dump))
		{
			dasm_str += "\n";
		}
	}

	v->AddViewer();
	XdvPrintViewer(tag_handle, dasm_str);

	XdvExe("!qxnm.addtabv -va:%x -area:left -vb:%x", _current_handle, tag_handle);
	XdvExe("!qxnm.raise -handle:%x", tag_handle);
}

void TrackTrace(unsigned long long ptr)
{
	xvar var = XdvExe("!qxnm.addv -name:Tag -title:tag %I64x -type:txta -callback:non", ptr);
	xdv_handle tag_handle = handlevar(var);
	XdvExe("!qxnm.chkable -handle:%x", tag_handle);

	IViewer *v = (IViewer *)XdvGetObjectByHandle(tag_handle);
	if (!v)
	{
		return;
	}

	xdv_handle ah = XdvGetArchitectureHandle();
	xdv_handle ih = XdvGetParserHandle();
	std::vector<unsigned long long> trace_vector;
	XdvAnalyze(ah, ih, ptr, trace_vector);
	if (trace_vector.size() == 0)
	{
		return;
	}

	std::string dasm_str;
	for (auto it : trace_vector)
	{
		std::string str;
		Disassemble(it, str);
		dasm_str += str;

		unsigned char dump[16] = { 0, };
		unsigned long long readn = XdvReadMemory(ih, it, dump, sizeof(dump));
		if (readn == 0)
		{
			break;
		}

		if (!XdvIsRetCode(ah, it, dump))
		{
			dasm_str += "\n";
		}
	}

	v->AddViewer();
	XdvPrintViewer(tag_handle, dasm_str);

	XdvExe("!qxnm.addtabv -va:%x -area:left -vb:%x", _current_handle, tag_handle);
	XdvExe("!qxnm.raise -handle:%x", tag_handle);
}

// ------------------------------------------------------
//
typedef struct _tag_ref_value_ctx
{
	xdv_handle handle;
	unsigned long long base;
	unsigned long long end;
}ref_value_ctx;

void FindReferenceString(IWorker *worker, unsigned long long base, unsigned long long end)
{
	xdv_handle ah = XdvGetArchitectureHandle();
	xdv_handle ih = XdvGetParserHandle();
	std::multimap<unsigned long long, unsigned long long> ref_map = *_ref_map_ptr;

	int i = 0;
	std::string str;
	std::multimap<unsigned long long, unsigned long long>::iterator it = ref_map.begin();
	for (it; it != ref_map.end(); ++it)
	{
		if (base < it->second && it->second < end)
		{
			unsigned char dump[16] = { 0, };
			unsigned long long readn = XdvReadMemory(ih, it->second, dump, sizeof(dump));
			if (readn == 0)
			{
				continue;
			}

			if (!XdvIsReadableCode(ah, it->second, dump))
			{
				continue;
			}

			std::vector<unsigned long long> ov;
			bool ovr = XdvGetOperandValues(ah, ih, it->second, dump, ov);
			if (!ovr && ov.size() >= 1)
			{
				for (size_t i = 0; i < ov.size(); ++i)
				{
					unsigned char str[1024] = { 0, };
					readn = XdvReadMemory(ih, ov[i], str, sizeof(str));
					if (readn == 0)
					{
						continue;
					}

					std::string asm_str;
					char mn[512] = { 0, };
					XdvDisassemble(ah, it->second, dump, mn, sizeof(mn));

					std::string ascii;
					if (XdvIsAscii(str, sizeof(str), ascii))
					{
						unsigned long align = (unsigned long)(100 - strlen(mn));
						char asm_mn[512] = { 0, };
						memset(asm_mn, 0, sizeof(asm_mn));
						sprintf_s(asm_mn, sizeof(asm_mn), " %s%*c; \"%s\"\n", mn, align, ' ', ascii.c_str());
						asm_str = asm_mn;
					}

					std::string unicode;
					if (XdvIsUnicode(str, sizeof(str), unicode))
					{
						unsigned long align = (unsigned long)(100 - strlen(mn));
						char asm_mn[512] = { 0, };
						memset(asm_mn, 0, sizeof(asm_mn));
						sprintf_s(asm_mn, sizeof(asm_mn), " %s%*c; L\"%s\"\n", mn, align, ' ', unicode.c_str());
						asm_str = asm_mn;
					}

					worker->InsertString(asm_str);
					worker->Update();
				}
			}
		}
	}
}

void PrintReferenceString(IWorker *worker, void *ctx)
{
	ref_value_ctx *rctx = (ref_value_ctx *)ctx;
	FindReferenceString(worker, rctx->base, rctx->end);
	free(ctx);
}

void FindReferenceString(unsigned long long ptr)
{
	xdv_handle ah = XdvGetArchitectureHandle();
	xdv_handle ih = XdvGetParserHandle();
	xdv::memory::type mbi;
	if (!XdvQueryMemory(ih, ptr, &mbi))
	{
		return;
	}

	xvar var = XdvExe("!qxnm.addv -name:Tag -title:Reference String %I64x-%I64x -type:txta -callback:non", mbi.BaseAddress, mbi.BaseAddress + mbi.RegionSize);
	xdv_handle tag_handle = handlevar(var);
	XdvExe("!qxnm.chkable -handle:%x", tag_handle);

	IViewer *v = (IViewer *)XdvGetObjectByHandle(tag_handle);
	if (v)
	{
		v->AddViewer();
		ref_value_ctx * ctx = new ref_value_ctx;
		ctx->base = mbi.BaseAddress;
		ctx->end = mbi.BaseAddress + mbi.RegionSize;
		ctx->handle = tag_handle;
		XdvRun(tag_handle, PrintReferenceString, ctx);

		XdvExe("!qxnm.addtabv -va:%x -area:left -vb:%x", _current_handle, tag_handle);
		XdvExe("!qxnm.raise -handle:%x", tag_handle);
	}	
}

// ------------------------------------------------------
//
void FindIntermodularCall(IWorker * worker, unsigned long long base, unsigned long long end)
{
	std::multimap<unsigned long long, unsigned long long> ref_map = *_ref_map_ptr;

	int i = 0;
	std::string str;
	std::multimap<unsigned long long, unsigned long long>::iterator it = ref_map.begin();
	for (it; it != ref_map.end(); ++it)
	{
		if (base < it->second && it->second < end)
		{
			xdv_handle ah = XdvGetArchitectureHandle();
			xdv_handle ih = XdvGetParserHandle();
			unsigned char dump[16] = { 0, };
			unsigned long long readn = XdvReadMemory(ih, it->second, dump, sizeof(dump));
			if (readn == 0)
			{
				continue;
			}

			unsigned char test[1000] = { 0, };
			if (XdvReadMemory(ih, it->first, test, sizeof(test)) == 0)
			{
				continue;
			}

			bool jxx = false;
			if (!(XdvIsJumpCode(ah, it->second, dump, &jxx) || XdvIsCallCode(ah, it->second, dump)))
			{
				continue;
			}

			char mn[200] = { 0, };
			unsigned long long r = XdvDisassemble(ah, it->second, dump, mn, sizeof(mn));
			if (r == 0)
			{
				continue;
			}

			unsigned long align = (unsigned long)(100 - strlen(mn));
			char asm_mn[3072] = { 0, };
			sprintf_s(asm_mn, sizeof(asm_mn), " %s", mn);

			//
			//
			std::vector<unsigned long long> ov;
			bool ovr = XdvGetOperandValues(ah, ih, it->second, dump, ov);
			if (ovr && ov.size() >= 1) // is call code
			{
				char symbol[1000] = { 0, };
				memset(asm_mn, 0, sizeof(asm_mn));
				if (XdvGetSymbolString(ih, ov[0], symbol, sizeof(symbol)))
				{
					sprintf_s(asm_mn, sizeof(asm_mn), " %s%*c; 0x%I64x, %s\n", mn, align, ' ', ov[0], symbol);
				}
				else
				{
					xdv::memory::type mbi;
					if (XdvQueryMemory(ih, ov[0], &mbi))
					{
						unsigned long long end = mbi.BaseAddress + mbi.RegionSize;
						sprintf_s(asm_mn, sizeof(asm_mn), " %s%*c; 0x%I64x, %s %I64x::%I64x-%I64x", mn, align, ' ', ov[0], "<unknown>\n", mbi.AllocationBase, mbi.BaseAddress, end);
					}
					else
					{
						sprintf_s(asm_mn, sizeof(asm_mn), " %s%*c; 0x%I64x, %s", mn, align, ' ', ov[0], "<unknown>\n");
					}
				}
			}
			else
			{
				continue;
			}
			worker->InsertString(asm_mn);
			worker->Update();
		}
	}
}

void PrintIntermodularCall(IWorker *worker, void *ctx)
{
	ref_value_ctx *rctx = (ref_value_ctx *)ctx;
	FindIntermodularCall(worker, rctx->base, rctx->end);
	free(ctx);
}

void FindIntermodularCall(unsigned long long ptr)
{
	xdv_handle ah = XdvGetArchitectureHandle();
	xdv_handle ih = XdvGetParserHandle();
	xdv::memory::type mbi;
	if (!XdvQueryMemory(ih, ptr, &mbi))
	{
		return;
	}

	//
	//
	xvar var = XdvExe("!qxnm.addv -name:Tag -title:Intermodular Call %I64x-%I64x -type:txta -callback:non", mbi.BaseAddress, mbi.BaseAddress + mbi.RegionSize);
	xdv_handle tag_handle = handlevar(var);
	XdvExe("!qxnm.chkable -handle:%x", tag_handle);

	IViewer *v = (IViewer *)XdvGetObjectByHandle(tag_handle);
	if (v)
	{
		v->AddViewer();
		ref_value_ctx * ctx = new ref_value_ctx;
		ctx->base = mbi.BaseAddress;
		ctx->end = mbi.BaseAddress + mbi.RegionSize;
		ctx->handle = tag_handle;
		XdvRun(tag_handle, PrintIntermodularCall, ctx);

		XdvExe("!qxnm.addtabv -va:%x -area:left -vb:%x", _current_handle, tag_handle);
		XdvExe("!qxnm.raise -handle:%x", tag_handle);
	}
}