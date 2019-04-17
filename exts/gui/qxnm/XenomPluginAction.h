#ifndef __DEFINE_XENOM_PLUGIN_ACTION_TYPE__
#define __DEFINE_XENOM_PLUGIN_ACTION_TYPE__

#include <qaction.h>

class PluginAction : public QAction
{
private:
	QString command_;

public:
	PluginAction(QString command);
	~PluginAction();

	void commandAction(xdv_handle viewer_handle, QString args);
};

#endif