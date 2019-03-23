#include "XenomWorker.h"
#include "qxnm.h"

XenomWorker::XenomWorker(xdv_handle link_handle)
	: link_handle_(link_handle)
{
}

xdv::object::id XenomWorker::ObjectType()
{
	return xdv::object::id::XENOM_WORKER_OBJECT;
}

std::string XenomWorker::ObjectString()
{
	return "XenomWorker";
}

void XenomWorker::SetModuleName(std::string module)
{
}

std::string XenomWorker::ModuleName()
{
	return "";
}

void XenomWorker::Run(xdv_handle viewer_handle, ThreadRunCallbackType callback, void *callback_context)
{
	worker_ = new XenomThreadWorker(viewer_handle, this, callback, callback_context);
	QObject::connect(worker_, SIGNAL(finished()), worker_, SLOT(deleteLater()), Qt::DirectConnection);
	worker_->start();
}

void PrintCallback(IWorker *worker, void *ctx)
{
	worker->Update();
}

void PrintAndClearCallback(IWorker *worker, void *ctx)
{
	worker->UpdateAndClear();
}

void ClearCallback(IWorker *worker, void *ctx)
{
	worker->ClearString();
}

void XenomWorker::Print(xdv_handle viewer_handle, std::string str)
{
	xnm *xenom = getXenom();
	XenomDockWidget *dock = xenom->Viewer(viewer_handle);
	if (!dock)
	{
		return;
	}

	XenomPlainTextEdit *pe = (XenomPlainTextEdit *)dock->TextViewer();
	if (!pe)
	{
		return;
	}

	//pe->moveCursor(QTextCursor::MoveOperation::End); // 버그.. 동기화 실패로 크래시가 발생한다;
	pe->insertPlainText(str.c_str());
}

void XenomWorker::Print(xdv_handle viewer_handle, std::string str, bool wait)
{
	text_string_ += str;
	worker_ = new XenomThreadWorker(viewer_handle, this, PrintCallback, nullptr);
	QObject::connect(worker_, SIGNAL(finished()), worker_, SLOT(deleteLater()), Qt::DirectConnection);
	worker_->start();
	if (wait)
	{
		worker_->wait();
	}
}

void XenomWorker::PrintAndClear(xdv_handle viewer_handle, std::string str, bool wait)
{
	text_string_ += str;
	worker_ = new XenomThreadWorker(viewer_handle, this, PrintAndClearCallback, nullptr);
	QObject::connect(worker_, SIGNAL(finished()), worker_, SLOT(deleteLater()), Qt::DirectConnection);
	worker_->start();
	if (wait)
	{
		worker_->wait();
	}
}

void XenomWorker::Clear(xdv_handle viewer_handle)
{
	worker_ = new XenomThreadWorker(viewer_handle, this, ClearCallback, nullptr);
	QObject::connect(worker_, SIGNAL(finished()), worker_, SLOT(deleteLater()), Qt::DirectConnection);
	worker_->start();
}

void XenomWorker::InsertString(std::string str)
{
	text_string_ += str;
}

std::string XenomWorker::String()
{
	return text_string_;
}

void XenomWorker::Update()
{
	worker_->update(text_string_);
}

void XenomWorker::UpdateAndClear()
{
	worker_->update(text_string_);
	text_string_.clear();
}

void XenomWorker::ClearString()
{
	text_string_.clear();
}

void XenomWorker::Lock()
{
	mutex_.lock();
}

void XenomWorker::UnLock()
{
	mutex_.unlock();
}

xdv_handle XenomWorker::LinkViewerHandle()
{
	return link_handle_;
}
