#ifndef __SOCKET_H__
#define __SOCKET_H__

#ifndef  _WINSOCKAPI_
#include <winsock.h>
#endif //_WINSOCKAPI_

#if _MSC_VER >= 1900
#undef EWOULDBLOCK
#undef ECONNRESET
#define EWOULDBLOCK (WSAEWOULDBLOCK)
#define ECONNRESET (WSAECONNRESET)
#else
const int EWOULDBLOCK = WSAEWOULDBLOCK;
const int ECONNRESET = WSAECONNRESET;
#endif

typedef int socklen_t;

#endif __SOCKET_H__