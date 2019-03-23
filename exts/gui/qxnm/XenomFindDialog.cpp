#include "XenomFindDialog.h"
#include "xdv_sdk.h"

XenomFindDialog::XenomFindDialog(unsigned long long ptr, QWidget * parent)
	: QDialog(parent), ptr_(ptr)
{
	grid_ = new QGridLayout(this);
	grid_->setSpacing(6);
	grid_->setContentsMargins(11, 11, 11, 11);
	grid_->setObjectName(QStringLiteral("gridLayout"));

	start_address_label_ = new QLabel(" Start address : ", this);
	start_address_line_edit_ = new QLineEdit(this);
	start_address_horizontal_ = new QHBoxLayout();
	start_address_horizontal_->setSpacing(6);
	start_address_horizontal_->addWidget(start_address_label_);
	start_address_horizontal_->addWidget(start_address_line_edit_);
	grid_->addLayout(start_address_horizontal_, 0, 0, 1, 1);

	end_address_label_ = new QLabel("  End address : ", this);
	end_address_line_edit_ = new QLineEdit(this);
	end_address_horizontal_ = new QHBoxLayout();
	end_address_horizontal_->setSpacing(6);
	end_address_horizontal_->addWidget(end_address_label_);
	end_address_horizontal_->addWidget(end_address_line_edit_);
	grid_->addLayout(end_address_horizontal_, 1, 0, 1, 1);

	string_label_ = new QLabel(" String : ", this);
	string_line_edit_ = new QLineEdit(this);
	string_horizontal_ = new QHBoxLayout();
	string_horizontal_->setSpacing(6);
	string_horizontal_->addWidget(string_label_);
	string_horizontal_->addWidget(string_line_edit_);
	grid_->addLayout(string_horizontal_, 2, 0, 1, 1);

	find_btn_ = new QPushButton("Find", this);
	grid_->addWidget(find_btn_, 3, 0, 1, 1);

	QObject::connect(find_btn_, &QPushButton::clicked, this, &XenomFindDialog::buttonClick);

	if (ptr_ == 0)
	{
		start_address_line_edit_->setText("0");
		xdv_handle ah = XdvGetArchitectureHandle();
		IObject *obj = XdvGetObjectByHandle(ah);
		if (xdv::object::id::XENOM_X86_ARCHITECTURE_OBJECT == obj->ObjectType()
			|| xdv::object::id::XENOM_X86_ANALYZER_OBJECT == obj->ObjectType())
		{
			end_address_line_edit_->setText("0x7fffffff");
		}
		else
		{
			end_address_line_edit_->setText("0x7fffffffffffffff");
		}
	}
	else
	{
		xdv_handle ih = XdvGetParserHandle();
		xdv::memory::type mbi;
		if (XdvQueryMemory(ih, ptr_, &mbi))
		{
			char start_str[100] = { 0, };
			sprintf(start_str, "%I64x", mbi.BaseAddress);

			char end_str[100] = { 0, };
			sprintf(end_str, "%I64x", mbi.BaseAddress + mbi.RegionSize);

			start_address_line_edit_->setText(start_str);
			end_address_line_edit_->setText(end_str);
		}
	}
}

XenomFindDialog::~XenomFindDialog()
{
}

void XenomFindDialog::buttonClick()
{
	string_ = string_line_edit_->text().toStdString().c_str();
	this->close();
}

//
//
QString XenomFindDialog::GetString()
{
	return string_;
}

unsigned long long XenomFindDialog::FindPattern()
{
	char *end = nullptr;
	unsigned long long start_ptr = strtoull(start_address_line_edit_->text().toStdString().c_str(), &end, 16);
	unsigned long long end_ptr = strtoull(end_address_line_edit_->text().toStdString().c_str(), &end, 16);

	size_t pattern_size = string_label_->text().size();
	unsigned char *pattern = (unsigned char *)malloc(pattern_size);
	if (!pattern)
	{
		return 0;
	}
	std::shared_ptr<void> pattern_closer(pattern, free);
	memset(pattern, 0, pattern_size);

	const char *pt = string_.toStdString().c_str();
	unsigned long long j = 0;
	for (unsigned long long i = 0; i < pattern_size; ++i)
	{
		if (pt[i] == '?')
		{
			pattern[j++] = '?';
		}
		else if (pt[i] != ' ')
		{
			char *end = nullptr;
			pattern[j++] = (unsigned char)strtol(&pt[i], &end, 16);
			i = end - pt;
		}
	}

	unsigned long long ptr = 0;
	xdv_handle ih = XdvGetParserHandle();
	do
	{
		xdv::memory::type mbi;
		if (XdvQueryMemory(ih, start_ptr, &mbi))
		{
			start_ptr += mbi.RegionSize;

			unsigned char *buffer = (unsigned char *)malloc((size_t)mbi.RegionSize);
			if (!buffer)
			{
				continue;
			}
			std::shared_ptr<void> buffer_closer(buffer, free);
			memset(buffer, 0, (size_t)mbi.RegionSize);

			unsigned long long read = XdvReadMemory(ih, ptr_, buffer, mbi.RegionSize);
			if (read == 0)
			{
				continue;
			}

			unsigned char * f = (unsigned char *)XdvFindPattern(buffer, read, pattern, j);
			if (f)
			{
				unsigned long offset = f - buffer;
				ptr = offset + ptr_;
				break;
			}
		}
		else
		{
			break;
		}
	} while (start_ptr < end_ptr);

	return ptr;
}
