#include "qxnm.h"

#include <qpen.h>
#include <qfontmetrics.h>
#include <qpainter.h>

XenomTextViewer::XenomTextViewer(xdv_handle handle, xdv::viewer::id id, QWidget *parent)
	: XenomPlainTextEdit(parent), viewer_handle_(handle), id_(id)
{
	setFont(QFont("Consolas", 9));
	setReadOnly(true);
	setMouseTracking(true);
	setWordWrapMode(QTextOption::NoWrap);
	highlighter_ = new SyntaxHighlighter(this->document());

	if (id_ == xdv::viewer::id::TEXT_VIEWER_DASM)
	{
		navigationArea_ = new NavigationLineArea(this);
		QObject::connect(this, &QPlainTextEdit::updateRequest, this, &XenomTextViewer::updateBlockArea);
		updateBlockAreaWidth(0);
	}

	line_color_ = QColor("#509CE4").light(160);
}

XenomTextViewer::~XenomTextViewer()
{
}

//
//
void mnString(unsigned long long ptr, std::string &mnstr)
{
	xdv_handle ah = XdvGetArchitectureHandle();
	xdv_handle ih = XdvGetParserHandle();

	unsigned long long tmp = ptr;
	for (int i = 0; i < 2; ++i)
	{
		tmp = XdvGetBeforePtr(ah, ih, tmp);
		if (tmp == 0)
		{
			return;
		}
	}

	unsigned long long start = ptr;
	if (tmp)
	{
		start = tmp;
	}

	for (int i = 0; i < 4; ++i)
	{
		XdvExe("!dasmv.navistr -ptr:%I64x -buf:%p", start, &mnstr);
		if (start == ptr)
		{
			mnstr += "> ";
		}
		XdvExe("!dasmv.codestr -ptr:%I64x -buf:%p", start, &mnstr);
		mnstr += "\n";

		xvar sizevar = XdvExe("!dasmv.codesize -ptr:%I64x", start);
		unsigned long long size = ullvar(sizevar);
		if (size == 0)
		{
			break;
		}
		start += size;
	}
}

bool XenomTextViewer::event(QEvent *e)
{
	if (id_ != xdv::viewer::id::TEXT_VIEWER_DASM)
	{
		return QPlainTextEdit::event(e);
	}

	if (e->type() != QEvent::ToolTip)
	{
		return QPlainTextEdit::event(e);
	}

	IObject *current_object = XdvGetObjectByHandle(viewer_handle_);
	if (!current_object)
	{
		return QPlainTextEdit::event(e);
	}

	QHelpEvent *he = (QHelpEvent *)e;
	QTextCursor cursor = cursorForPosition(he->pos());
	cursor.select(QTextCursor::WordUnderCursor);

#if 1
	if (!cursor.selectedText().isEmpty())
	{
		std::string str = cursor.selectedText().toStdString();
		const char * text = str.c_str();
		if (strstr(text, "0x"))
		{
			char *end = nullptr;
			unsigned long long ptr = strtoull(text, &end, 16);

			xdv_handle ah = XdvGetArchitectureHandle();
			xdv_handle ih = XdvGetParserHandle();

			unsigned long long tmp = ptr;
			for (int i = 0; i < 2; ++i)
			{
				tmp = XdvGetBeforePtr(ah, ih, tmp);
				if (tmp == 0)
				{
					return "";
				}
			}

			unsigned long long start = ptr;
			if (tmp)
			{
				start = tmp;
			}
			
			std::string mnstr;
			mnString(ptr, mnstr);

			QToolTip::setFont(QFont("Consolas", 9));
			QToolTip::showText(he->globalPos(), mnstr.c_str());

			return true;
		}
	}
#endif

	return QPlainTextEdit::event(e);
}

void XenomTextViewer::mouseDoubleClickEvent(QMouseEvent *e)
{
	IViewer *viewer = (IViewer *)XdvGetObjectByHandle(viewer_handle_);
	if (!viewer)
	{
		QPlainTextEdit::mouseDoubleClickEvent(e);
		return;
	}

	QTextCursor check_cursor = textCursor();
	check_cursor.select(QTextCursor::LineUnderCursor);
	if (check_cursor.selectedText().size() == 0)
	{
		QPlainTextEdit::mouseDoubleClickEvent(e);
		return;
	}

	QTextCursor pre_cursor = textCursor();
	pre_cursor.select(QTextCursor::LineUnderCursor);
	viewer->Update(xdv::status::id::XENOM_UPDATE_STATUS_PRE_EVENT, pre_cursor.selectedText().toStdString());

	QTextCursor post_cursor = textCursor();
	post_cursor.select(QTextCursor::WordUnderCursor);
	viewer->Update(xdv::status::id::XENOM_UPDATE_STATUS_DOUBLE_CLICK_POST_EVENT, post_cursor.selectedText().toStdString());
}

