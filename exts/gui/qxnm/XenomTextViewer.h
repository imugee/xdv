#ifndef __DEFINE_XENOM_TEXT_VIEWER__
#define __DEFINE_XENOM_TEXT_VIEWER__

#include "xdv_sdk.h"

#include <qaction.h>
#include <qmenu.h>
#include <qtooltip.h>
#include <qpoint.h>

#include "XenomPlainTextEdit.h"
#include "SyntaxHighlighter.h"

class NavigationLineArea;
class XenomTextViewer : public XenomPlainTextEdit
{
private:
	typedef struct _tag_point
	{
		unsigned long long dest;
		QLine current_line;
	}point;

	std::string update_command_;
	xdv_handle viewer_handle_;
	QAction action_[10];

	SyntaxHighlighter * highlighter_;

	QColor line_color_;
	xdv::viewer::id id_;

	std::vector<QMenu *> context_menu_vector_;
	QWidget * navigationArea_;

public:
	explicit XenomTextViewer(xdv_handle handle, xdv::viewer::id id, QWidget *parent = 0);
	~XenomTextViewer();

	void syntaxHighlight();
	virtual void updateText(QString string);
	virtual void clearText();

	void addCommand(char * plugin, char * menu, char * name, char * shortcut);
	void commandAction();

	SyntaxHighlighter * Highlighter();

	int blockAreaWidth();
	void drawBlockPaintEvent(QPaintEvent *event);
	void updateBlockAreaWidth(int);
	void updateBlockArea(const QRect &rect, int dy);

protected:
	virtual bool event(QEvent *e) override;
	void resizeEvent(QResizeEvent *event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent *e) override;
	virtual void wheelEvent(QWheelEvent *e) override;
	virtual void keyPressEvent(QKeyEvent *e) override;
	virtual void mousePressEvent(QMouseEvent *e) override;
	virtual void mouseReleaseEvent(QMouseEvent *e) override;
	void contextMenuEvent(QContextMenuEvent * e);
};

class NavigationLineArea : public QWidget
{
public:
	NavigationLineArea(XenomTextViewer *editor) 
		: QWidget(editor)
	{
		editor_ = editor;
	}

	QSize sizeHint() const override
	{
		return QSize(editor_->blockAreaWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent *event) override
	{
		editor_->drawBlockPaintEvent(event);
	}

private:
	XenomTextViewer *editor_;
};

#endif