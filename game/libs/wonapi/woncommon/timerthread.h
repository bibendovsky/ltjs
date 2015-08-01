#ifndef __WON_TIMERTHREAD_H__
#define __WON_TIMERTHREAD_H__
#include "wonshared.h"

#include "bimap.h"
#include "thread.h"
#include "asyncop.h"
#include "criticalsection.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class TimerThread : public Thread
{
private:
	CriticalSection mDataCrit;

	typedef AsyncOp::TimerMap TimerMap;

	TimerMap mTimerMap;
	TimerMap mWrapAroundMap;
	DWORD mLastTick;

private:
	virtual void ThreadFunc();	

public:
	TimerThread();
	~TimerThread();

	void AddTimerOp(AsyncOp *theOp, DWORD theTimeout);
	void RemoveTimerOp(AsyncOp *theOp);
	void Pump();
	DWORD GetWaitTime();

	void PurgeOps();
};


}; // namespace WONAPI

#endif
