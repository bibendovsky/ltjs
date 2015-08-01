/****************************************************************************
 *
 *  $Id: rngcresult.h,v 1.6 2000/07/07 19:13:04 ingalls Exp $
 *
 *  Copyright (C) 1995-1999 RealNetworks, Inc. All rights reserved.
 *
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary
 *  information of Progressive Networks, Inc, and is licensed
 *  subject to restrictions on use and distribution.
 *
 *  This file contains the RNGC_*  result codes
 */


#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

#ifndef _WIN32
    #define FACILITY_ITF 4
    #define MAKE_HRESULT(sev,fac,code)						\
    ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) |   \
    ((unsigned long)(code))) )
    #define SUCCEEDED(Status) (((unsigned long)(Status)>>31) == 0)
    #define FAILED(Status) (((unsigned long)(Status)>>31) != 0)
#else
    #ifndef _HRESULT_DEFINED
	typedef LONG32 HRESULT;
    #endif	/* _HRESULT_DEFINED */
    #include <winerror.h>
#endif /* _WIN32 */

#define MAKE_RNGC_RESULT(sev,fac,code) MAKE_HRESULT(sev, FACILITY_ITF,	    \
    ((fac << 6) | (code)))

#define RNGC_CORE 48 /* These are the start of the Games Console Error Codes */

#define RNGC_NOCONSOLE			MAKE_RNGC_RESULT(1,RNGC_CORE,0x0001)
#define RNGC_INVALID_IID		MAKE_RNGC_RESULT(1,RNGC_CORE,0x0002)
#define RNGC_INVALID_LAUNCH		MAKE_RNGC_RESULT(1,RNGC_CORE,0x0003)
#define RNGC_TOURNEY_OVER		MAKE_RNGC_RESULT(1,RNGC_CORE,0x0004)
#define RNGC_SERVER_UNREACHABLE		MAKE_RNGC_RESULT(1,RNGC_CORE,0x0005)
#define RNGC_RESULTS_INVALID		MAKE_RNGC_RESULT(1,RNGC_CORE,0x0006)
#define RNGC_RESULTS_TOO_LATE		MAKE_RNGC_RESULT(1,RNGC_CORE,0x0007)
#define RNGC_IGNORE			MAKE_RNGC_RESULT(1,RNGC_CORE,0x0008)
#define RNGC_BUFFERTOOSMALL		MAKE_RNGC_RESULT(1,RNGC_CORE,0x0009)
#define RNGC_NOTFOUND		        MAKE_RNGC_RESULT(1,RNGC_CORE,0x000A)
#define RNGC_QUEUE_EMPTY                MAKE_RNGC_RESULT(0,RNGC_CORE,0x000B) // not an error
#define RNGC_TIMED_OUT                  MAKE_RNGC_RESULT(0,RNGC_CORE,0x000C) // not an error
#define RNGC_NOTINITIALIZED	        MAKE_RNGC_RESULT(1,RNGC_CORE,0x000D)

