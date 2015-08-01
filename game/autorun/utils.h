// *********************************************************************** //
//
//	MODULE:		UTILS (.H)
//
//	PURPOSE:	Gernal purpose utilities
//
//	HISTORY:	02/15/95 [blg] This file was created
//
//	NOTICE:		Copyright (c) 1995, Monolith Productions, Inc.
//
// *********************************************************************** //

#ifndef _UTILS_H_
#define _UTILS_H_


// Prototypes...

BOOL	ExistProcess(const char* sExe, int thresh = 0, HANDLE* phProcess = NULL);
BOOL    ExistWindow(char *winName);


// EOF...

#endif