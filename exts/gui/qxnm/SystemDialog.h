#ifndef __DEFINE_XENOM_SYSTEM_DIALOG__
#define __DEFINE_XENOM_SYSTEM_DIALOG__

#include <qdialog.h>
#include <qgridlayout.h>
#include <qlabel.h>
#include <qlistwidget.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlineedit.h>

class SystemDialog : public QDialog
{
	//Q_OBJECT

public:
	SystemDialog(QString, QWidget * parent = Q_NULLPTR);
	~SystemDialog();

public:
	QGridLayout *grid_;

	QLabel *parser_label_;
	QListWidget *list_view_;
	QVBoxLayout *list_view_vertical_;

	QLabel *architecture_label_;
	QComboBox *combo_box_;
	QVBoxLayout *combo_box_vertical_;

	QLabel *base_address_label_;
	QLineEdit *line_edit_;
	QHBoxLayout *line_edit_horizontal_;

	QPushButton *btn_;

	std::map<std::string, void *> engine_map_;
	void *parser_;
	void *arch_;

public:
	void addParsers(QString signature);
	void addArchs();

	void *getParser();
	void *getArch();

	public slots:
	void Run();
};

#endif