#ifndef __DEFINE_XENOM_LINE_EDIT__
#define __DEFINE_XENOM_LINE_EDIT__

#include "xdv_sdk.h"

#include <qaction.h>
#include <qlineedit.h>

class XenomLineEdit : public QLineEdit
{
private:
	xdv_handle handle_;

public:
	explicit XenomLineEdit(xdv_handle handle, QWidget *parent = 0);
	virtual void keyPressEvent(QKeyEvent *e) override;
};

#endif