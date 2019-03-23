#ifndef __XENOM_DOCK_WIDGET__
#define __XENOM_DOCK_WIDGET__

#include <qdockwidget.h>
#include <qgridlayout.h>
#include <qtablewidget.h>
#include <qprogressbar.h>
#include <qplaintextedit.h>

class XenomDockWidget : public QDockWidget
{
public:
	explicit XenomDockWidget(QWidget *parent = 0);
	~XenomDockWidget();

public:
	QWidget *contents_;
	QGridLayout *gridLayout_;
	QVBoxLayout *vBoxLayout_;
	QHBoxLayout *hBoxLayout_;
	QTabWidget *tabWidget_;
	QPlainTextEdit *text_viewer_;

public:
	int addTab(QString tabString, QWidget *tabWidget);
	void closeTab(int);

	void addDefault(QWidget *widget);
	void addVbox(QWidget *widget);
	void addHbox(QWidget *widget);

public:
	QPlainTextEdit *TextViewer();
	void setTextViewer(QPlainTextEdit *text_viewer);
	void raiseDock();

protected:
	void closeEvent(QCloseEvent *event) override;
};

#endif