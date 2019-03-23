#ifndef __DEFINE_XENOM_ACTION_TYPE__
#define __DEFINE_XENOM_ACTION_TYPE__

#include <qaction.h>

class ViewerAction : public QAction
{
private:
	IViewer *viewer_;

public:
	ViewerAction(IViewer *viewer);
	~ViewerAction();

	IViewer *getViewer();
};

#endif