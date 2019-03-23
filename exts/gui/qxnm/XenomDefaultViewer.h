#ifndef __DEFINE_XENOM_DEFAULT_VIEWER__
#define __DEFINE_XENOM_DEFAULT_VIEWER__

#include "xdv_sdk.h"

class XenomDefaultViewer : public IViewer
{
private:
	IWorker *worker_;
	xdv_handle current_handle_;
	std::string object_str_;
	std::string title_str_;
	std::string command_str_;
	std::string module_name_;
	bool is_open_;
	bool use_event_;
	bool checkable_;
	xdv::viewer::id id_;

public:
	virtual xdv::object::id ObjectType();
	virtual std::string ObjectString();
	virtual void SetModuleName(std::string module);
	virtual std::string ModuleName();

public:
	XenomDefaultViewer();
	void SetCurrentHandle(xdv_handle handle);

public:
	virtual void AddViewer();
	virtual void CloseViewer();

	virtual bool IsOpen();
	virtual bool IsCheckable();

	virtual bool Update(int status, std::string str);
	virtual void Print(std::string str);
	virtual void Print(std::string str, bool wait);
	virtual void PrintAndClear(std::string str, bool wait = false);

	virtual IWorker * GetWorker();

public:
	void SetCheckable(bool checkable);
	void SetObjectString(std::string obj_str);
	void SetTitleString(std::string obj_str);
	void SetCommandString(std::string command_str);
	void SetViewerId(xdv::viewer::id id);
};

#endif