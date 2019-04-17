#include "qxnm.h"

#include <qlibrary.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qmimedata.h>
#include <io.h>

xnm * _xenom;
void setXenom(xnm *xenom)
{
	_xenom = xenom;
}

xnm *getXenom()
{
	return _xenom;
}

// -------------------------------------------
//
xnm::xnm(QWidget *parent)
	: QMainWindow(parent)
{
	QFile f(".\\exts\\css\\style.qss");
	if (f.exists())
	{
		f.open(QFile::ReadOnly | QFile::Text);
		QTextStream ts(&f);
		qApp->setStyleSheet(ts.readAll());
	}

	status_bar_ = new QStatusBar(this);
	this->setStatusBar(status_bar_);

	tool_bar_ = new QToolBar(this);
	tool_bar_->setFixedHeight(35);
	tool_bar_->setIconSize(QSize(16, 16));
	tool_bar_->setStyleSheet("QToolButton { padding-left: 0px; padding-right: 0px; }");
	this->addToolBar(tool_bar_);

	menu_bar_ = new QMenuBar(this);
	menu_bar_->setGeometry(QRect(0, 0, 756, 21));

	menu_file_ = new QMenu("File", menu_bar_);
	menu_view_ = new QMenu("View", menu_bar_);
	menu_debug_ = new QMenu("Debug", menu_bar_);
	//menu_debug_->setEnabled(false);

	menu_plugin_ = new QMenu("Plugin", menu_bar_);
	//menu_plugin_->setEnabled(false);

	menu_system_ = new QMenu("Option", menu_bar_);
	//menu_system_->setEnabled(false);

	menu_help_ = new QMenu("Help", menu_bar_);
	menu_help_->setEnabled(false);

	this->setMenuBar(menu_bar_);
	menu_bar_->addAction(menu_file_->menuAction());
	menu_bar_->addAction(menu_debug_->menuAction());
	menu_bar_->addAction(menu_view_->menuAction());
	menu_bar_->addAction(menu_plugin_->menuAction());
	menu_bar_->addAction(menu_system_->menuAction());
	menu_bar_->addAction(menu_help_->menuAction());

	//
	// set file actions
	action_open_ = new QAction("Open", this);
	action_open_->setIcon(QPixmap(":/xenom/Resources/OpenFolderBlack.ico"));
	action_open_->setIconVisibleInMenu(true);
	action_open_->setShortcut(Qt::CTRL | Qt::Key_O);

	action_new_file_ = new QAction("New", this);
	action_new_file_->setIcon(QPixmap(":/xenom/Resources/NewFile.ico"));
	action_new_file_->setIconVisibleInMenu(true);
	action_new_file_->setEnabled(false);

	action_save_ = new QAction("Save", this);
	action_save_->setIcon(QPixmap(":/xenom/Resources/FileSave.ico"));
	action_save_->setIconVisibleInMenu(true);
	action_save_->setEnabled(false);

	action_save_all_ = new QAction("Save as", this);
	action_save_all_->setIcon(QPixmap(":/xenom/Resources/SaveAs.ico"));
	action_save_all_->setIconVisibleInMenu(true);
	action_save_all_->setEnabled(false);

	menu_file_->addAction(action_open_);
	menu_file_->addAction(action_new_file_);
	menu_file_->addAction(action_save_);
	menu_file_->addAction(action_save_all_);
	menu_file_->addSeparator();

	tool_bar_->addAction(action_open_);
	tool_bar_->addAction(action_new_file_);
	tool_bar_->addAction(action_save_);
	tool_bar_->addAction(action_save_all_);
	tool_bar_->addSeparator();

	action_navigate_backward_ = new QAction("Navigate backward", this);
	action_navigate_backward_->setIcon(QPixmap(":/xenom/Resources/NBackward.ico"));
	action_navigate_backward_->setIconVisibleInMenu(true);

	action_navigate_forward_ = new QAction("Navigate forward", this);
	action_navigate_forward_->setIcon(QPixmap(":/xenom/Resources/NForward.ico"));
	action_navigate_forward_->setIconVisibleInMenu(true);

	tool_bar_->addAction(action_navigate_backward_);
	tool_bar_->addAction(action_navigate_forward_);
	tool_bar_->addSeparator();

	//
	// set debug actions
	action_process_list_ = new QAction("Process list", this);
	action_process_list_->setIcon(QPixmap(":/xenom/Resources/Workstation.ico"));
	action_process_list_->setIconVisibleInMenu(true);
	//action_process_list_->setEnabled(false);

	action_thread_list_ = new QAction("Thread list", this);
	action_thread_list_->setIcon(QPixmap(":/xenom/Resources/Threads.ico"));
	action_thread_list_->setIconVisibleInMenu(true);

	action_thread_stack_ = new QAction("Stack trace", this);
	action_thread_stack_->setIcon(QPixmap(":/xenom/Resources/Stack.ico"));
	action_thread_stack_->setIconVisibleInMenu(true);

	action_thread_register_ = new QAction("Register", this);
	action_thread_register_->setIcon(QPixmap(":/xenom/Resources/CPU.ico"));
	action_thread_register_->setIconVisibleInMenu(true);

	action_attach_process_ = new QAction("Attach process", this);
	action_attach_process_->setIcon(QPixmap(":/xenom/Resources/Attach.ico"));
	action_attach_process_->setIconVisibleInMenu(true);

	action_debuggee_run_ = new QAction("Run", this);
	action_debuggee_run_->setIcon(QPixmap(":/xenom/Resources/RunProcess.ico"));
	action_debuggee_run_->setIconVisibleInMenu(true);
	action_debuggee_run_->setShortcut(Qt::Key_F9);

	action_debuggee_stepinto_ = new QAction("Step into", this);
	action_debuggee_stepinto_->setIcon(QPixmap(":/xenom/Resources/StepInto.ico"));
	action_debuggee_stepinto_->setIconVisibleInMenu(true);
	action_debuggee_stepinto_->setShortcut(Qt::Key_F7);

	action_debuggee_stepover_ = new QAction("Step over", this);
	action_debuggee_stepover_->setIcon(QPixmap(":/xenom/Resources/StepOver.ico"));
	action_debuggee_stepover_->setIconVisibleInMenu(true);
	action_debuggee_stepover_->setShortcut(Qt::Key_F8);

	menu_debug_->addAction(action_process_list_);
	menu_debug_->addAction(action_thread_list_);
	menu_debug_->addAction(action_thread_stack_);
	menu_debug_->addAction(action_thread_register_);
	menu_debug_->addSeparator();
	menu_debug_->addAction(action_attach_process_);
	menu_debug_->addSeparator();
	menu_debug_->addAction(action_debuggee_stepinto_);
	menu_debug_->addAction(action_debuggee_stepover_);
	menu_debug_->addAction(action_debuggee_run_);

	//
	// set system actions
	action_system_ = new QAction("System", this);
	action_system_->setIcon(QPixmap(":/xenom/Resources/Tool.ico"));
	action_system_->setIconVisibleInMenu(true);
	menu_system_->addAction(action_system_);

	tool_bar_->addAction(action_process_list_);
	tool_bar_->addAction(action_thread_list_);
	tool_bar_->addAction(action_thread_stack_);
	tool_bar_->addAction(action_thread_register_);
	tool_bar_->addSeparator();

	tool_bar_->addAction(action_attach_process_);
	tool_bar_->addAction(action_debuggee_run_);
	tool_bar_->addAction(action_debuggee_stepinto_);
	tool_bar_->addAction(action_debuggee_stepover_);
	tool_bar_->addSeparator();

	char title[1024] = { 0, };
	sprintf_s(title, sizeof(title), "xenom - %s", __DATE__);
	this->setWindowTitle(title);
	this->resize(1100, 553);

	QMetaObject::connectSlotsByName(this);
	QObject::connect(action_open_, &QAction::triggered, this, &xnm::toolbarActionFileOpen);
	QObject::connect(action_process_list_, &QAction::triggered, this, &xnm::toolbarActionProcessOpen);
	QObject::connect(action_thread_list_, &QAction::triggered, this, &xnm::toolbarActionThreadList);
	QObject::connect(action_thread_stack_, &QAction::triggered, this, &xnm::toolbarActionThreadStack);
	QObject::connect(action_thread_register_, &QAction::triggered, this, &xnm::toolbarActionThreadRegister);

	QObject::connect(action_attach_process_, &QAction::triggered, this, &xnm::toolbarActionAttachProcess);
	QObject::connect(action_debuggee_run_, &QAction::triggered, this, &xnm::toolbarActionDebugRun);
	QObject::connect(action_debuggee_stepinto_, &QAction::triggered, this, &xnm::toolbarActionDebugStepInto);
	QObject::connect(action_debuggee_stepover_, &QAction::triggered, this, &xnm::toolbarActionDebugStepOver);

	QObject::connect(action_system_, &QAction::triggered, this, &xnm::toolbarActionSystemOption);

	this->setAcceptDrops(true);
}

