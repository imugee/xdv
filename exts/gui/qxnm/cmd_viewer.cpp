#include "xdv_sdk.h"
#include "qxnm.h"
#include "XenomWorker.h"
#include "XenomDefaultViewer.h"

EXTS_FUNC(add_txtv)		// argv = handle
						// argv = title
						// argv = type
						// return = IWorker ptr
{
	char * handle_str = argof("handle");
	char * title = argof("title");
	char * type = argof("type");

	char *end = nullptr;
	xdv_handle handle = strtoull(handle_str, &end, 16);
	xnm *xenom = getXenom();
	XenomDockWidget * dock = new XenomDockWidget(xenom);
	XenomPlainTextEdit *text_viewer = nullptr;
	xdv::viewer::id id = xdv::viewer::id::DEFAULT_TEXT_VIEWER;
	if (type && strstr(type, "event"))
	{
		id = xdv::viewer::id::EVENT_BASE_TEXT_VIEWER;
	}
	else if (type && strstr(type, "dasm"))
	{
		id = xdv::viewer::id::TEXT_VIEWER_DASM;
	}

	text_viewer = new XenomTextViewer(handle, id);

	dock->addVbox(text_viewer);
	dock->setTextViewer(text_viewer);
	dock->setWindowTitle(title);
	xenom->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dock);
	xenom->addViewer(handle, dock);

	return ptrvar(new XenomWorker(handle));
}

EXTS_FUNC(add_cmdv)		// argv = handle
						// argv = title
						// argv = type
						// return = IWorker ptr
{
	char * handle_str = argof("handle");
	char * title = argof("title");
	char *end = nullptr;
	xdv_handle handle = strtoull(handle_str, &end, 16);
	xnm *xenom = getXenom();
	XenomDockWidget * dock = new XenomDockWidget(xenom);
	XenomPlainTextEdit *text_viewer = nullptr;

	xdv::viewer::id id = xdv::viewer::id::DEFAULT_TEXT_VIEWER;
	text_viewer = new XenomTextViewer(handle, id);
	dock->addVbox(new XenomCommandWidget(handle));
	dock->addVbox(text_viewer);
	dock->setTextViewer(text_viewer);
	dock->setWindowTitle(title);
	xenom->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dock);
	xenom->addViewer(handle, dock);

	return ptrvar(new XenomWorker(handle));
}

EXTS_FUNC(add_viewer)		// argv = obj_string
							// argv = title 
							// argv = cb command
							// argv = type
							// return = xdv_handle
{
	char * name = argof("name");
	char * title = argof("title");
	if (!name || !title)
	{
		return nullvar();
	}

	xdv::viewer::id id = xdv::viewer::id::DEFAULT_TEXT_VIEWER;
	char *type = argof("type");
	if (type && strstr(type, "event"))
	{
		id = xdv::viewer::id::EVENT_BASE_TEXT_VIEWER;
	}
	else if (type && strstr(type, "cmd"))
	{
		id = xdv::viewer::id::COMMAND_VIEWER;
	}
	else if (type && strstr(type, "dasm"))
	{
		id = xdv::viewer::id::TEXT_VIEWER_DASM;
	}

	char * command = argof("callback");
	XenomDefaultViewer *viewer = __add_object(XenomDefaultViewer);
	xdv_handle handle = XdvGetHandleByObject(viewer);
	if (viewer)
	{
		viewer->SetCurrentHandle(handle);
		viewer->SetObjectString(name);
		viewer->SetTitleString(title);
		viewer->SetCommandString(command);
		viewer->SetViewerId(id);
	}
	return handlevar(handle);
}

EXTS_FUNC(set_checkable)		// argv[0] = handle
{
	char * handle_str = argof("handle");
	char *end = nullptr;
	xdv_handle handle = strtoull(handle_str, &end, 16);
	XenomDefaultViewer * viewer = (XenomDefaultViewer*)XdvGetObjectByHandle(handle);
	viewer->SetCheckable(true);

	return nullvar();
}

EXTS_FUNC(add_command)		// argv[0] = handle
							// argv[1] = name
							// argv[2] = key
{
	xdv_handle handle = tohandlearg("handle");
	if (XdvGetObjectByHandle(handle)->ObjectType() != xdv::object::id::XENOM_VIEWER_OBJECT)
	{
		return nullvar();
	}

	char * menu = argof("menu");
	char * name = argof("name");
	char * key = argof("key");
	char * plugin = argof("plugin");
	if (!name)
	{
		return nullvar();
	}

	xnm *xenom = getXenom();
	XenomDockWidget * dock = xenom->Viewer(handle);
	if (!dock)
	{
		return nullvar();
	}

	XenomTextViewer * text = (XenomTextViewer *)dock->TextViewer();
	if (text)
	{
		text->addCommand(plugin, menu, name, key);
	}

	return nullvar();
}

