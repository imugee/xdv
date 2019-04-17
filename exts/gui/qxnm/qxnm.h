#ifndef QXNM_H
#define QXNM_H

#include <qapplication.h>
#include <qmainwindow.h>
#include <qstatusbar.h>
#include <qtoolbar.h>
#include <qmenubar.h>
#include <qmenu.h>
#include <qaction.h>
#include <qfile>
#include <qtextstream.h>

#include "XenomPlainTextEdit.h"
#include "XenomTextViewer.h"
#include "XenomDockWidget.h"
#include "XenomCommandWidget.h"
#include "ViewerAction.h"
#include "SystemDialog.h"
#include "XenomProcessListDialog.h"

#include "xdv_sdk.h"

#include "XenomPluginAction.h"

#pragma comment(lib, "corexts.lib")

class xnm : public QMainWindow
{
public:
	xnm(QWidget *parent = 0);
	~xnm();

private:
	QStatusBar *status_bar_;
	QMenuBar *menu_bar_;
	QToolBar *tool_bar_;

	QMenu *menu_file_;
	QMenu *menu_debug_;
	QMenu *menu_view_;
	QMenu *menu_plugin_;
	QMenu *menu_system_;
	QMenu *menu_help_;

	QAction *action_open_;
	QAction *action_new_file_;
	QAction *action_save_;
	QAction *action_save_all_;
	QAction *action_option_;

	QAction *action_navigate_backward_;
	QAction *action_navigate_forward_;

	QAction *action_process_list_;
	QAction *action_thread_list_;
	QAction *action_thread_stack_;
	QAction *action_thread_register_;

	QAction *action_attach_process_;

	QAction *action_debuggee_stepinto_;
	QAction *action_debuggee_stepover_;
	QAction *action_debuggee_run_;

	QAction *action_system_;

	std::map<xdv_handle, XenomDockWidget *> viewer_table_;

protected:
	virtual void dragEnterEvent(QDragEnterEvent *e) override;
	virtual void dropEvent(QDropEvent *e) override;

public:
	QMenuBar *menuBar();
	QToolBar *toolBar();

public:
	QString getFileSignature(QString file_name);
	bool openFile(QString file_name);

public:
	void addViewer(xdv_handle handle, XenomDockWidget *dock);
	XenomDockWidget *Viewer(xdv_handle handle);
	xdv_handle Viewer(XenomDockWidget * dock);

	void loadExts(char *exts_path);

	void Exts();

	void addViewer(IViewer *obj);
	void addViewer(ViewerAction *obj);
	void addViewMenuAction();
	void toolbarActionViewerOpen();

	void addPlugin(PluginAction *, QString menu);

	void toolbarActionFileOpen();
	void toolbarActionProcessOpen();

	void toolbarActionThreadList();
	void toolbarActionThreadStack();
	void toolbarActionThreadRegister();
	void toolbarActionAttachProcess();

	void toolbarActionDebugRun();
	void toolbarActionDebugStepInto();
	void toolbarActionDebugStepOver();

	void toolbarActionSystemOption();

	QMenu * menuViewer();
};

void setXenom(xnm *xenom);
xnm *getXenom();

#endif // QXNM_H