xnm::~xnm()
{
}

// -------------------------------------------
//
QMenuBar *xnm::menuBar()
{
	return menu_bar_;
}

QToolBar *xnm::toolBar()
{
	return tool_bar_;
}

// -------------------------------------------
//
void xnm::addViewer(xdv_handle handle, XenomDockWidget *dock)
{
	viewer_table_[handle] = dock;
}

XenomDockWidget * xnm::Viewer(xdv_handle handle)
{
	std::map<xdv_handle, XenomDockWidget *>::iterator fit = viewer_table_.find(handle);
	return fit->second;
}

xdv_handle xnm::Viewer(XenomDockWidget * dock)
{
	std::map<xdv_handle, XenomDockWidget *>::iterator it = viewer_table_.begin();
	for (it; it != viewer_table_.end(); ++it)
	{
		if (it->second == dock)
		{
			return it->first;
		}
	}

	return 0;
}

// -------------------------------------------
//
void xnm::loadExts(char *exts_path)
{
	char dll[512] = { 0, };
	strcpy(dll, exts_path);
	strcat(dll, "\\*.*");

	intptr_t handle;
	_finddatai64_t fd;
	if ((handle = _findfirsti64(dll, &fd)) == -1)
	{
		return;
	}

	do {
		if (!strstr(fd.name, ".dll"))
		{
			continue;
		}

		char path[512] = { 0, };
		strcpy(path, exts_path);
		strcat(path, "\\");
		strcat(path, fd.name);

		QLibrary exts(path);
		if (exts.isLoaded())
		{
			continue;
		}

		if (!exts.load())
		{
			continue;
		}

		AddInterfaceType AddInterface = (AddInterfaceType)exts.resolve("AddInterface");
		if (AddInterface)
		{
			xdv_handle h = AddInterface();
			IObject *obj = XdvGetObjectByHandle(h);
			if (obj)
			{
				char * d = strstr(fd.name, ".");
				if (d)
				{
					std::string name(fd.name, d);
					obj->SetModuleName(name);
				}
				else
				{
					obj->SetModuleName(fd.name);
				}
			}
		}
	} while (_findnexti64(handle, &fd) == 0);
}