EXTS_FUNC(add_tab)		// argv[0] = va
						// argv[1] = vb
{
	char * vaarg = argof("va");
	char *end = nullptr;
	xdv_handle va_handle = strtoull(vaarg, &end, 16);
	if (XdvGetObjectByHandle(va_handle)->ObjectType() != xdv::object::id::XENOM_VIEWER_OBJECT)
	{
		return nullvar();
	}

	char * vbarg = argof("vb");
	xdv_handle vb_handle = strtoull(vbarg, &end, 16);
	if (XdvGetObjectByHandle(vb_handle)->ObjectType() != xdv::object::id::XENOM_VIEWER_OBJECT)
	{
		return nullvar();
	}

	xnm *xenom = getXenom();
	XenomDockWidget *va_dock = xenom->Viewer(va_handle);
	if (!va_dock)
	{
		return nullvar();
	}

	XenomDockWidget *vb_dock = xenom->Viewer(vb_handle);
	if (!vb_dock)
	{
		return nullvar();
	}

	xenom->tabifyDockWidget(va_dock, vb_dock);
	return nullvar();
}

EXTS_FUNC(raise_viewer)		// argv[0] = handle
{
	char * arg = argof("handle");
	char *end = nullptr;
	xdv_handle handle = strtoull(arg, &end, 16);
	if (XdvGetObjectByHandle(handle)->ObjectType() != xdv::object::id::XENOM_VIEWER_OBJECT)
	{
		return nullvar();
	}

	xnm *xenom = getXenom();
	XenomDockWidget *dock = xenom->Viewer(handle);
	dock->setVisible(true);
	dock->setFocus();
	dock->raise();

	return nullvar();
}

EXTS_FUNC(add_split)		// argv[0] = va
{
	char * vaarg = argof("va");
	char *end = nullptr;
	xdv_handle va_handle = strtoull(vaarg, &end, 16);
	if (XdvGetObjectByHandle(va_handle)->ObjectType() != xdv::object::id::XENOM_VIEWER_OBJECT)
	{
		return nullvar();
	}

	char * vbarg = argof("vb");
	xdv_handle vb_handle = strtoull(vbarg, &end, 16);
	if (XdvGetObjectByHandle(vb_handle)->ObjectType() != xdv::object::id::XENOM_VIEWER_OBJECT)
	{
		return nullvar();
	}

	xnm *xenom = getXenom();
	XenomDockWidget *va_dock = xenom->Viewer(va_handle);
	if (!va_dock)
	{
		return nullvar();
	}

	XenomDockWidget *vb_dock = xenom->Viewer(vb_handle);
	if (!vb_dock)
	{
		return nullvar();
	}

	char * area = argof("area");
	if (strstr(area, "left"))
	{
		xenom->splitDockWidget(vb_dock, va_dock, Qt::Orientation::Horizontal);
	}
	else if (strstr(area, "right"))
	{
		xenom->splitDockWidget(va_dock, vb_dock, Qt::Orientation::Horizontal);
	}
	else if (strstr(area, "top"))
	{
		xenom->splitDockWidget(vb_dock, va_dock, Qt::Orientation::Vertical);
	}
	else if (strstr(area, "bottom"))
	{
		xenom->splitDockWidget(va_dock, vb_dock, Qt::Orientation::Vertical);
	}

	return nullvar();
}

EXTS_FUNC(change_color)		// argv[0] = viewer handle
							// argv[1] = line color
{
	char * handle_arg = argof("handle");
	char * line_color = argof("color");
	char *end = nullptr;
	xdv_handle handle = strtoull(handle_arg, &end, 16);
	if (XdvGetObjectByHandle(handle)->ObjectType() != xdv::object::id::XENOM_VIEWER_OBJECT)
	{
		return nullvar();
	}

	xnm *xenom = getXenom();
	XenomDockWidget *dock = xenom->Viewer(handle);
	if (!dock)
	{
		return nullvar();
	}

	QPlainTextEdit * text = dock->TextViewer();
	QTextCursor cursor = text->textCursor();
	cursor.select(QTextCursor::LineUnderCursor);

	QTextEdit::ExtraSelection current_line;
	if (line_color)
	{
		current_line.format.setBackground(QColor(line_color));
	}

	current_line.format.setProperty(QTextFormat::FullWidthSelection, true);
	current_line.cursor = cursor;

	QList<QTextEdit::ExtraSelection> extra_selections;
	extra_selections.append(current_line);
	text->setExtraSelections(extra_selections);

	return nullvar();
}

EXTS_FUNC(express_color)		// argv viewer handle
								// argv color
								// argv expression
{
	char * handle_arg = argof("handle");
	char *end = nullptr;
	xdv_handle handle = strtoull(handle_arg, &end, 16);
	if (XdvGetObjectByHandle(handle)->ObjectType() != xdv::object::id::XENOM_VIEWER_OBJECT)
	{
		return nullvar();
	}

	xnm *xenom = getXenom();
	XenomDockWidget *dock = xenom->Viewer(handle);
	if (!dock)
	{
		return nullvar();
	}

	XenomTextViewer * text = (XenomTextViewer *)dock->TextViewer();
	if (!text)
	{
		return nullvar();
	}

	char * color = argof("color");
	char * expression = argof("expression");
	if (color && expression)
	{
		char * bold = argof("bold");
		if (bold)
		{
			text->Highlighter()->addHighlightBlock(expression, color, true);
		}
		else
		{
			text->Highlighter()->addHighlightBlock(expression, color, false);
		}
	}

	return nullvar();
}