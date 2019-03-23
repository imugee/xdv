#include "XenomDefaultViewer.h"
#include "qxnm.h"
#include "XenomDockWidget.h"
#include "XenomTextViewer.h"

XenomDefaultViewer::XenomDefaultViewer()
	: current_handle_(0), is_open_(false), worker_(nullptr), checkable_(false), id_(xdv::viewer::id::TEXT_VIEWER_A)
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
	case xdv::viewer::id::TEXT_VIEWER_A:
		var = XdvExe("!qxnm.addtxtv -handle:%x -title:%s", current_handle_, title_str_.c_str());
		worker_ = (IWorker *)ptrvar(var);
		break;

	case xdv::viewer::id::TEXT_VIEWER_B:
		var = XdvExe("!qxnm.addtxtv -handle:%x -title:%s -type:txtb", current_handle_, title_str_.c_str());
		worker_ = (IWorker *)ptrvar(var);
		break;

	case xdv::viewer::id::TEXT_VIEWER_C:
		var = XdvExe("!qxnm.addtxtv -handle:%x -title:%s -type:txtc", current_handle_, title_str_.c_str());
		worker_ = (IWorker *)ptrvar(var);
		break;

	case xdv::viewer::id::COMMAND_VIEWER_A:
		var = XdvExe("!qxnm.addcmdv -handle:%x -title:%s -type:txta", current_handle_, title_str_.c_str());
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
	case xdv::status::XENOM_UPDATE_STATUS_UP: // wheel up
		command += " -status:up";
		command += " -handle:";
		command += handle;
		XdvExe((char *)command.c_str());
		break;

	case xdv::status::XENOM_UPDATE_STATUS_DOWN: // wheel down
		command += " -status:down";
		command += " -handle:";
		command += handle;
		XdvExe((char *)command.c_str());
		break;

	case xdv::status::XENOM_UPDATE_STATUS_DOUBLE_CLICK: // mouse double click
	{
		command += " -status:doubleclick";
		command += " -handle:";
		command += handle;
		if (str.size())
		{
			command += " -str:";
			command += str.c_str();
		}
		XdvExe((char *)command.c_str());
	}
		break;

	case xdv::status::XENOM_UPDATE_STATUS_PRE_EVENT: 
	{
		command += " -status:pre_event";
		command += " -handle:";
		command += handle;
		if (str.size())
		{
			command += " -str:";
			command += str.c_str();
		}
		XdvExe((char *)command.c_str());
	}
	break;

	case xdv::status::XENOM_UPDATE_STATUS_POST_EVENT:
	{
		command += " -status:post_event";
		command += " -handle:";
		command += handle;
		if (str.size())
		{
			command += " -str:";
			command += str.c_str();
		}
		XdvExe((char *)command.c_str());
	}
	break;

	case xdv::status::XENOM_UPDATE_STSTUS_BACKSPACE: // mouse double click
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
	}
	break;

	case xdv::status::XENOM_UPDATE_STSTUS_SPACE: // mouse double click
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
	}
	break;

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