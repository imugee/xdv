#include "XenomCommandWidget.h"

XenomCommandWidget::XenomCommandWidget(xdv_handle handle, QWidget *parent)
	: QWidget(parent)
{
	hBoxLayout_ = new QHBoxLayout(this);
	hBoxLayout_->setSpacing(6);

	label_ = new QLabel("Command > ", this);
	line_edit_ = new XenomLineEdit(handle, this);

	hBoxLayout_->addWidget(label_);
	hBoxLayout_->addWidget(line_edit_);
}