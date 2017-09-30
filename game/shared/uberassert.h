// ----------------------------------------------------------------------- //
//
// MODULE  : UberAssert.h
//
// PURPOSE : More powerful assert declaration:
//           - breaks at the line of code that called assert.
//           - gives a plain english explaination, along with the expression.
//           - shows a stack trace.
//           - copies output to windows clipboard.
//
// CREATED : 6/27/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __UBER_ASSERT_H__
#define __UBER_ASSERT_H__

#include <cassert>


#ifdef _DEBUG

#ifdef __MINGW32__
    #define UBER_ASSERT( exp, desc ) (assert(exp))
    #define UBER_ASSERT1( exp, desc, d1 ) (assert(exp))
    #define UBER_ASSERT2( exp, desc, d1, d2 ) (assert(exp))
    #define UBER_ASSERT3( exp, desc, d1, d2, d3 ) (assert(exp))
    #define UBER_ASSERT4( exp, desc, d1, d2, d3, d4 ) (assert(exp))
    #define UBER_ASSERT5( exp, desc, d1, d2, d3, d4, d5 ) (assert(exp))
#else
	extern bool UberAssert( long nLine, char const* szFile, char const* szExp, char const* szDesc, ... );

	#define UBER_ASSERT( exp, description ) \
		if ( !(exp) ) { \
			if( UberAssert( __LINE__, __FILE__, #exp, description )) \
			{ \
				_asm { int 3 } \
			} \
		}   

	#define UBER_ASSERT0	UBER_ASSERT

	#define UBER_ASSERT1( exp, desc, d1 ) \
		if ( !(exp) ) { \
			if( UberAssert( __LINE__, __FILE__, #exp, desc, d1 )) \
			{ \
				_asm { int 3 } \
			} \
		}   
	#define UBER_ASSERT2( exp, desc, d1, d2 ) \
		if ( !(exp) ) { \
			if( UberAssert( __LINE__, __FILE__, #exp, desc, d1, d2 )) \
			{ \
				_asm { int 3 } \
			} \
		}   
	#define UBER_ASSERT3( exp, desc, d1, d2, d3 ) \
		if ( !(exp) ) { \
			if( UberAssert( __LINE__, __FILE__, #exp, desc, d1, d2, d3 )) \
			{ \
				_asm { int 3 } \
			} \
		}   
	#define UBER_ASSERT4( exp, desc, d1, d2, d3, d4 ) \
		if ( !(exp) ) { \
			if( UberAssert( __LINE__, __FILE__, #exp, desc, d1, d2, d3, d4 )) \
			{ \
				_asm { int 3 } \
			} \
		}   
	#define UBER_ASSERT5( exp, desc, d1, d2, d3, d4, d5 ) \
		if ( !(exp) ) { \
			if( UberAssert( __LINE__, __FILE__, #exp, desc, d1, d2, d3, d4, d5 )) \
			{ \
				_asm { int 3 } \
			} \
		}   
#endif

#else	// ndef _DEBUG

	#define UBER_ASSERT( exp, desc )		(void)0
	#define UBER_ASSERT1( exp, desc, d1 )		(void)0
	#define UBER_ASSERT2( exp, desc, d1, d2 )		(void)0
	#define UBER_ASSERT3( exp, desc, d1, d2, d3 )		(void)0
	#define UBER_ASSERT4( exp, desc, d1, d2, d3, d4 )		(void)0
	#define UBER_ASSERT5( exp, desc, d1, d2, d3, d4, d5 )		(void)0

#endif


#endif // _UBER_ASSERT_H_

