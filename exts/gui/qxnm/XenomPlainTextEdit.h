#ifndef _DEFINE_XENOM_PLAINTEXT_EDIT
#define _DEFINE_XENOM_PLAINTEXT_EDIT

#include <qplaintextedit.h>

class XenomPlainTextEdit : public QPlainTextEdit
{
public:
	XenomPlainTextEdit(QWidget *parent) : QPlainTextEdit(parent) {};

	virtual void updateText(QString string) = 0;
	virtual void clearText() = 0;
};

#endif