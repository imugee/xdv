#include "xdv_sdk.h"
#include "XenomPluginAction.h"

PluginAction::PluginAction(QString command)
	: command_(command)
{
}

PluginAction::~PluginAction()
{
}

void PluginAction::commandAction(xdv_handle viewer_handle, QString args)
{
	IViewer *viewer = (IViewer *)XdvGetObjectByHandle(viewer_handle);
	if (!viewer)
	{
		return;
	}

	QString str = this->text() + " -tag:" + args;
	if (command_.size())
	{
		QString cmd;
		cmd.sprintf("%s -handle:%x %s", command_.toStdString().c_str(), viewer_handle, str.toStdString().c_str());
		XdvExe((char *)cmd.toStdString().c_str());
	}
	else
	{
		viewer->Update(xdv::status::id::XENOM_UPDATE_STATUS_SHORTCUT, str.toStdString().c_str());
	}
}