// -------------------------------------------
//
void xnm::Exts()
{
	std::vector<IObject *> obj_table = XdvGetObjectTable();
	for (int i = 0; i < obj_table.size(); ++i)
	{
		QAction * exts = new QAction();
		exts->setText(obj_table[i]->ObjectString().c_str());
		menu_plugin_->addAction(exts);
	}
}

// -------------------------------------------
//
QMenu * xnm::menuViewer()
{
	return menu_view_;
}

void xnm::addViewer(IViewer *obj)
{
	IViewer *v = (IViewer *)obj;
	QLibrary exts(v->ModuleName().c_str());
	if (exts.isLoaded())
	{
		xdv_handle oh = XdvGetHandleByObject(obj);
		AddInterfaceType AddInterface = (AddInterfaceType)exts.resolve("AddInterface");
		xdv_handle h = AddInterface();
		IObject * new_obj = XdvGetObjectByHandle(h);
		if (new_obj)
		{
			new_obj->SetModuleName(v->ModuleName().c_str());
			v = (IViewer *)new_obj;
			v->AddViewer();

			XenomDockWidget * od = this->Viewer(oh);
			XenomDockWidget * nd = this->Viewer(h);
			this->tabifyDockWidget(od, nd);
		}
	}
}

void xnm::addViewer(ViewerAction *obj)
{
	IViewer *v = obj->getViewer();
	if (!v->IsOpen())
	{
		v->AddViewer();
	}

	obj->setChecked(true);
}

