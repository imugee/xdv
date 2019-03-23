#include "XenomDockWidget.h"
#include "qxnm.h"

XenomDockWidget::XenomDockWidget(QWidget *parent) 
	: QDockWidget(parent), tabWidget_(nullptr), gridLayout_(nullptr), vBoxLayout_(nullptr), hBoxLayout_(nullptr), text_viewer_(nullptr)
{
	contents_ = new QWidget();

	gridLayout_ = new QGridLayout(contents_);
	gridLayout_->setSpacing(6);
	gridLayout_->setContentsMargins(11, 11, 11, 11);

	//
	//this->setFeatures(DockWidgetFloatable | DockWidgetMovable); // disable close button
	this->setWidget(contents_);
}

XenomDockWidget::~XenomDockWidget()
{
}

//
//
int XenomDockWidget::addTab(QString tabString, QWidget *tabWidget)
{
	if (!tabWidget_)
	{
		tabWidget_ = new QTabWidget(contents_);
		tabWidget_->setTabsClosable(true);
		QObject::connect(tabWidget_, &QTabWidget::tabCloseRequested, this, &XenomDockWidget::closeTab);
		gridLayout_->addWidget(tabWidget_, 0, 0, 1, 1);
	}

	QWidget *tab_contents = new QWidget();
	QGridLayout *tab_layout = new QGridLayout(tab_contents);

	tabWidget->setParent(tab_contents);
	tab_layout->setSpacing(6);
	tab_layout->setContentsMargins(11, 11, 11, 11);
	tab_layout->addWidget(tabWidget);

	return tabWidget_->addTab(tab_contents, tabString);
}

void XenomDockWidget::closeTab(int idx)
{
	if (idx != 0)
	{
		tabWidget_->removeTab(idx);
	}
}

//
//
void XenomDockWidget::addDefault(QWidget *widget)
{
	gridLayout_->addWidget(widget, 0, 0, 1, 1);
}

void XenomDockWidget::addVbox(QWidget *widget)
{
	if (!vBoxLayout_)
	{
		vBoxLayout_ = new QVBoxLayout();
		vBoxLayout_->setSpacing(6);
		gridLayout_->addLayout(vBoxLayout_, 0, 0, 1, 1);
	}

	widget->setParent(contents_);
	vBoxLayout_->addWidget(widget);
}

void XenomDockWidget::addHbox(QWidget *widget)
{
	if (!hBoxLayout_)
	{
		hBoxLayout_ = new QHBoxLayout();
		hBoxLayout_->setSpacing(6);
		gridLayout_->addLayout(hBoxLayout_, 0, 0, 1, 1);
	}

	widget->setParent(contents_);
	hBoxLayout_->addWidget(widget);
}

void XenomDockWidget::raiseDock()
{
	this->raise();
	if (tabWidget_)
	{
		this->tabWidget_->setCurrentIndex(this->tabWidget_->count() - 1);
	}
}

QPlainTextEdit * XenomDockWidget::TextViewer()
{
	return text_viewer_;
}

void XenomDockWidget::setTextViewer(QPlainTextEdit *text_viewer)
{
	text_viewer_ = text_viewer;
}

void XenomDockWidget::closeEvent(QCloseEvent *e)
{
	xnm *xenom = getXenom();
	xdv_handle handle = xenom->Viewer(this);

	IViewer * viewer = (IViewer *)XdvGetObjectByHandle(handle);
	if (viewer)
	{
		viewer->CloseViewer();
		viewer->ObjectString();
		QList<QAction *> actions = xenom->menuViewer()->actions();
		QAction * action = nullptr;
		for (int i = 0; i < actions.size(); ++i)
		{
			if(strstr(actions[i]->objectName().toStdString().c_str(),
				viewer->ObjectString().c_str()))
			{
				action = actions[i];
				break;
			}
		}

		if (action)
		{
			if (action->isChecked())
			{
				action->setChecked(false);
			}
		}
	}
}