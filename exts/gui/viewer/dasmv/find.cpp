#include "xdv_sdk.h"

typedef struct _tag_ref_value_ctx
{
	xdv_handle handle;
	unsigned long long base;
	unsigned long long end;
}ref_value_ctx;

void SetTextColor(xdv_handle);
unsigned long long GetCurrentPtr();
xdv_handle GetCurrentHandle();

void FindReferenceValueCallback(unsigned long long callee, unsigned long long caller, void *cb_ctx)
{
	std::multimap<unsigned long long, unsigned long long> * ref_map = (std::multimap<unsigned long long, unsigned long long> *)cb_ctx;
	if (ref_map)
	{
		ref_map->insert(std::multimap<unsigned long long, unsigned long long>::value_type(callee, caller));
	}
}

void FindReferenceString(IWorker *worker, unsigned long long base, unsigned long long end)
{
	xdv_handle ah = XdvGetArchitectureHandle();
	xdv_handle ih = XdvGetParserHandle();
	std::multimap<unsigned long long, unsigned long long> ref_map;
	XdvFineReferenceValues(ah, ih, base, (size_t)(end - base), FindReferenceValueCallback, &ref_map);

	int i = 0;
	std::string str;
	std::multimap<unsigned long long, unsigned long long>::iterator it = ref_map.begin();
	for (it; it != ref_map.end(); ++it)
	{
		if (base <= it->second && it->second <= end)
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
				}
			}
		}
	}
	worker->Update();
}

void PrintReferenceString(IWorker *worker, void *ctx)
{
	ref_value_ctx *rctx = (ref_value_ctx *)ctx;
	FindReferenceString(worker, rctx->base, rctx->end);
	free(ctx);

	XdvExe("!qxnm.raise_viewer -handle:%x", worker->LinkViewerHandle());
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

	xvar var = XdvExe("!qxnm.add_viewer -name:Tag -title:Reference String %I64x-%I64x -type:txta -callback:non", mbi.BaseAddress, mbi.BaseAddress + mbi.RegionSize);
	xdv_handle tag_handle = handlevar(var);
	XdvExe("!qxnm.set_checkable -handle:%x", tag_handle);

	IViewer *v = (IViewer *)XdvGetObjectByHandle(tag_handle);
	if (v)
	{
		v->AddViewer();
		SetTextColor(tag_handle);
		ref_value_ctx * ctx = new ref_value_ctx;
		ctx->base = mbi.BaseAddress;
		ctx->end = mbi.BaseAddress + mbi.RegionSize;
		ctx->handle = tag_handle;
		XdvRun(tag_handle, PrintReferenceString, ctx);

		XdvExe("!qxnm.add_tab -va:%x -area:left -vb:%x", GetCurrentHandle(), tag_handle);
	}
}

// ------------------------------------------------------
//
void FindIntermodularCall(IWorker * worker, unsigned long long base, unsigned long long end)
{
	xdv_handle ah = XdvGetArchitectureHandle();
	xdv_handle ih = XdvGetParserHandle();
	std::multimap<unsigned long long, unsigned long long> ref_map;
	XdvFineReferenceValues(ah, ih, base, (size_t)(end - base), FindReferenceValueCallback, &ref_map);

	int i = 0;
	std::string str;
	std::multimap<unsigned long long, unsigned long long>::iterator it = ref_map.begin();
	for (it; it != ref_map.end(); ++it)
	{
		if (!(base <= it->first && it->first <= end))
		{
			unsigned char dump[16] = { 0, };
			unsigned long long readn = XdvReadMemory(ih, it->second, dump, sizeof(dump));
			if (readn == 0)
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
		}
	}

	worker->Update();
}

void PrintIntermodularCall(IWorker *worker, void *ctx)
{
	ref_value_ctx *rctx = (ref_value_ctx *)ctx;
	FindIntermodularCall(worker, rctx->base, rctx->end);
	free(ctx);

	XdvExe("!qxnm.raise_viewer -handle:%x", worker->LinkViewerHandle());
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
	xvar var = XdvExe("!qxnm.add_viewer -name:Tag -title:Intermodular Call %I64x-%I64x -type:txta -callback:non", mbi.BaseAddress, mbi.BaseAddress + mbi.RegionSize);
	xdv_handle tag_handle = handlevar(var);
	XdvExe("!qxnm.set_checkable -handle:%x", tag_handle);

	IViewer *v = (IViewer *)XdvGetObjectByHandle(tag_handle);
	if (v)
	{
		v->AddViewer();
		SetTextColor(tag_handle);

		ref_value_ctx * ctx = new ref_value_ctx;
		ctx->base = mbi.BaseAddress;
		ctx->end = mbi.BaseAddress + mbi.RegionSize;
		ctx->handle = tag_handle;
		XdvRun(tag_handle, PrintIntermodularCall, ctx);

		XdvExe("!qxnm.add_tab -va:%x -area:left -vb:%x", GetCurrentHandle(), tag_handle);
	}
}