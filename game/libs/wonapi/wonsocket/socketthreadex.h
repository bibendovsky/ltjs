#ifndef __WON_SOCKETTHREADEX_H__
#define __WON_SOCKETTHREADEX_H__
#include "wonshared.h"

#if defined(WIN32)
#include "socketthreadex_windows.h"
#elif defined(_LINUX)
#include "socketthreadex_linux.h"
#else

#include "socketthreadsimple.h"
namespace WONAPI
{

typedef SocketThreadSimple SocketThreadEx;

}; // namespace WONAPI


#endif
#endif