void XenomTextViewer::wheelEvent(QWheelEvent *e)
{
	IViewer *viewer = (IViewer *)XdvGetObjectByHandle(viewer_handle_);
	if (!viewer)
	{
		QPlainTextEdit::wheelEvent(e);
		return;
	}

	if (id_ == xdv::viewer::id::DEFAULT_TEXT_VIEWER)
	{
		QPlainTextEdit::wheelEvent(e);
		return;
	}

	if (e->delta() > 0) // up
	{
		viewer->Update(xdv::status::id::XENOM_UPDATE_STATUS_UP, "");
	}
	else // down
	{
		viewer->Update(xdv::status::id::XENOM_UPDATE_STATUS_DOWN, "");
	}
}

void XenomTextViewer::keyPressEvent(QKeyEvent *e)
{
	IViewer *viewer = (IViewer *)XdvGetObjectByHandle(viewer_handle_);
	if (!viewer)
	{
		QPlainTextEdit::keyPressEvent(e);
		return;
	}

	if (id_ == xdv::viewer::id::DEFAULT_TEXT_VIEWER)
	{
		QPlainTextEdit::keyPressEvent(e);
		return;
	}

	switch (e->key())
	{
	case Qt::Key_Backspace:
		viewer->Update(xdv::status::id::XENOM_UPDATE_STSTUS_BACKSPACE, "");
		break;

	case Qt::Key_Space:
	{
		QTextCursor pre_cursor = textCursor();
		pre_cursor.select(QTextCursor::LineUnderCursor);
		viewer->Update(xdv::status::id::XENOM_UPDATE_STATUS_PRE_EVENT, pre_cursor.selectedText().toStdString());

		QTextCursor post_cursor = textCursor();
		post_cursor.select(QTextCursor::WordUnderCursor);
		viewer->Update(xdv::status::id::XENOM_UPDATE_STSTUS_SPACE_POST_EVENT, post_cursor.selectedText().toStdString());

		break;
	}

	case Qt::Key_Up:
	{
		viewer->Update(xdv::status::id::XENOM_UPDATE_STATUS_UP, "");
		break;
	}

	case Qt::Key_Down:
	{
		viewer->Update(xdv::status::id::XENOM_UPDATE_STATUS_DOWN, "");
		break;
	}

	default:
		QPlainTextEdit::keyPressEvent(e);
		break;
	}
}

void XenomTextViewer::mousePressEvent(QMouseEvent *e)
{
	QPlainTextEdit::mousePressEvent(e);
}

void XenomTextViewer::mouseReleaseEvent(QMouseEvent *e)
{
	syntaxHighlight();
	QPlainTextEdit::mouseReleaseEvent(e);
}

void XenomTextViewer::contextMenuEvent(QContextMenuEvent * e)
{
	QMenu menu("menu", this);
	QList<QAction *> actions = this->actions();
	for (auto action : actions)
	{
		menu.addAction(action);
	}
	menu.addSeparator();

	for (int i = 0; i < context_menu_vector_.size(); ++i)
	{
		menu.addMenu(context_menu_vector_[i]);
	}
	menu.exec(mapToGlobal(e->pos()));
}

void XenomTextViewer::resizeEvent(QResizeEvent *e)
{
	QPlainTextEdit::resizeEvent(e);

	if (id_ == xdv::viewer::id::TEXT_VIEWER_DASM && navigationArea_)
	{
		QRect cr = contentsRect();
		navigationArea_->setGeometry(QRect(cr.left(), cr.top(), blockAreaWidth(), cr.height()));
	}
}

// -------------------------------------------------
//
void XenomTextViewer::updateText(QString str)
{
	this->clear();
	this->insertPlainText(str);
}

void XenomTextViewer::clearText()
{
	this->clear();
}

void XenomTextViewer::syntaxHighlight()
{
	QTextEdit::ExtraSelection selection;
	selection.format.setBackground(line_color_);
	selection.format.setProperty(QTextFormat::FullWidthSelection, true);
	selection.cursor = textCursor();
	selection.cursor.clearSelection();

	//
	QTextCursor cursor = textCursor();
	cursor.select(QTextCursor::WordUnderCursor);

	QTextEdit::ExtraSelection current_word;
	current_word.format.setBackground(line_color_);
	current_word.cursor = cursor;

	QList<QTextEdit::ExtraSelection> extra_selections;
	extra_selections.append(selection);
	extra_selections.append(current_word);
	setExtraSelections(extra_selections);

	//
	// highlighting same text 
	QString select = cursor.selectedText();
	if (strlen(select.toStdString().c_str()) > 1 && strcmp(select.toStdString().c_str(), "00") != 0)
	{
		char *is_ptr = strstr((char *)select.toStdString().c_str(), "0x");
		if (is_ptr)
		{
			is_ptr += 2;
			select = is_ptr;
		}

		highlighter_->setHighlightedString(select);
	}
}

