#ifndef __WON_PLATFORM_H__
#define __WON_PLATFORM_H__
#include "wonshared.h"


#if defined(WIN32)
#include "platform_windows.h"
#elif defined(WINCE)
#include "platform_wince.h"
#elif defined(_LINUX)
#include "platform_linux.h"
#elif defined(macintosh) && (macintosh == 1)
#include "platform_mac.h"
#endif

#endif
