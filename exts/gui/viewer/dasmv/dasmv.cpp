#include "xdv_sdk.h"

#pragma comment(lib, "corexts.lib")

unsigned long long _current_ptr = 0;
xdv_handle _current_handle;
XENOM_ADD_INTERFACE()
{
	xvar var = XdvExe("!qxnm.add_viewer -name:Disassemble -title:code -type:dasm -callback:!dasmv.cbdasmv");
	_current_handle = handlevar(var);
	XdvExe("!qxnm.set_checkable -handle:%x", _current_handle);

	return _current_handle;
}

void SetTextColor(xdv_handle);
void Update();
bool _is_first = false;
EXTS_FUNC(update) // first callback
{
	Update();

	if (!_is_first)
	{
		XdvExe("!qxnm.add_command -handle:%x -name:Jump -key:%x", _current_handle, xdv::key::Key_CTRL | xdv::key::id::Key_G);

		XdvExe("!qxnm.add_command -handle:%x -menu:Tag -name:Trace from the entry point -key:%x", _current_handle, xdv::key::Key_CTRL | xdv::key::id::Key_E);
		XdvExe("!qxnm.add_command -handle:%x -menu:Tag -name:Trace from the current point -key:%x", _current_handle, xdv::key::Key_CTRL | xdv::key::id::Key_P);
		XdvExe("!qxnm.add_command -handle:%x -menu:Tag -name:Track trace -key:%x", _current_handle, xdv::key::Key_CTRL | xdv::key::id::Key_T);

		XdvExe("!qxnm.add_command -handle:%x -menu:Find -name:Find reference string", _current_handle);
		XdvExe("!qxnm.add_command -handle:%x -menu:Find -name:Find intermodular call", _current_handle);

		XdvExe("!qxnm.add_command -handle:%x -menu:Break point -name:Suspend point", _current_handle);
		XdvExe("!qxnm.add_command -handle:%x -menu:Break point -name:Software break point", _current_handle);
		XdvExe("!qxnm.add_command -handle:%x -menu:Break point -name:Hardware break point", _current_handle);
		XdvExe("!qxnm.add_command -handle:%x -menu:Break point -name:Delete break point", _current_handle);

		SetTextColor(_current_handle);

		_is_first = true;
	}

	return nullvar();
}

std::multimap<unsigned long long, unsigned long long> * _ref_map_ptr = nullptr;
std::map<unsigned long long, xdv::architecture::x86::block::id> * _proc_map_ptr = nullptr;
std::vector<unsigned long long> _trace_vector;
std::set<unsigned long long> _break_point_set;