// -------------------------------------------------
//
void XenomTextViewer::addCommand(char * plugin_command, char * menu, char * name, char * shortcut)
{
	QList<QAction *> actions = this->actions();
	for (auto action : actions)
	{
		if (strstr(action->text().toStdString().c_str(), name))
		{
			return;
		}
	}

	for (int i = 0; i < context_menu_vector_.size(); ++i)
	{
		QList<QAction *> actions = context_menu_vector_[i]->actions();
		for (auto action : actions)
		{
			if (strstr(action->text().toStdString().c_str(), name))
			{
				return;
			}
		}
	}

	//
	PluginAction * action = new PluginAction(plugin_command);
	if (plugin_command)
	{
		xnm * xenom = getXenom();
		xenom->addPlugin(action, menu);
	}

	if (shortcut)
	{
		char * end = nullptr;
		unsigned long sc = strtoul(shortcut, &end, 16);
		if (sc)
		{
			action->setShortcut(sc);
		}
	}

	action->setShortcutContext(Qt::ShortcutContext::WidgetShortcut);
	action->setText(name);
	QObject::connect(action, &QAction::triggered, this, &XenomTextViewer::commandAction);
	if (menu)
	{
		QMenu * ctx = nullptr;
		for (int i = 0; i < context_menu_vector_.size(); ++i)
		{
			if (strstr(context_menu_vector_[i]->title().toStdString().c_str(), menu))
			{
				ctx = context_menu_vector_[i];
				break;
			}
		}

		if (!ctx)
		{
			ctx = new QMenu(menu, this);
			context_menu_vector_.push_back(ctx);
		}
		ctx->addAction(action);
	}
	else
	{
		this->addAction(action);
	}
}

void XenomTextViewer::commandAction()
{
	QTextCursor cursor = textCursor();
	cursor.select(QTextCursor::LineUnderCursor);

	PluginAction * plugin = (PluginAction*)this->sender();
	plugin->commandAction(viewer_handle_, cursor.selectedText());
}

int XenomTextViewer::blockAreaWidth()
{
	int digits = 1;
	int max = qMax(1, blockCount());
	while (max >= 10) 
	{
		max /= 10;
		++digits;
	}

	int space = 7 + fontMetrics().width(QLatin1Char('9')) * digits;

	return space;
}

void XenomTextViewer::updateBlockAreaWidth(int)
{
	setViewportMargins(blockAreaWidth(), 0, 0, 0);
}

void XenomTextViewer::updateBlockArea(const QRect &rect, int dy)
{
	if (dy)
	{
		navigationArea_->scroll(0, dy);
	}
	else
	{
		navigationArea_->update(0, rect.y(), navigationArea_->width(), rect.height());
	}

	if (rect.contains(viewport()->rect()))
		updateBlockAreaWidth(0);
}

void XenomTextViewer::drawBlockPaintEvent(QPaintEvent *event)
{
	QTextCursor cursor = textCursor();
	cursor.select(QTextCursor::LineUnderCursor);
	unsigned long long current = XdvToUll((char *)cursor.selectedText().toStdString().c_str());

	std::map<unsigned long long, point> points;
	QTextBlock block = this->document()->firstBlock();
	int interval = 2;
	for (int i = 0; i < this->document()->blockCount(); ++i)
	{
		if (block.text().size())
		{
			point p;
			char * end = nullptr;
			const char * ptr = block.text().toStdString().c_str();
			unsigned long long current = XdvToUll((char *)ptr);
			if (current)
			{
				const char * dest = strstr(block.text().toStdString().c_str(), "0x");
				if (dest)
				{
					p.dest = XdvToUll((char *)dest);
				}
				else
				{
					p.dest = 0;
				}

				QPoint p1 = QPoint(blockBoundingGeometry(block).topLeft().x(), blockBoundingGeometry(block).topLeft().y());
				QPoint p2 = QPoint(blockBoundingGeometry(block).topRight().x(), blockBoundingGeometry(block).topRight().y());

				p1.setX(p1.x() + blockAreaWidth() - interval);
				p1.setY(blockBoundingGeometry(block).center().y());

				p2.setY(blockBoundingGeometry(block).center().y());
				p.current_line = QLine(p1, p2);

				points.insert(std::pair<unsigned long long, point>(current, p));
				interval += 2;

				if (interval >= blockAreaWidth())
				{
					interval = 2;
				}
			}
		}

		block = block.next();
	}

	QVector<QLine> lines;
	for (auto it : points) // draw line
	{
		unsigned char dump[16] = { 0, };
		if (XdvReadMemory(XdvGetParserHandle(), it.first, dump, 16) == 0)
		{
			continue;
		}

		bool jxx = false;
		if (!XdvIsJumpCode(XdvGetArchitectureHandle(), it.first, dump, &jxx))
		{
			continue;
		}

		unsigned long long dest = it.second.dest;
		auto f = points.find(dest);
		if (f != points.end())
		{
			QLine src = it.second.current_line;
			QLine dest = f->second.current_line;
			QPoint dp1(src.p1().x(), dest.p1().y());
			dest.setP1(dp1);

			lines.clear();
			lines.push_back(src);
			lines.push_back(dest);
			lines.push_back(QLine(src.p1(), dest.p1()));
			lines.push_back(QLine());

			QPen pen;
			pen.setStyle(Qt::DashLine);
			pen.setColor(Qt::gray);

			if (current == it.first)
			{
				pen.setColor(Qt::blue);
			}

			QPainter painter(navigationArea_);
			painter.setPen(pen);
			painter.drawLines(lines);
		}
	}
}

SyntaxHighlighter * XenomTextViewer::Highlighter()
{
	return highlighter_;
}