void xnm::addViewMenuAction()
{
	std::vector<IViewer *> table = XdvGetViewerTable();
	for (int i = 0; i < table.size(); ++i)
	{
		std::string menu_name = table[i]->ObjectString();
		ViewerAction *exts_action = new ViewerAction(table[i]);

		exts_action->setObjectName(menu_name.c_str());
		exts_action->setText(menu_name.c_str());
		if (table[i]->IsCheckable())
		{
			exts_action->setCheckable(true);
		}
		QObject::connect(exts_action, &QAction::triggered, this, &xnm::toolbarActionViewerOpen);

		menu_view_->addAction(exts_action);
		addViewer(exts_action);
	}
}

void xnm::toolbarActionViewerOpen()
{
	ViewerAction *s = (ViewerAction *)sender();
	IViewer *viewer = s->getViewer();
	if (!viewer->IsCheckable())
	{
		addViewer(viewer);
		return;
	}

	if (s->isChecked() && !s->getViewer()->IsOpen())
	{
		addViewer(s);
	}
	else
	{
		if (viewer)
		{
			viewer->CloseViewer();
		}
	}
}

void xnm::addPlugin(PluginAction *action, QString menu)
{
	if (menu.size())
	{
		QMenu * m = menu_plugin_->findChild<QMenu *>(menu);
		if (!m)
		{
			m = new QMenu(menu);
		}
		m->addAction(action);
		menu_plugin_->addMenu(m);
	}
	else
	{
		menu_plugin_->addAction(action);
	}
}

// -------------------------------------------
//
QString xnm::getFileSignature(QString file_name)
{
	QFile file(file_name.toStdString().c_str());
	if (!file.open(QIODevice::ReadOnly))
	{
		return "";
	}

	// get signature
	QString signature;
	QByteArray arr = file.read(16);
	for (int i = 0; i < arr.size(); ++i)
	{
		if (!isprint(arr.at(i)))
		{
			break;
		}
		signature += arr.at(i);
	}

	return signature;
}

bool xnm::openFile(QString file_name)
{
	QString signature = getFileSignature(file_name);
	SystemDialog sd(signature);
	sd.setModal(true);
	sd.exec();

	xdv_handle ih = XdvGetParserHandle();
	if (!XdvOpenFile(ih, (char *)file_name.toStdString().c_str()))
	{
		return false;
	}

	xdv::architecture::x86::context::type ctx;
	if (XdvGetThreadContext(XdvGetParserHandle(), &ctx))
	{
		XdvExe("!segv.segall");

		XdvExe("!dasmv.dasm -ptr:%I64x", ctx.rip);
		XdvExe("!hexv.hex -ptr:%I64x", ctx.rip);
		XdvExe("!thrdv.threads");
		XdvExe("!cpuv.printctx");
		XdvExe("!stackv.printframe");

		return true;
	}

	return false;
}

void xnm::toolbarActionFileOpen()
{
	QString file_name = QFileDialog::getOpenFileName(this);
	if (!openFile(file_name))
	{
		XdvPrintLog("xenom:: %s open fail..", file_name.toStdString().c_str());
	}
	else
	{
		XdvPrintLog("xenom:: open=>%s", file_name.toStdString().c_str());
	}
}