void TraceFromEntryPoint(unsigned long long ptr);
void TraceFromCurrentPoint(unsigned long long ptr);
void TrackTrace(unsigned long long ptr);
void FindReferenceString(unsigned long long ptr);
void FindIntermodularCall(unsigned long long ptr);
EXTS_FUNC(cbdasmv)	// argv = status
					// argv = string
{
	if (hasarg("status", "up"))
	{
		_current_ptr -= 3;
		Update();
	}
	else if (hasarg("status", "down"))
	{
		_current_ptr += 3;
		Update();
	}
	else if (hasarg("status", "pre"))
	{
		unsigned long long ptr = toullarg("str");
		if (ptr)
		{
			unsigned char dump[16] = { 0, };
			if (XdvReadMemory(XdvGetParserHandle(), ptr, dump, sizeof(dump)))
			{
				_trace_vector.push_back(ptr);
			}
		}
	}
	else if (hasarg("status", "dc"))
	{
		char * ptr_str = argof("str");
		if (strlen(ptr_str))
		{
			ptr_str = strstr(ptr_str, "0x");
			if (ptr_str)
			{
				XdvExe("!dasmv.dasm -ptr:%s", ptr_str);
			}
		}
	}
	else if (hasarg("status", "backspace"))
	{
		unsigned long long ptr = _trace_vector[_trace_vector.size() - 1];
		if (ptr)
		{
			XdvExe("!dasmv.dasm -ptr:%I64x", ptr);
		}
	}
	else if (hasarg("status", "space"))
	{
		char * ptr_str = argof("str");
		ptr_str = strstr(ptr_str, "0x");
		if (ptr_str)
		{
			XdvExe("!dasmv.dasm -ptr:%s", ptr_str);
			Update();
		}
	}

	// ------------------------------------------------------
	// context menu
	else if (hasarg("status", "Jump"))
	{
		xvar var = XdvExe("!qxnm.goto_dialog");
		unsigned long long ptr = ullvar(var);
		if (ptr)
		{
			XdvExe("!dasmv.dasm -ptr:%I64x", ptr);
		}
	}
	else if (hasarg("status", "Trace from the entry point"))
	{
		unsigned long long ptr = toullarg("tag");
		if (ptr)
		{
			TraceFromEntryPoint(ptr);
		}
	}
	else if (hasarg("status", "Trace from the current point"))
	{
		unsigned long long ptr = toullarg("tag");
		if (ptr)
		{
			TraceFromCurrentPoint(ptr);
		}
	}
	else if (hasarg("status", "Track trace"))
	{
		unsigned long long ptr = toullarg("tag");
		if (ptr)
		{
			TrackTrace(ptr);
		}
	}
	else if (hasarg("status", "Find reference string"))
	{
		unsigned long long ptr = toullarg("tag");
		if (ptr)
		{
			FindReferenceString(ptr);
		}
	}
	else if (hasarg("status", "Find intermodular call"))
	{
		unsigned long long ptr = toullarg("tag");
		if (ptr)
		{
			FindIntermodularCall(ptr);
		}
	}
	else if (hasarg("status", "Suspend point"))
	{
		unsigned long long ptr = toullarg("tag");
		if (ptr)
		{
			if (XdvSetBreakPoint(XdvGetParserHandle(), DebugBreakPointId::SUSPEND_BREAK_POINT_ID, ptr))
			{
				_break_point_set.insert(ptr);
				Update();
			}
		}
	}
	else if (hasarg("status", "Software break point"))
	{
		unsigned long long ptr = toullarg("tag");
		if (ptr)
		{
			XdvSuspendProcess(XdvGetParserHandle());
			if (XdvSetBreakPoint(XdvGetParserHandle(), DebugBreakPointId::SOFTWARE_BREAK_POINT_ID, ptr))
			{
				_break_point_set.insert(ptr);
				Update();
			}
			XdvResumeProcess(XdvGetParserHandle());
		}
	}
	else if (hasarg("status", "Hardware break point"))
	{
		unsigned long long ptr = toullarg("tag");
		if (ptr)
		{
			if (XdvSetBreakPoint(XdvGetParserHandle(), DebugBreakPointId::HARDWARE_BREAK_POINT_ID, ptr))
			{
				_break_point_set.insert(ptr);
				Update();
			}
		}
	}
	else if (hasarg("status", "Delete break point"))
	{
		unsigned long long ptr = toullarg("tag");
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

unsigned long long GetCurrentPtr()
{
	return _current_ptr;
}

void SetCurrentPtr(unsigned long long ptr)
{
	_current_ptr = ptr;
}

xdv_handle GetCurrentHandle()
{
	return _current_handle;
}

void SetTextColor(xdv_handle viewer_handle)
{
	XdvExe("!qxnm.express_color -handle:%x -expression:(call|jmp|jne|je|jae|ja|jbe|jb|jge|jg|jle|jl|jz|jnz) -color:#842f2f", viewer_handle);
	XdvExe("!qxnm.express_color -handle:%x -expression:(movzx|movsb|movsx|movs|mov) -color:#664e72", viewer_handle);
	XdvExe("!qxnm.express_color -handle:%x -expression:(pushad|popad|pusha|popa|pushfd|popfd|pushf|popf|push|pop) -color:#5e875b", viewer_handle);
	XdvExe("!qxnm.express_color -handle:%x -expression:(qword ptr|dword ptr|word ptr|byte ptr) -color:#774343", viewer_handle);
	XdvExe("!qxnm.express_color -handle:%x -expression:(eax|ebx|ecx|edx|edi|esi|ebp|esp|eip|rax|rbx|rcx|rdx|rdi|rsi|rbp|rsp|rip|r8|r9|r10|r11|r12|r13|r14|r15) -color:#3d556d", viewer_handle);

	XdvExe("!qxnm.express_color -handle:%x -expression:;[^\\n]* -color:#009327", viewer_handle);
	XdvExe("!qxnm.express_color -handle:%x -expression:.sym[^\\n]* -color:#006caf", viewer_handle);
	XdvExe("!qxnm.express_color -handle:%x -expression:.xref[^\\n]* -color:#006caf", viewer_handle);

	XdvExe("!qxnm.express_color -handle:%x -expression:=========================== subroutine ===========================[^\\n]* -color:#005e5e -bold:true", viewer_handle);
	XdvExe("!qxnm.express_color -handle:%x -expression:======= current point -color:#ff3838", viewer_handle);
	XdvExe("!qxnm.express_color -handle:%x -expression:======= break point -color:#ff3838", viewer_handle);

	XdvExe("!qxnm.express_color -handle:%x -expression:\"[^\\n]* -color:#ff9b77", viewer_handle);
	XdvExe("!qxnm.express_color -handle:%x -expression:L\"[^\\n]* -color:#ff9b77", viewer_handle);
}

// ------------------------------------------------------
//
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
		if (!dump)
		{
			dump = cdump;
			_break_point_set.erase(bi);
		}
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

// ------------------------------------------------------
//
void TraceFromEntryPoint(unsigned long long ptr)
{
	xvar var = XdvExe("!qxnm.add_viewer -name:Tag -title:tag %I64x -type:txta -callback:non", ptr);
	xdv_handle tag_handle = handlevar(var);
	XdvExe("!qxnm.set_checkable -handle:%x", tag_handle);

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
	SetTextColor(tag_handle);
	XdvPrintViewer(tag_handle, dasm_str);

	XdvExe("!qxnm.add_tab -va:%x -area:left -vb:%x", _current_handle, tag_handle);
	XdvExe("!qxnm.raise_viewer -handle:%x", tag_handle);
}

void TraceFromCurrentPoint(unsigned long long ptr)
{
	xvar var = XdvExe("!qxnm.add_viewer -name:Tag -title:tag %I64x -type:txta -callback:non", ptr);
	xdv_handle tag_handle = handlevar(var);
	XdvExe("!qxnm.set_checkable -handle:%x", tag_handle);

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
	SetTextColor(tag_handle);
	XdvPrintViewer(tag_handle, dasm_str);

	XdvExe("!qxnm.add_tab -va:%x -area:left -vb:%x", _current_handle, tag_handle);
	XdvExe("!qxnm.raise_viewer -handle:%x", tag_handle);
}

void TrackTrace(unsigned long long ptr)
{
	xvar var = XdvExe("!qxnm.add_viewer -name:Tag -title:tag %I64x -type:txta -callback:non", ptr);
	xdv_handle tag_handle = handlevar(var);
	XdvExe("!qxnm.set_checkable -handle:%x", tag_handle);

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
	SetTextColor(tag_handle);
	XdvPrintViewer(tag_handle, dasm_str);

	XdvExe("!qxnm.add_tab -va:%x -area:left -vb:%x", _current_handle, tag_handle);
	XdvExe("!qxnm.raise_viewer -handle:%x", tag_handle);
}

