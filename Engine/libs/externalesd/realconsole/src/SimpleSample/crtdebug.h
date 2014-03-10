//---------------------------------------------------------------------------
//
//	CRT_DEBUG.h
//
//
//	Description:	Diagnostic Trace	
//	Author:			Andrew Johnston
//	Notes:			Based on AFX routines
//
//	History:
//	
//	1     02/07/2001 7:18p Ajohnston
//---------------------------------------------------------------------------
#pragma once
#ifndef __CRT_DEBUG__
#define __CRT_DEBUG__
#ifndef __AFX_H__

#ifdef _DEBUG
#	ifndef _CRTDBG_MAP_ALLOC
#		define _CRTDBG_MAP_ALLOC
#	endif
#endif

#pragma warning(disable:4786)

#include <new>
#include <memory>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <crtdbg.h>
#include <cstdarg>

#include <windows.h>
#include <windef.h>
#include <winnt.h>
#include <Tchar.h>

#define ASSERT	_ASSERT				// use the CRT version of ASSERT

// determine number of elements in an array (not bytes)
#define _countof(array) (sizeof(array)/sizeof(array[0]))

#ifdef _DEBUG

	#define	SET_CRT_DEBUG_FIELD(a)		_CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
	#define	CLEAR_CRT_DEBUG_FIELD(a)	_CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))

	//---------------------------------------------------------------------------
	//	crtTrace	- equivalent of the afx TRACE
	//---------------------------------------------------------------------------
	inline void crtTrace(LPCTSTR lpszFormat, ...)
	{
		va_list args;
		va_start(args, lpszFormat);

		int nBuf;
		TCHAR szBuffer[512];

		nBuf = _vsntprintf(szBuffer, _countof(szBuffer), lpszFormat, args);

		// was there an error? was the expanded string too long?
		_ASSERT(nBuf >= 0);

		OutputDebugString(szBuffer);

		va_end(args);
	}

	//---------------------------------------------------------------------------
	//	defines
	//---------------------------------------------------------------------------
	#define TRACE	crtTrace

	#define _new_crt	new(_NORMAL_BLOCK, __FILE__, __LINE__)
	#define new			_new_crt

// Function Timing to within 1 microsecond (ticksPerSecond should be about 1.1 million)
// NOTE:  the overhead of timing is about 7us on a PII 233 Mhz, so about 1600 cycles
// Use the format:
//
// DBGOUT_START_TIMER( <block name> )
// {
//		what you want timed
// }
// DBGOUT_END_TIMER( <block name> )
//
// This should print out the time in microseconds in the debug window
//
#include <windowsx.h>

inline void _cdecl DbgPrint( LPCTSTR lpFmt, ... )
{
    va_list arglist;
    TCHAR   lpOutput[1024];

    va_start(arglist, lpFmt);
    vsprintf(lpOutput, lpFmt, arglist);
    va_end(arglist);

    OutputDebugString(lpOutput);
}


#define DBGOUT_START_TIMER( _block )										\
    _int64  _block##end, _block##start, _block##ticksPerSecond;				\
    QueryPerformanceFrequency( (LARGE_INTEGER*)&_block##ticksPerSecond );	\
    QueryPerformanceCounter( (LARGE_INTEGER *)&_block##start );

#define DBGOUT_END_TIMER( _block )											\
    QueryPerformanceCounter( (LARGE_INTEGER *)&_block##end );				\
    DbgPrint( #_block " time: %f us\n", 1000000.0 * (double)(_block##end - _block##start) / _block##ticksPerSecond );


#else

	#define  SET_CRT_DEBUG_FIELD(a)			((void) 0)
	#define  CLEAR_CRT_DEBUG_FIELD(a)		((void) 0)
	#define _new_crt	new

	inline void crtTrace(char*, ...) { }
	#define TRACE   1 ? (void)0 : ::crtTrace
	#define DBGOUT_START_TIMER( _block )	((void) 0)
	#define DBGOUT_END_TIMER( _block )		((void) 0)								
	
#endif

#else // __AFX_H__ defined
#ifdef _DEBUG
		#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif // _DEBUG_
#endif // #ifndef __AFX_H__

#endif	// __CRTDEBUGFUNCTIONS__