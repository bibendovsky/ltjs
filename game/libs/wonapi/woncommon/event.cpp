#ifndef __WON_SINGLETHREADED__

#if defined(WIN32)
#include "event_windows.cpp"
#elif defined(_LINUX)
#include "event_linux.cpp"
#endif

#endif // __WON_SINGLETHREADED__

