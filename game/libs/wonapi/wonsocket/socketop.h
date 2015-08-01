#ifndef __WON_SOCKETOP_H__
#define __WON_SOCKETOP_H__
#include "wonshared.h"

#include "woncommon/asyncop.h"
#include "woncommon/readbuffer.h"
#include "woncommon/writebuffer.h"
#include "asyncsocket.h"

namespace WONAPI
{

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
enum SocketEventType
{
	SocketEvent_Read = 0,
	SocketEvent_Write = 1,
	SocketEvent_Except = 2
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class SocketOp : public AsyncOp
{
public:
	bool mInSocketThread; // used only internally by the various SocketThreads 
	int mArrayPos[3];

	static bool mRunAsyncImmediatelyDef;

protected:
	AsyncSocketPtr mSocket;
	bool mSocketEvent[3];

	virtual void RunHook();
	virtual void CleanupHook();

protected:
	friend class SocketThread;
	virtual WONStatus Start() { return Continue(); }
	virtual WONStatus Continue() = 0;

public:
	SocketOp(AsyncSocket *theSocket) : mSocket(theSocket), mInSocketThread(false)
	{
		SetRunAsyncImmediately(mRunAsyncImmediatelyDef);
		for(int i=0; i<3; i++)
		{
			mSocketEvent[i] = false;
			mArrayPos[i] = -1;
		}
	}

	void SetSocket(AsyncSocket *theSocket) { mSocket = theSocket; }
	AsyncSocket* GetSocket() { return mSocket; }

	virtual SocketOp* Duplicate() { return NULL; }

	bool NeedSocketEvent(SocketEventType theType) { return mSocketEvent[theType]; }
};

typedef SmartPtr<SocketOp> SocketOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; // namespace WONAPI

#endif
