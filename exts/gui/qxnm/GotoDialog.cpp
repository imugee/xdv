#include "GotoDialog.h"

GotoDialog::GotoDialog(QWidget * parent) 
	: QDialog(parent), ptr_(0)
{
	grid_layout_ = new QGridLayout(this);
	grid_layout_->setSpacing(6);
	grid_layout_->setContentsMargins(11, 11, 11, 11);

	horizontal_layout_ = new QHBoxLayout();
	horizontal_layout_->setSpacing(6);

	label_ = new QLabel(this);
	label_->setText("Address");
	horizontal_layout_->addWidget(label_);

	line_edit_ = new QLineEdit(this);
	horizontal_layout_->addWidget(line_edit_);

	goto_btn_ = new QPushButton("Jump", this);
	horizontal_layout_->addWidget(goto_btn_);

	grid_layout_->addLayout(horizontal_layout_, 0, 0, 1, 1);
	QObject::connect(goto_btn_, &QPushButton::clicked, this, &GotoDialog::go);

	this->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
	this->resize(500, 50);
}

GotoDialog::~GotoDialog()
{
}

#include "xdv_sdk.h"

void GotoDialog::go()
{
	xdv_handle ih = XdvGetParserHandle();
	QString str = line_edit_->text();
	QString exp_str;
	{
		QStringList filter = str.split(QRegExp("[\r\n\t ]+"), QString::SkipEmptyParts);
		for (int i = 0; i < filter.size(); ++i)
		{
			exp_str += filter[i];
		}
	}

	unsigned long long result[30] = { 0, };
	QStringList filter = exp_str.split(QRegExp("[+-]+"), QString::SkipEmptyParts);
	for (int i = 0; i < filter.size(); ++i)
	{
		bool s = false;
		result[i] = filter[i].toULongLong(&s, 16);
		if (!s)
		{
			result[i] = XdvGetSymbolPointer(ih, (char *)filter[i].toStdString().c_str());
		}
	}

	int idx = 0;
	unsigned long long ptr = result[idx++];
	for (int i = 0; i < exp_str.size(); ++i)
	{
		unsigned char c = exp_str.at(i).cell();
		switch (c)
		{
		case '+':
			ptr += result[idx++];
			break;

		case '-':
			ptr -= result[idx++];
			break;
		}
	}

	ptr_ = ptr;
	this->close();
}

unsigned long long GotoDialog::getPtr()
{
	return ptr_;
}