void DebugCallback()
{
	while (1)
	{
		XdvWaitForDebugEvent();

		xdv::architecture::x86::context::type ctx;
		if (XdvGetThreadContext(XdvGetParserHandle(), &ctx))
		{
			XdvExe("!cpuv.printctx -ctx:%I64x", &ctx);
			XdvExe("!stackv.printframe -ctx:%I64x", &ctx);

			XdvExe("!dasmv.dasm -ptr:%I64x", ctx.rip);
			XdvExe("!hexv.hex -ptr:%I64x", ctx.rip);
			XdvExe("!thrdv.threads");

			DebugBreakPointId id = XdvGetBreakPointId(XdvGetParserHandle(), ctx.rip);
			if (id == DebugBreakPointId::SUSPEND_BREAK_POINT_ID)
			{
				XdvDeleteBreakPoint(XdvGetParserHandle(), ctx.rip);
			}
		}
	}
}

void xnm::toolbarActionProcessOpen()
{
	SystemDialog sd("DEBUGGER");
	sd.setModal(true);
	sd.exec();

	XenomProcessListDialog xpld;
	xpld.exec();
	if (xpld.checkConnect())
	{
		xdv::architecture::x86::context::type ctx;
		if (XdvGetThreadContext(XdvGetParserHandle(), &ctx))
		{
			xdv_handle ih = XdvGetParserHandle();
			if (XdvInstallDebugEvent(XdvProcessId(ih)))
			{
				std::thread * debug_thread = new std::thread(DebugCallback);
			}

			XdvExe("!segv.segall");

			XdvExe("!dasmv.dasm -ptr:%I64x", ctx.rip);
			XdvExe("!hexv.hex -ptr:%I64x", ctx.rip);
			XdvExe("!thrdv.threads");
			XdvExe("!cpuv.printctx");
			XdvExe("!stackv.printframe");
		}
	}
}

// -------------------------------------------
//
void xnm::toolbarActionThreadList()
{
	QList<QAction *> actions = menu_view_->actions();
	for (int i = 0; i < actions.size(); ++i)
	{
		ViewerAction * action = (ViewerAction *)actions[i];
		if (strstr(action->getViewer()->ObjectString().c_str(), "Thread"))
		{
			if (!action->getViewer()->IsOpen())
			{
				action->getViewer()->AddViewer();
				action->setChecked(true);
			}
			else
			{
				action->getViewer()->CloseViewer();
				action->setChecked(false);
			}
		}
	}
}

void xnm::toolbarActionThreadStack()
{
	QList<QAction *> actions = menu_view_->actions();
	for (int i = 0; i < actions.size(); ++i)
	{
		ViewerAction * action = (ViewerAction *)actions[i];
		if (strstr(action->getViewer()->ObjectString().c_str(), "Stack"))
		{
			if (!action->getViewer()->IsOpen())
			{
				action->getViewer()->AddViewer();
				action->setChecked(true);
			}
			else
			{
				action->getViewer()->CloseViewer();
				action->setChecked(false);
			}
		}
	}
}

void xnm::toolbarActionThreadRegister()
{
	QList<QAction *> actions = menu_view_->actions();
	for (int i = 0; i < actions.size(); ++i)
	{
		ViewerAction * action = (ViewerAction *)actions[i];
		if (strstr(action->getViewer()->ObjectString().c_str(), "Registers"))
		{
			if (!action->getViewer()->IsOpen())
			{
				action->getViewer()->AddViewer();
				action->setChecked(true);
			}
			else
			{
				action->getViewer()->CloseViewer();
				action->setChecked(false);
			}
		}
	}
}

