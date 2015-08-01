/****************************************************************************
 *
 *  $Id: rngctypes.h,v 1.7 2000/10/11 00:21:42 ingalls Exp $
 *
 *  Copyright (C) 1995-1999 RealNetworks, Inc. All rights reserved.
 *
 *  http://www.real.com/devzone
 *
 *  This program contains proprietary
 *  information of Progressive Networks, Inc, and is licensed
 *  subject to restrictions on use and distribution.
 *
 *  This file contains the custom types for use with the RN Game Console
 */

#include <objbase.h> /* Use Windows header files */

/** 
 * Helpfull macros
 *
 */
#define RNGC_DELETE(x) ((x) ? (delete (x), (x) = 0) : 0)
#define RNGC_VECTOR_DELETE(x) ((x) ? (delete [] (x), (x) = 0) : 0)
#define RNGC_RELEASE(x) ((x) ? ((x)->Release(), (x) = 0) : 0)

inline OLECHAR *SAFEBSTR(BSTR b){return b ? b : OLESTR("");}

#define RNGC_MAX_STRING 256

/*
	Defines the game startup modes.
*/
typedef enum __GameStartModes
{
	RNGC_ERROR = 0,
	RNGC_GAME_VALIDATE,
	RNGC_GAME_DESCRIBE,
	RNGC_GAME_START_SINGLE,
	RNGC_GAME_START_MULTI,
	RNGC_GAME_START_MULTI_TEAM,
	RNGC_GAME_START_MULTI_LADDER,
	RNGC_GAME_START_MULTI_TOURNEY,
	RNGC_GAME_START_SAVED,
	RNGC_GAME_EXIT = 0xFFFF	
} RNGC_GAME_START_MODES;
