#include "rmtexts.h"

void ExceptionMonitor(void * ctx)
{
	DebugContextPtr dcp = (DebugContextPtr)XdvDebugSharedMemory();
	if (!dcp)
	{
		return;
	}

	RemoteApi * ra = (RemoteApi *)ctx;
	if (!ra)
	{
		return;
	}

	do
	{
		XdvWaitForExceptionEvent(); // flush excption event
		ra->RestoreBreakPoint(dcp->context.rip);

		if (ra->GetStepOverPtr() == dcp->context.rip)
		{
			ra->DeleteBreakPoint(dcp->context.rip);
			ra->SetStepOverPtr(0);
		}

		XdvSetDebugEvent();

		//
		// event를 추가하여, 여기서 이벤트를 대기.


	} while (1);
}