#ifndef __DEFINE_XENOM_WORKER__
#define __DEFINE_XENOM_WORKER__

#include "xdv_sdk.h"
#include "XenomThreadWorker.h"

class XenomWorker : public IWorker
{
private:
	XenomThreadWorker *worker_;
	std::string text_string_;
	std::mutex mutex_;
	xdv_handle link_handle_;

public:
	XenomWorker(xdv_handle link_handle);

public:
	virtual xdv::object::id ObjectType();
	virtual std::string ObjectString();
	virtual void SetModuleName(std::string module);
	virtual std::string ModuleName();

	virtual void Run(xdv_handle viewer_handle, ThreadRunCallbackType callback, void *callback_context);

	virtual void Print(xdv_handle viewer_handle, std::string str);
	virtual void Print(xdv_handle viewer_handle, std::string str, bool wait);
	virtual void PrintAndClear(xdv_handle viewer_handle, std::string str, bool wait);
	virtual void Clear(xdv_handle viewer_handle);

	virtual void InsertString(std::string str);
	virtual void ClearString();

	virtual void Update();
	virtual void UpdateAndClear();

	virtual std::string String();
	virtual xdv_handle LinkViewerHandle();

	virtual void Lock();
	virtual void UnLock();
};

#endif