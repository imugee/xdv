#ifndef __DEFINE_GOTO_DIALOG__
#define __DEFINE_GOTO_DIALOG__

#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <qlabel.h>
#include <QPushButton>
#include <QDialog>

class GotoDialog : public QDialog 
{
public:
	GotoDialog(QWidget * parent = Q_NULLPTR);
	~GotoDialog();

	void go();
	unsigned long long getPtr();

private:
	QGridLayout *grid_layout_;
	QHBoxLayout *horizontal_layout_;
	QLabel *label_;
	QLineEdit *line_edit_;
	QPushButton *goto_btn_;

	unsigned long long ptr_;
};

#endif
