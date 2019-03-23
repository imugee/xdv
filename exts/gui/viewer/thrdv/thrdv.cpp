#include "xdv_sdk.h"

#pragma comment(lib, "corexts.lib")

xdv_handle _current_handle;
XENOM_ADD_INTERFACE()
{
	xvar var = XdvExe("!qxnm.addv -name:Threads -title:thread -type:txta -callback:!thrdv.cbthrdv");
	_current_handle = handlevar(var);
	XdvExe("!qxnm.chkable -handle:%x", _current_handle);

	return _current_handle;
}

EXTS_FUNC(cbthrdv)
{
	char * status = XdvValue(argv, argc, "status", nullptr);
	char * handle = XdvValue(argv, argc, "handle", nullptr);
	char * str = XdvValue(argv, argc, "str", nullptr);
	if (strstr(status, "doubleclick"))
	{
		//XdvExe("!thrdv.threads");

		if (strlen(str))
		{
			char * tid_str = strstr(str, ":");
			if (tid_str)
			{
				tid_str += 2;
				char * end = nullptr;
				unsigned long tid = strtoul(tid_str, &end, 16);
				if (XdvSelectThread(XdvGetParserHandle(), tid))
				{
					printf("test:: select\n");
					//XdvSuspendThread(XdvGetParserHandle(), tid);
					XdvExe("!cpuv.printctx");
					XdvExe("!stackv.printframe");
					//XdvResumeThread(XdvGetParserHandle(), tid);
				}
			}
		}
	}
	else if (strstr(status, "Suspend thread"))
	{
		char * tag = XdvValue(argv, argc, "tag", nullptr);
		char * end = nullptr;
		if (tag)
		{
			char * tid_str = strstr(tag, ":");
			if (tid_str)
			{
				tid_str += 2;
				char * end = nullptr;
				unsigned long tid = strtoul(tid_str, &end, 16);
				XdvSuspendThread(XdvGetParserHandle(), tid);
			}
		}
	}
	else if (strstr(status, "Resume thread"))
	{
		char * tag = XdvValue(argv, argc, "tag", nullptr);
		char * end = nullptr;
		if (tag)
		{
			char * tid_str = strstr(tag, ":");
			if (tid_str)
			{
				tid_str += 2;
				char * end = nullptr;
				unsigned long tid = strtoul(tid_str, &end, 16);
				XdvResumeThread(XdvGetParserHandle(), tid);
			}
		}
	}
	else if (strstr(status, "Suspend process"))
	{
		//XdvUpdateDebuggee(XdvGetParserHandle());

		std::map<unsigned long, unsigned long long> thread_map;
		XdvThreads(XdvGetParserHandle(), thread_map);

		if (thread_map.size())
		{
			std::map<unsigned long, unsigned long long>::reverse_iterator rit = thread_map.rbegin();
			for (rit; rit != thread_map.rend(); ++rit)
			{
				XdvSuspendThread(XdvGetParserHandle(), rit->first);
			}

			XdvExe("!thrdv.threads");
		}
	}
	else if (strstr(status, "Resume process"))
	{
		XdvUpdateDebuggee(XdvGetParserHandle());

		std::map<unsigned long, unsigned long long> thread_map;
		XdvThreads(XdvGetParserHandle(), thread_map);

		if (thread_map.size())
		{
			std::map<unsigned long, unsigned long long>::reverse_iterator rit = thread_map.rbegin();
			for (rit; rit != thread_map.rend(); ++rit)
			{
				XdvResumeThread(XdvGetParserHandle(), rit->first);
			}
		}

		XdvExe("!thrdv.threads");
	}

	return nullvar();
}

EXTS_FUNC(threads)
{
	std::map<unsigned long, unsigned long long> thread_map;
	XdvThreads(XdvGetParserHandle(), thread_map);

	if (thread_map.size())
	{
		unsigned long tid = 0;
		std::string str;
		std::map<unsigned long, unsigned long long>::reverse_iterator rit = thread_map.rbegin();
		for (rit; rit != thread_map.rend(); ++rit)
		{
			if (tid == 0)
			{
				tid = rit->first;
			}

			char line[1024] = { 0, };
			sprintf_s(line, sizeof(line), "tid:%08x, entry:%I64x\n", rit->first, rit->second);
			str += line;
		}

		XdvSelectThread(XdvGetParserHandle(), tid);
		XdvPrintAndClear(_current_handle, str, false);
	}

	return nullvar();
}

EXTS_FUNC(update)
{
	XdvExe("!qxnm.ctxmenu -handle:%x -name:Suspend thread", _current_handle);
	XdvExe("!qxnm.ctxmenu -handle:%x -name:Resume thread", _current_handle);

	XdvExe("!qxnm.ctxmenu -handle:%x -name:Suspend process", _current_handle);
	XdvExe("!qxnm.ctxmenu -handle:%x -name:Resume process", _current_handle);

	return nullvar();
}