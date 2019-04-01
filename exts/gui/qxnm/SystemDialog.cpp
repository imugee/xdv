#include "SystemDialog.h"
#include "xdv_sdk.h"

SystemDialog::SystemDialog(QString signature, QWidget * parent) : QDialog(parent), parser_(nullptr), arch_(nullptr)
{
	grid_ = new QGridLayout(this);
	grid_->setSpacing(6);
	grid_->setContentsMargins(11, 11, 11, 11);
	grid_->setObjectName(QStringLiteral("gridLayout"));

	//
	//
	parser_label_ = new QLabel("Parser", this);
	list_view_ = new QListWidget(this);
	list_view_vertical_ = new QVBoxLayout();
	list_view_vertical_->setSpacing(6);
	list_view_vertical_->addWidget(parser_label_);
	list_view_vertical_->addWidget(list_view_);

	grid_->addLayout(list_view_vertical_, 0, 0, 1, 2);

	//
	//
	architecture_label_ = new QLabel("Architecture", this);
	combo_box_ = new QComboBox(this);
	combo_box_vertical_ = new QVBoxLayout();
	combo_box_vertical_->setSpacing(6);
	combo_box_vertical_->addWidget(architecture_label_);
	combo_box_vertical_->addWidget(combo_box_);

	grid_->addLayout(combo_box_vertical_, 1, 0, 1, 1);

	//
	//
	base_address_label_ = new QLabel("Offset", this);
	line_edit_ = new QLineEdit(this);
	line_edit_->setDisabled(true);
	line_edit_horizontal_ = new QHBoxLayout();
	line_edit_horizontal_->setSpacing(6);
	line_edit_horizontal_->addWidget(base_address_label_);
	line_edit_horizontal_->addWidget(line_edit_);

	grid_->addLayout(line_edit_horizontal_, 2, 0, 1, 1);

	//
	//
	btn_ = new QPushButton("Run", this);
	grid_->addWidget(btn_, 3, 0, 1, 1);
	QObject::connect(btn_, &QPushButton::clicked, this, &SystemDialog::Run);

	//
	//
	this->resize(500, 250);
	this->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

	//
	//
	addParsers(signature);
	addArchs();
}

SystemDialog::~SystemDialog()
{
}

void SystemDialog::addParsers(QString signature)
{
	std::vector<IParser *> table = XdvGetParserTable();
	for (int i = 0; i < table.size(); ++i)
	{
		std::string obj_str = table[i]->ObjectString();
		if (strstr(obj_str.c_str(), signature.toStdString().c_str()))
		{
			//QString parser_string = "Supported Format : ";
			//parser_string += obj_str.c_str();
			list_view_->addItem(obj_str.c_str());

			engine_map_.insert(std::pair<std::string, void *>(obj_str.c_str(), table[i]));
		}
	}

	if (list_view_->item(0))
	{
		list_view_->item(0)->setSelected(true);
	}
}

void SystemDialog::addArchs()
{
	std::vector<IArchitecture *> table = XdvGetArchitectureTable();
	for (int i = 0; i < table.size(); ++i)
	{
		std::string obj_str = table[i]->ObjectString();

		QString arch_string = "Supported Architecture : ";
		arch_string += obj_str.c_str();
		combo_box_->addItem(arch_string.toStdString().c_str());

		engine_map_.insert(std::pair<std::string, void *>(arch_string.toStdString().c_str(), table[i]));
	}
}

void SystemDialog::Run()
{
	if (list_view_->count() && combo_box_->count())
	{
		if (list_view_->currentIndex().row() != -1)
		{
			std::map<std::string, void *>::iterator fit = engine_map_.find(list_view_->item(list_view_->currentIndex().row())->text().toStdString().c_str());
			if (fit != engine_map_.end())
			{
				parser_ = fit->second;
				XdvSetParserHandle((IObject *)parser_);
			}
		}

		if (combo_box_->currentIndex() != -1)
		{
			std::map<std::string, void *>::iterator fit = engine_map_.find(combo_box_->currentText().toStdString().c_str());
			if (fit != engine_map_.end())
			{
				arch_ = fit->second;
				XdvSetArchitectureHandle((IObject *)arch_);
			}
		}
	}

	this->close();
}

void * SystemDialog::getParser()
{
	return parser_;
}

void * SystemDialog::getArch()
{
	return arch_;
}
