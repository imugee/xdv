#ifndef __DEFINE_XENOM_TEXT_VIEWER__
#define __DEFINE_XENOM_TEXT_VIEWER__

#include "xdv_sdk.h"

#include <qaction.h>
#include <qmenu.h>
#include <qtooltip.h>
#include <qpoint.h>

#include "XenomPlainTextEdit.h"
#include "SyntaxHighlighter.h"

class XenomTextViewer : public XenomPlainTextEdit
{
private:
	std::string update_command_;
	xdv_handle viewer_handle_;
	QAction action_[10];

	SyntaxHighlighter *highlighter_;

	QColor line_color_;
	xdv::viewer::id id_;

	std::vector<QMenu *> context_menu_vector_;

public:
	explicit XenomTextViewer(xdv_handle handle, QWidget *parent = 0);
	explicit XenomTextViewer(xdv_handle handle, xdv::viewer::id id, QWidget *parent = 0);
	~XenomTextViewer();

	void syntaxHighlight();
	virtual void updateText(QString string);
	virtual void clearText();

	void addShortcutAction(char * menu, char * menu_icon, char * name, char * shortcut, char * icon);
	void shortcutAction();

protected:
	virtual bool event(QEvent *e) override;
	virtual void mouseDoubleClickEvent(QMouseEvent *e) override;
	virtual void wheelEvent(QWheelEvent *e) override;
	virtual void keyPressEvent(QKeyEvent *e) override;
	virtual void mousePressEvent(QMouseEvent *e) override;
	virtual void mouseReleaseEvent(QMouseEvent *e) override;
	void contextMenuEvent(QContextMenuEvent * e);
};

#endif