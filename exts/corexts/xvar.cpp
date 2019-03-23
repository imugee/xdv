#include "xdv_sdk.h"

xvar nullvar()
{
	xvar r = { 0, };
	return r;
}

xvar ullvar(unsigned long long var)
{
	xvar r = { 0, };
	sprintf_s(r.tag, sizeof(r.tag), "-ull:%I64x", var);
	return r;
}

xvar ptrvar(void * var)
{
	xvar r = { 0, };
	sprintf_s(r.tag, sizeof(r.tag), "-ptr:%p", var);
	return r;
}

xvar handlevar(xdv_handle var)
{
	xvar r = { 0, };
	sprintf_s(r.tag, sizeof(r.tag), "-handle:%x", var);

	return r;
}

// --------------------------------------------------------
// 
unsigned long long ullvar(xvar var)
{
	if (strstr(var.tag, "ull"))
	{
		const char * svar = strstr(var.tag, ":") + 1;
		if (svar)
		{
			char * end = nullptr;
			unsigned long long r = strtoull(svar, &end, 16);
			return r;
		}
	}

	return 0;
}

void * ptrvar(xvar var)
{
	if (strstr(var.tag, "ptr"))
	{
		const char * svar = strstr(var.tag, ":") + 1;
		if (svar)
		{
			char * end = nullptr;
			unsigned long long r = strtoull(svar, &end, 16);
			return (void *)r;
		}
	}

	return nullptr;
}

xdv_handle handlevar(xvar var)
{
	if (strstr(var.tag, "handle"))
	{
		const char * svar = strstr(var.tag, ":") + 1;
		if (svar)
		{
			char * end = nullptr;
			xdv_handle xh = strtoul(svar, &end, 16);
			return xh;
		}
	}

	return 0;
}


