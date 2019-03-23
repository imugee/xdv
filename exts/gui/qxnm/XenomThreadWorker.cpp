#include "qxnm.h"
#include "XenomThreadWorker.h"
#include "XenomDockWidget.h"
#include "XenomPlainTextEdit.h"

XenomThreadWorker::XenomThreadWorker(xdv_handle viewer_handle, IWorker *worker, IWorker::ThreadRunCallbackType callback, void *callback_context)
	: worker_(worker), callback_(callback), callback_context_(callback_context)
{
	xnm *xenom = getXenom();
	XenomDockWidget *dock = xenom->Viewer(viewer_handle);
	if (!dock)
	{
		return;
	}

	XenomPlainTextEdit *pe = (XenomPlainTextEdit *)dock->TextViewer();
	QObject::connect(this, &XenomThreadWorker::updateText, pe, &XenomPlainTextEdit::updateText);
	QObject::connect(this, &XenomThreadWorker::clearText, pe, &XenomPlainTextEdit::clearText);
	QObject::connect(this, &XenomThreadWorker::raiseDock, dock, &XenomDockWidget::raiseDock);
}

XenomThreadWorker::~XenomThreadWorker()
{
}

void XenomThreadWorker::update(std::string str)
{
	emit(updateText(str.c_str()));
	emit(raiseDock());
}

void XenomThreadWorker::clear()
{
	emit(clearText());
}

void XenomThreadWorker::run()
{
	worker_->Lock();

	if (callback_)
	{
		callback_(worker_, callback_context_);
	}

	worker_->UnLock();
}