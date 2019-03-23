#ifndef __DEFINE_COMMAND_WIDGET__
#define __DEFINE_COMMAND_WIDGET__

#include "xdv_sdk.h"

#include <qwidget.h>
#include <qlabel.h>
#include <qlayout.h>

#include "XenomLineEdit.h"

class XenomCommandWidget : public QWidget
{
private:
	QHBoxLayout *hBoxLayout_;
	QLabel * label_;
	XenomLineEdit * line_edit_;

public:
	explicit XenomCommandWidget(xdv_handle handle, QWidget *parent = 0);
};

#endif