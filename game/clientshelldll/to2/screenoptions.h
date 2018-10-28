// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenOptions.h
//
// PURPOSE : Interface screen for navigation to various option setting screens
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_OPTIONS_H_
#define _SCREEN_OPTIONS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "basescreen.h"

class CScreenOptions : public CBaseScreen
{
public:
	CScreenOptions();
	virtual ~CScreenOptions();

	// Build the screen
    LTBOOL   Build();

    void    OnFocus(LTBOOL bFocus);

protected:
    uint32  OnCommand(uint32 dwCommand, std::uintptr_t dwParam1, std::uintptr_t dwParam2) override;

};

#endif // _SCREEN_OPTIONS_H_