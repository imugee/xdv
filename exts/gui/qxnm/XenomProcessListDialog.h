#ifndef __XENOM_PROCESS_LIST_DIALOG__
#define __XENOM_PROCESS_LIST_DIALOG__

#include <qdialog.h>
#include <qgridlayout.h>
#include <qlabel.h>
#include <qlistwidget.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlineedit.h>

class XenomProcessListDialog : public QDialog
{
private:
	QGridLayout *grid_;

	QLabel *parser_label_;
	QListWidget *list_view_;
	QVBoxLayout *list_view_vertical_;

	QLabel *base_address_label_;
	QLineEdit *line_edit_;
	QVBoxLayout *line_edit_vertical_;

	QPushButton *open_btn_;

	bool connect_;

public:
	XenomProcessListDialog(QWidget * parent = nullptr);
	void AttachProcess();
	void OpenProcess();
	bool checkConnect();
};

#endif