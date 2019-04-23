#include "xdv_sdk.h"
#pragma comment(lib, "corexts.lib")

xdv_handle _current_handle = 0;
XENOM_ADD_INTERFACE()
{
	xvar var = XdvExe("!qxnm.add_viewer -name:Procedure -title:subroutine -callback:!procv.cbprocv -type:event");
	_current_handle = handlevar(var);
	XdvExe("!qxnm.set_checkable -handle:%x", _current_handle);

	return _current_handle;
}

// ------------------------------------------------------
//
std::set<unsigned long long> _visit_set;
std::map<unsigned long long, unsigned long long> _visit_map;
std::multimap<unsigned long long, unsigned long long> _ref_map;
std::map<unsigned long long, xdv::architecture::x86::block::id> _ptr_map;

std::mutex _ref_map_mutex;
std::mutex _analyze_map_mutex;
std::map<unsigned long long, xdv::architecture::x86::block::id>::iterator _current_it = _ptr_map.end();

std::string PrintSubroutineList();
void UpdateDisasmViewer()
{
	XdvExe("!dasmv.dasm");
}

void UpdateCurrentViewer()
{
	if (_ptr_map.size())
	{
		XdvPrintAndClear(_current_handle, PrintSubroutineList(), false);
	}
}

// ------------------------------------------------------
//
void findReferenceValueCallback(unsigned long long callee, unsigned long long caller, void *cb_ctx)
{
	_ref_map_mutex.lock();
	_ref_map.insert(std::multimap<unsigned long long, unsigned long long>::value_type(callee, caller));
	_ref_map_mutex.unlock();
}

bool analyzeCodeBlockCallback(unsigned long long ptr, void *cb_ctx, xdv::architecture::x86::block::id id)
{
	_analyze_map_mutex.lock();
	_ptr_map.insert(std::map<unsigned long long, xdv::architecture::x86::block::id>::value_type(ptr, id));
	_analyze_map_mutex.unlock();

	return true;
}

unsigned long long getEntryPoint(unsigned long long ptr)
{
	xdv_handle ah = XdvGetArchitectureHandle();
	xdv_handle ih = XdvGetParserHandle();
	unsigned long long entry = 0;
	while (entry == 0)
	{
		unsigned char dump[16] = { 0, };
		unsigned long long read = XdvReadMemory(ih, ptr, dump, sizeof(dump));
		if (read == 0)
		{
			break;
		}

		xdv::architecture::x86::type type;
		unsigned long long size = XdvDisassemble(ah, ptr, dump, &type);
		if (size < 4)
		{
			--ptr;
			continue;
		}

		//ptr += size;
		entry = XdvFindEntryPoint(ah, ih, ptr--);
	}

	return entry;
}

void AnalyzeCallback(IWorker *worker, void *ctx)
{
	xdv_handle ah = XdvGetArchitectureHandle();
	xdv_handle ih = XdvGetParserHandle();

	unsigned long long * ptr = (unsigned long long *)ctx;
	xdv::memory::type mbi;
	if (!XdvQueryMemory(ih, *ptr, &mbi))
	{
		free(ctx);
		return;
	}

	unsigned long long begine = *ptr;
	unsigned long long end = *ptr + 0x5000;
	if (begine <= mbi.BaseAddress)
	{
		begine = mbi.BaseAddress;
	}
	else
	{
		begine = getEntryPoint(begine);
	}

	if (end >= mbi.BaseAddress + mbi.RegionSize)
	{
		end = mbi.BaseAddress + mbi.RegionSize;
	}
	else
	{
		end = getEntryPoint(end);
	}

	if (begine == 0 || end == 0)
	{
		_visit_set.erase(mbi.BaseAddress); // remove lock
		free(ctx);
		return;
	}

	//
	// collect xref values
	XdvPrintLog("xenom:: collect values %I64x-%I64x s", begine, end);
	{
		XdvFineReferenceValues(ah, ih, begine, (size_t)(end - begine), findReferenceValueCallback, nullptr);
	}
	UpdateDisasmViewer();
	XdvPrintLog("xenom:: collect values %I64x-%I64x e", begine, end);

	//
	// analyze block
	XdvPrintLog("xenom:: analyze subr %I64x-%I64x s", begine, end);
	{
		XdvAnalyze(ah, ih, begine, (size_t)(end - begine), analyzeCodeBlockCallback, nullptr);
	}
	_current_it = _ptr_map.begin();
	UpdateDisasmViewer();
	UpdateCurrentViewer();

	XdvPrintLog("xenom:: analyze subr %I64x-%I64x e", begine, end);
	XdvPrintLog("xenom:: analyze subr %I64x-%I64x count %d", begine, end, _ptr_map.size());

	_visit_set.erase(mbi.BaseAddress); // remove lock
	_visit_map[begine] = end; // add visit map

	free(ctx);
}

