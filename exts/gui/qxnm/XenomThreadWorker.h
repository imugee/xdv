#ifndef __DEFINE_XENOM_THREAD_WORKER__
#define __DEFINE_XENOM_THREAD_WORKER__

#include <qthread>
#include <qmutex.h>

#include <xdv_sdk.h>

class XenomThreadWorker :public QThread
{
	Q_OBJECT

private:
	IWorker *worker_;
	IWorker::ThreadRunCallbackType callback_;
	void *callback_context_;

public:
	XenomThreadWorker(xdv_handle viewer_handle, IWorker *worker, IWorker::ThreadRunCallbackType callback, void *callback_context);
	~XenomThreadWorker();

	void update(std::string str);
	void clear();

signals:
	void updateText(QString string);
	void raiseDock();
	void clearText();

private:
	void run();
};

#endif