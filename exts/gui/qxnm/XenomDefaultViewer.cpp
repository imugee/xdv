#include "XenomDefaultViewer.h"
#include "qxnm.h"
#include "XenomDockWidget.h"
#include "XenomTextViewer.h"

XenomDefaultViewer::XenomDefaultViewer()
	: current_handle_(0), is_open_(false), worker_(nullptr), checkable_(false), id_(xdv::viewer::id::DEFAULT_TEXT_VIEWER)
{
}

void XenomDefaultViewer::SetCurrentHandle(xdv_handle handle)
{
	current_handle_ = handle;
}

xdv::object::id XenomDefaultViewer::ObjectType()
{
	return xdv::object::id::XENOM_VIEWER_OBJECT;
}

std::string XenomDefaultViewer::ObjectString()
{
	return object_str_;
}

void XenomDefaultViewer::SetModuleName(std::string module)
{
	module_name_ = module;
}

std::string XenomDefaultViewer::ModuleName()
{
	return module_name_;
}

void XenomDefaultViewer::SetObjectString(std::string obj_str)
{
	object_str_ = obj_str;
}

void XenomDefaultViewer::SetTitleString(std::string title_str)
{
	title_str_ = title_str;
}

void XenomDefaultViewer::SetCommandString(std::string command_str)
{
	command_str_ = command_str;
}

void XenomDefaultViewer::SetViewerId(xdv::viewer::id id)
{
	id_ = id;
}

void XenomDefaultViewer::SetCheckable(bool checkable)
{
	checkable_ = checkable;
}

bool XenomDefaultViewer::IsOpen()
{
	return is_open_;
}

bool XenomDefaultViewer::IsCheckable()
{
	return checkable_;
}

void XenomDefaultViewer::AddViewer()
{
	xvar var;
	switch (id_)
	{
	case xdv::viewer::id::DEFAULT_TEXT_VIEWER:
		var = XdvExe("!qxnm.add_txtv -handle:%x -title:%s", current_handle_, title_str_.c_str());
		worker_ = (IWorker *)ptrvar(var);
		break;

	case xdv::viewer::id::EVENT_BASE_TEXT_VIEWER:
		var = XdvExe("!qxnm.add_txtv -handle:%x -title:%s -type:event", current_handle_, title_str_.c_str());
		worker_ = (IWorker *)ptrvar(var);
		break;

	case xdv::viewer::id::TEXT_VIEWER_DASM:
		var = XdvExe("!qxnm.add_txtv -handle:%x -title:%s -type:dasm", current_handle_, title_str_.c_str());
		worker_ = (IWorker *)ptrvar(var);
		break;

	case xdv::viewer::id::COMMAND_VIEWER:
		var = XdvExe("!qxnm.add_cmdv -handle:%x -title:%s", current_handle_, title_str_.c_str());
		worker_ = (IWorker *)ptrvar(var);
		break;
	}

	if (checkable_)
	{
		is_open_ = true;
	}
}

void XenomDefaultViewer::CloseViewer()
{
	xnm * xenom = getXenom();
	XenomDockWidget * dock = xenom->Viewer(current_handle_);
	if (dock)
	{
		if (dock->TextViewer())
		{
			dock->TextViewer()->close();
		}

		dock->close();
	}
	
	is_open_ = false;
}

bool XenomDefaultViewer::Update(int status, std::string str)
{
	if (command_str_.size() == 0)
	{
		return false;
	}

	std::string command = command_str_;
	char handle[100] = { 0, };
	sprintf_s(handle, sizeof(handle), "%x", current_handle_);
	switch (status)
	{
	case xdv::status::XENOM_UPDATE_STATUS_UP:
		command += " -status:up";
		command += " -handle:";
		command += handle;
		XdvExe((char *)command.c_str());
		break;

	case xdv::status::XENOM_UPDATE_STATUS_DOWN:
		command += " -status:down";
		command += " -handle:";
		command += handle;
		XdvExe((char *)command.c_str());
		break;

	case xdv::status::XENOM_UPDATE_STATUS_PRE_EVENT:
	{
		command += " -status:pre";
		command += " -handle:";
		command += handle;
		if (str.size())
		{
			command += " -str:";
			command += str.c_str();
		}
		XdvExe((char *)command.c_str());
		break;
	}

	case xdv::status::XENOM_UPDATE_STATUS_DOUBLE_CLICK_POST_EVENT: // mouse double click
	{
		command += " -status:dc";
		command += " -handle:";
		command += handle;
		if (str.size())
		{
			command += " -str:";
			command += str.c_str();
		}
		XdvExe((char *)command.c_str());
		break;
	}

	case xdv::status::XENOM_UPDATE_STSTUS_BACKSPACE:
	{
		command += " -status:backspace";
		command += " -handle:";
		command += handle;
		if (str.size())
		{
			command += " -str:";
			command += str.c_str();
		}
		XdvExe((char *)command.c_str());
		break;
	}

	case xdv::status::XENOM_UPDATE_STSTUS_SPACE_POST_EVENT:
	{
		command += " -status:space";
		command += " -handle:";
		command += handle;
		if (str.size())
		{
			command += " -str:";
			command += str.c_str();
		}
		XdvExe((char *)command.c_str());
		break;
	}

	case xdv::status::id::XENOM_UPDATE_STATUS_SHORTCUT:
		command += " -status:";
		command += str;
		command += " -handle:";
		command += handle;
		XdvExe((char *)command.c_str());
		break;
	}

	return true;
}

void XenomDefaultViewer::Print(std::string str)
{
	if (worker_)
	{
		worker_->Print(current_handle_, str);
	}
}

void XenomDefaultViewer::Print(std::string str, bool wait)
{
	if (worker_)
	{
		worker_->Print(current_handle_, str, wait);
	}
}

void XenomDefaultViewer::PrintAndClear(std::string str, bool wait)
{
	if (worker_)
	{
		worker_->PrintAndClear(current_handle_, str, wait);
	}
}

IWorker * XenomDefaultViewer::GetWorker()
{
	return worker_;
}