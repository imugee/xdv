#include "xdv_sdk.h"
#include "qxnm.h"
#include "XenomWorker.h"
#include "XenomDefaultViewer.h"

EXTS_FUNC(addtxtv)		// argv = handle
						// argv = title
						// argv = type
						// return = IWorker ptr
{
	char * handle_str = XdvValue(argv, argc, "handle", nullptr);
	char * title = XdvValue(argv, argc, "title", nullptr);
	char * type = XdvValue(argv, argc, "type", nullptr);

	char *end = nullptr;
	xdv_handle handle = strtoull(handle_str, &end, 16);
	xnm *xenom = getXenom();
	XenomDockWidget * dock = new XenomDockWidget(xenom);
	XenomPlainTextEdit *text_viewer = nullptr;
	xdv::viewer::id id = xdv::viewer::id::TEXT_VIEWER_A;
	if (type && strstr(type, "txtb"))
	{
		id = xdv::viewer::id::TEXT_VIEWER_B;
	}
	else if (type && strstr(type, "txtc"))
	{
		id = xdv::viewer::id::TEXT_VIEWER_C;
	}

	text_viewer = new XenomTextViewer(handle, id);

	dock->addVbox(text_viewer);
	dock->setTextViewer(text_viewer);
	dock->setWindowTitle(title);
	xenom->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dock);
	xenom->addViewer(handle, dock);

	return ptrvar(new XenomWorker(handle));
}

EXTS_FUNC(addcmdv)		// argv = handle
						// argv = title
						// argv = type
						// return = IWorker ptr
{
	char * handle_str = XdvValue(argv, argc, "handle", nullptr);
	char * title = XdvValue(argv, argc, "title", nullptr);
	char * type = XdvValue(argv, argc, "type", nullptr);

	char *end = nullptr;
	xdv_handle handle = strtoull(handle_str, &end, 16);
	xnm *xenom = getXenom();
	XenomDockWidget * dock = new XenomDockWidget(xenom);
	XenomPlainTextEdit *text_viewer = nullptr;
	xdv::viewer::id id = xdv::viewer::id::TEXT_VIEWER_A;
	if (type && strstr(type, "txtb"))
	{
		id = xdv::viewer::id::TEXT_VIEWER_B;
	}
	else if (type && strstr(type, "txtc"))
	{
		id = xdv::viewer::id::TEXT_VIEWER_C;
	}

	text_viewer = new XenomTextViewer(handle, id);
	dock->addVbox(new XenomCommandWidget(handle));
	dock->addVbox(text_viewer);
	dock->setTextViewer(text_viewer);
	dock->setWindowTitle(title);
	xenom->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dock);
	xenom->addViewer(handle, dock);

	return ptrvar(new XenomWorker(handle));
}

EXTS_FUNC(addv)		// argv = obj_string
					// argv = title 
					// argv = cb command
					// argv = type
					// return = xdv_handle
{
	char * name = XdvValue(argv, argc, "name", nullptr);
	char * title = XdvValue(argv, argc, "title", nullptr);
	if (!name || !title)
	{
		return nullvar();
	}

	xdv::viewer::id id = xdv::viewer::id::TEXT_VIEWER_A;
	char *type = XdvValue(argv, argc, "type", nullptr);
	if (type && strstr(type, "txtb"))
	{
		id = xdv::viewer::id::TEXT_VIEWER_B;
	}
	else if (type && strstr(type, "txtc"))
	{
		id = xdv::viewer::id::TEXT_VIEWER_C;
	}
	else if (type && strstr(type, "cmda"))
	{
		id = xdv::viewer::id::COMMAND_VIEWER_A;
	}

	char * command = XdvValue(argv, argc, "callback", nullptr);
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

EXTS_FUNC(chkable)		// argv[0] = handle
{
	char * handle_str = XdvValue(argv, argc, "handle", nullptr);
	char *end = nullptr;
	xdv_handle handle = strtoull(handle_str, &end, 16);
	XenomDefaultViewer * viewer = (XenomDefaultViewer*)XdvGetObjectByHandle(handle);
	viewer->SetCheckable(true);

	return nullvar();
}

EXTS_FUNC(ctxmenu)		// argv[0] = handle
						// argv[1] = name
						// argv[2] = key
{
	char * handle_str = XdvValue(argv, argc, "handle", nullptr);
	char * menu = XdvValue(argv, argc, "menu", nullptr);
	char * name = XdvValue(argv, argc, "name", nullptr);
	char * key = XdvValue(argv, argc, "key", nullptr);
	char * menu_icon = XdvValue(argv, argc, "mco", nullptr);
	char * icon = XdvValue(argv, argc, "nco", nullptr);

	if (!handle_str || !name)
	{
		return nullvar();
	}

	char *end = nullptr;
	xdv_handle handle = strtoull(handle_str, &end, 16);
	xnm *xenom = getXenom();

	XenomDockWidget * dock = xenom->Viewer(handle);
	if (!dock)
	{
		return nullvar();
	}

	XenomTextViewer * text = (XenomTextViewer *)dock->TextViewer();
	if (text)
	{
		text->addShortcutAction(menu, menu_icon, name, key, icon);
	}

	return nullvar();
}

EXTS_FUNC(addtabv)		// argv[0] = va
						// argv[1] = vb
{
	char * vaarg = XdvValue(argv, argc, "va", nullptr);
	char *end = nullptr;
	xdv_handle va_handle = strtoull(vaarg, &end, 16);
	if (XdvGetObjectByHandle(va_handle)->ObjectType() != xdv::object::id::XENOM_VIEWER_OBJECT)
	{
		return nullvar();
	}

	char * vbarg = XdvValue(argv, argc, "vb", nullptr);
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

EXTS_FUNC(raise)		// argv[0] = handle
{
	char * arg = XdvValue(argv, argc, "handle", nullptr);
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

EXTS_FUNC(addsplitv)		// argv[0] = va
{
	char * vaarg = XdvValue(argv, argc, "va", nullptr);
	char *end = nullptr;
	xdv_handle va_handle = strtoull(vaarg, &end, 16);
	if (XdvGetObjectByHandle(va_handle)->ObjectType() != xdv::object::id::XENOM_VIEWER_OBJECT)
	{
		return nullvar();
	}

	char * vbarg = XdvValue(argv, argc, "vb", nullptr);
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

	char * area = XdvValue(argv, argc, "area", nullptr);
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

EXTS_FUNC(linecolor)		// argv[0] = viewer handle
							// argv[1] = line color
{
	char * handle_arg = XdvValue(argv, argc, "handle", nullptr);
	char * line_color = XdvValue(argv, argc, "color", nullptr);
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