void xnm::toolbarActionAttachProcess()
{
	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this, "xenom", "Do you want to attach process?",
		QMessageBox::Yes | QMessageBox::No);

	if (reply == QMessageBox::Yes)
	{
		xdv_handle ih = XdvGetParserHandle();
		if (XdvAttachProcess(ih, XdvProcessId(ih)))
		{
			xdv::architecture::x86::context::type ctx;
			if (XdvGetThreadContext(XdvGetParserHandle(), &ctx))
			{
				XdvExe("!segv.segall");

				XdvExe("!cpuv.printctx -ctx:%I64x", &ctx);
				XdvExe("!stackv.printframe -ctx:%I64x", &ctx);

				XdvExe("!dasmv.dasm -ptr:%I64x", ctx.rip);
				XdvExe("!hexv.hex -ptr:%I64x", ctx.rip);
				XdvExe("!thrdv.threads");
			}


			QMessageBox::information(this, "xenom", "Success");
		}
		else
		{
			QMessageBox::information(this, "xenom", "f");
		}
	}
}

void xnm::toolbarActionDebugRun()
{
	XdvRunningProcess(XdvGetParserHandle());
}

void xnm::toolbarActionDebugStepInto()
{
	XdvStepInto(XdvGetParserHandle(), nullptr, nullptr);
	xdv::architecture::x86::context::type ctx;
	if (XdvGetThreadContext(XdvGetParserHandle(), &ctx))
	{
		XdvExe("!cpuv.printctx");
		XdvExe("!segv.segall");

		XdvExe("!dasmv.dasm -ptr:%I64x", ctx.rip);
		XdvExe("!hexv.hex -ptr:%I64x", ctx.rip);
		XdvExe("!thrdv.threads");
		XdvExe("!stackv.printframe");
	}
}

void xnm::toolbarActionDebugStepOver()
{
	XdvStepOver(XdvGetParserHandle(), nullptr, nullptr);
	xdv::architecture::x86::context::type ctx;
	if (XdvGetThreadContext(XdvGetParserHandle(), &ctx))
	{
		XdvExe("!cpuv.printctx");

		XdvExe("!dasmv.dasm -ptr:%I64x", ctx.rip);
		XdvExe("!hexv.hex -ptr:%I64x", ctx.rip);
		XdvExe("!thrdv.threads");
		XdvExe("!stackv.printframe");
	}
}

void xnm::toolbarActionSystemOption()
{
	SystemDialog sd("DEBUGGER");
	sd.setModal(true);
	sd.exec();

	XdvUpdateDebuggee(XdvGetParserHandle());
}

void xnm::dragEnterEvent(QDragEnterEvent *e)
{
	if (e->mimeData()->hasUrls()) 
	{
		e->acceptProposedAction();
	}
}

void xnm::dropEvent(QDropEvent *e)
{
	foreach(const QUrl &url, e->mimeData()->urls()) 
	{
		QString file_name = url.toLocalFile();
		if (!openFile(file_name))
		{
			XdvPrintLog("xenom:: %s open fail..", file_name.toStdString().c_str());
		}
		else
		{
			XdvPrintLog("xenom:: open=>%s", file_name.toStdString().c_str());
		}
	}
}

// -------------------------------------------
//
EXTS_FUNC(xenom)
{
	int qargc = argc;
	char * qargv[100] = { 0, };
	for (int i = 0; i < qargc; ++i)
	{
		qargv[i] = argv[i];
	}

	QCoreApplication::addLibraryPath(".\\exts\\qt\\icons");
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication a(qargc, qargv);
	a.setWindowIcon(QPixmap(":/xenom/Resources/xenom_3d.ico"));

	xnm w;
	w.show();

	setXenom(&w);
	w.loadExts(".\\exts\\arch");
	w.loadExts(".\\exts\\parser");
	w.loadExts(".\\exts\\viewer");
	w.loadExts(".\\exts");
	//w.Exts();
	w.addViewMenuAction();

	//
	// first callback
	XdvExe("!stylev.style");

	std::vector<IViewer *> vt = XdvGetViewerTable();
	for (int i = 0; i < vt.size(); ++i)
	{
		std::string cmd = "!" + vt[i]->ModuleName() + ".update";
		XdvExe((char *)cmd.c_str());
	}

	return ullvar((unsigned long long)a.exec());
}
