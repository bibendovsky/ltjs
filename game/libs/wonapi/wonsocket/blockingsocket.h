#ifndef __WON_BLOCKINGSOCKET_H__
#define __WON_BLOCKINGSOCKET_H__
#include "wonshared.h"

#include "queuesocket.h"
#include "socketop.h"
#include "connectop.h"
#include "acceptop.h"
#include "sendbytesop.h"
#include "recvbytesop.h"
#include "sendbytestoop.h"
#include "recvbytesfromop.h"
#include "sendmsgop.h"
#include "recvmsgop.h"
#include "recvcrmsgop.h"

namespace WONAPI
{
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class BlockingSocket : public QueueSocket
{
public:
	BlockingSocket(SocketType theType = TCP);

	WONStatus Connect(const IPAddr &theAddr, DWORD theTimeout = OP_TIMEOUT_INFINITE);
	WONStatus Accept(AsyncSocketPtr &theSocket, DWORD theTimeout = OP_TIMEOUT_INFINITE);

	WONStatus SendBytes(const ByteBuffer *theBytes, DWORD theTimeout = OP_TIMEOUT_INFINITE);
	WONStatus RecvBytes(ByteBufferPtr &theBytes, DWORD theNumBytes, DWORD theTimeout = OP_TIMEOUT_INFINITE);

	WONStatus SendBytesTo(const ByteBuffer *theBytes, const IPAddr &theAddr, DWORD theTimeout = OP_TIMEOUT_INFINITE);
	WONStatus RecvBytesFrom(ByteBufferPtr &theBytes, unsigned short theMaxBytes, IPAddr &theAddr, DWORD theTimeout = OP_TIMEOUT_INFINITE);

	WONStatus SendMsg(const ByteBuffer *theMsg, DWORD theTimeout = OP_TIMEOUT_INFINITE);
	WONStatus RecvMsg(ByteBufferPtr &theMsg, DWORD theTimeout = OP_TIMEOUT_INFINITE);

	WONStatus RecvCRMsg(ByteBufferPtr &theMsg, DWORD theTimeout = OP_TIMEOUT_INFINITE);

	virtual AsyncSocket* Duplicate() { return new BlockingSocket(mType); }
};

typedef SmartPtr<BlockingSocket> BlockingSocketPtr;

}; // namespace WONAPI

#endif
