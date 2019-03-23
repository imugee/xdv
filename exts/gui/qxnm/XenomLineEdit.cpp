#include "XenomLineEdit.h"
#include <qevent.h>
#include <qtextcursor.h>

XenomLineEdit::XenomLineEdit(xdv_handle handle, QWidget *parent)
	: handle_(handle)
{
}

void XenomLineEdit::keyPressEvent(QKeyEvent *e)
{
	switch (e->key())
	{
	case Qt::Key_Return:
		if (this->text().size())
		{
			std::string cmd = this->text().toStdString();
			cmd += " -handle:%x ";
			XdvExe((char *)cmd.c_str(), handle_);
		}
		this->clear();
		return;
	}

	QLineEdit::keyPressEvent(e);
}