EXTS_FUNC(analyze)	// argv[0] = ptr
					// return nullptr
					// help
{
	unsigned long long ptr = toullarg("ptr");
	if (ptr == 0)
	{
		return nullvar();
	}

	xdv_handle ah = XdvGetArchitectureHandle();
	xdv_handle ih = XdvGetParserHandle();
	xdv::memory::type mbi;
	if (!XdvQueryMemory(ih, ptr, &mbi))
	{
		return nullvar();
	}

	std::set<unsigned long long>::iterator vsit = _visit_set.find(mbi.BaseAddress);
	if (vsit == _visit_set.end())
	{
		std::map<unsigned long long, unsigned long long>::iterator vmit = _visit_map.begin();
		for (vmit; vmit != _visit_map.end(); ++vmit)
		{
			if (vmit->first <= ptr && ptr <= vmit->second)
			{
				return nullvar();
			}
		}

		unsigned long long * pptr = (unsigned long long *)malloc(sizeof(unsigned long long));
		*pptr = ptr;
		XdvPrintLog("xenom:: analyze ptr=%I64x", ptr);

		_visit_set.insert((unsigned long long)mbi.BaseAddress);
		XdvRun(_current_handle, AnalyzeCallback, pptr);
	}

	return nullvar();
}

EXTS_FUNC(reftable)
{
	return ptrvar(&_ref_map);
}

EXTS_FUNC(proctable)
{
	return ptrvar(&_ptr_map);
}

bool IsPrintLine()
{
	std::map<unsigned long long, xdv::architecture::x86::block::id>::iterator it = _current_it;
	int check = 0;
	for (it; it != _ptr_map.end(); ++it, ++check)
	{
		if (check == 100)
		{
			break;
		}
	}

	if (check < 100)
	{
		return false;
	}

	return true;
}

std::string PrintSubroutineList()
{
	std::map<unsigned long long, xdv::architecture::x86::block::id>::iterator it = _current_it;
	xdv_handle ih = XdvGetParserHandle();
	std::string str;
	int i = 0;
	for (it; it != _ptr_map.end(); ++it, ++i)
	{
		if (i == 100)
		{
			break;
		}

		char symbol[256] = { 0, };
		if (XdvGetSymbolString(ih, it->first, symbol, sizeof(symbol)))
		{
			str += " ";
			str += symbol;
			str += "\n";
		}
		else
		{
			char sub_name[500] = { 0, };
			sprintf_s(sub_name, sizeof(sub_name), " sub_%I64x\n", it->first);

			str += sub_name;
		}
	}

	return str;
}

EXTS_FUNC(cbprocv)	// argv[0] = status
					// argv[1] = handle
{
	if (hasarg("status", "up"))
	{
		if (_current_it != _ptr_map.begin())
		{
			--_current_it;
			UpdateCurrentViewer();
		}
	}
	else if (hasarg("status", "down"))
	{
		if (_current_it != _ptr_map.end() && IsPrintLine())
		{
			++_current_it;
			UpdateCurrentViewer();
		}
	}
	else if (hasarg("status", "pre"))
	{
		char * str = argof("str");
		std::vector<std::string> np = XdvSplit(str, "\\s+");
		std::vector<std::string> filter = XdvSplit(np[0], "[+-]+");
		unsigned long long result[30] = { 0, };
		for (size_t i = 0; i < filter.size(); ++i)
		{
			char * end = nullptr;
			result[i] = XdvGetSymbolPointer(XdvGetParserHandle(), (char *)filter[i].c_str());
			if (result[i] == 0)
			{
				result[i] = strtoull(filter[i].c_str(), &end, 16);
			}
		}

		int idx = 0;
		unsigned long long ptr = result[idx++];
		for (size_t i = 0; i < strlen(str); ++i)
		{
			char c = str[i];
			switch (c)
			{
			case '+':
				ptr += result[idx++];
				break;

			case '-':
				ptr -= result[idx++];
				break;
			}
		}

		XdvExe("!dasmv.dasm -ptr:%I64x", ptr);
	}

	return nullvar();
}

EXTS_FUNC(update) // first callback
{
	if (_ptr_map.size())
	{
		XdvPrintAndClear(_current_handle, PrintSubroutineList(), false);
	}
	return nullvar();
}