#ifndef __DEFINE_FIND_BINARY_BLOCK_DIALOG__
#define __DEFINE_FIND_BINARY_BLOCK_DIALOG__

#include <QDialog>

#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qplaintextedit.h>
#include <qcheckbox.h>

class XenomFindDialog : public QDialog
{
private:
	QGridLayout *grid_;

	QLabel *start_address_label_;
	QLineEdit *start_address_line_edit_;
	QHBoxLayout *start_address_horizontal_;

	QLabel *end_address_label_;
	QLineEdit *end_address_line_edit_;
	QHBoxLayout *end_address_horizontal_;

	QLabel *string_label_;
	QLineEdit *string_line_edit_;
	QHBoxLayout *string_horizontal_;

	QCheckBox * binary_checkbox_;
	QCheckBox * text_checkbox_;
	QHBoxLayout *checkbox_horizontal_;

	QPushButton *find_btn_;

	QString string_;
	unsigned long long ptr_;

public:
	XenomFindDialog(unsigned long long ptr, QWidget * parent = nullptr);
	~XenomFindDialog();

public:
	QString GetString();
	unsigned long long FindPattern();

	void buttonClick();
};

#endif