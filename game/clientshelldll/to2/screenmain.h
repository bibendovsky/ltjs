// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMain.h
//
// PURPOSE : Top level interface screen
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_MAIN_H_
#define _SCREEN_MAIN_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "basescreen.h"
#include "basescalefx.h"

class CScreenMain : public CBaseScreen
{
public:
	CScreenMain();
	virtual ~CScreenMain();

	// Build the screen
    LTBOOL   Build();

	// This is called when the screen gets or loses focus
    virtual void    OnFocus(LTBOOL bFocus);

	virtual void    Escape();

    LTBOOL   DoMultiplayer(LTBOOL bMinimize);


protected:
    uint32  OnCommand(uint32 dwCommand, std::uintptr_t dwParam1, std::uintptr_t dwParam2) override;
    LTBOOL   Render(HSURFACE hDestSurf);

	CLTGUITextCtrl* m_pResume;

	CLTGUIListCtrl* m_pGameType;
	CLTGUIFrame*	m_pGameTypeFrame;

};

#endif // _SCREEN_MAIN_H_