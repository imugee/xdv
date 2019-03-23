#include "xdv_sdk.h"
#include "ViewerAction.h"

ViewerAction::ViewerAction(IViewer *viewer) : viewer_(viewer)
{
}

ViewerAction::~ViewerAction()
{
}

IViewer *ViewerAction::getViewer()
{
	return viewer_;
}