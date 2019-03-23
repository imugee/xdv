#include "XenomProcessListDialog.h"
#include "xdv_sdk.h"

XenomProcessListDialog::XenomProcessListDialog(QWidget * parent)
	: QDialog(parent), connect_(false)
{
	grid_ = new QGridLayout(this);
	grid_->setSpacing(6);
	grid_->setContentsMargins(11, 11, 11, 11);
	grid_->setObjectName(QStringLiteral("gridLayout"));

	//
	parser_label_ = new QLabel("Process list", this);
	list_view_ = new QListWidget(this);
	list_view_->setSpacing(list_view_->spacing() / 2);
	list_view_vertical_ = new QVBoxLayout();
	list_view_vertical_->setSpacing(6);
	list_view_vertical_->addWidget(parser_label_);
	list_view_vertical_->addWidget(list_view_);

	grid_->addLayout(list_view_vertical_, 0, 0, 1, 2);

	//
	base_address_label_ = new QLabel("Wait for process", this);
	line_edit_ = new QLineEdit(this);
	line_edit_vertical_ = new QVBoxLayout();
	line_edit_vertical_->setSpacing(6);
	line_edit_vertical_->addWidget(base_address_label_);
	line_edit_vertical_->addWidget(line_edit_);

	grid_->addLayout(line_edit_vertical_, 2, 0, 1, 1);

	//
	open_btn_ = new QPushButton("Open process", this);
	grid_->addWidget(open_btn_, 4, 0, 1, 1);

	this->resize(250, 500);
	this->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

	QObject::connect(open_btn_, &QPushButton::clicked, this, &XenomProcessListDialog::OpenProcess);

	//
	std::map<unsigned long, std::string> process_map = XdvProcessList(XdvGetParserHandle());
	for (auto it : process_map)
	{
		if (it.first == 4 || it.first == 0)
		{
			continue;
		}

		char print[500] = { 0, };
		sprintf_s(print, sizeof(print), "%d %s\n", it.first, it.second.c_str());
		list_view_->addItem(print);
	}
}

void XenomProcessListDialog::AttachProcess()
{
	unsigned long pid = 0;
	if (line_edit_->text().size())
	{
		pid = XdvWaitForProcess(XdvGetParserHandle(), line_edit_->text().toStdString());
	}
	else if (list_view_->currentIndex().row() != -1)
	{
		QString select_process = list_view_->item(list_view_->currentIndex().row())->text();
		char * end = nullptr;
		pid = strtoul(select_process.toStdString().c_str(), &end, 10);
	}

	if (XdvAttachProcess(XdvGetParserHandle(), pid))
	{
		connect_ = true;
	}

	this->close();
}

void XenomProcessListDialog::OpenProcess()
{
	unsigned long pid = 0;
	if (line_edit_->text().size())
	{
		pid = XdvWaitForProcess(XdvGetParserHandle(), line_edit_->text().toStdString());
	}
	else if (list_view_->currentIndex().row() != -1)
	{
		QString select_process = list_view_->item(list_view_->currentIndex().row())->text();
		char * end = nullptr;
		pid = strtoul(select_process.toStdString().c_str(), &end, 10);
	}

	if (XdvOpenProcess(XdvGetParserHandle(), pid))
	{
		connect_ = true;
	}

	this->close();
}

bool XenomProcessListDialog::checkConnect()
{
	return connect_;
}