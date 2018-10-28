// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenKeyboard.h
//
// PURPOSE : Interface screen for keyboard settings
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef _SCREEN_KEYBOARD_H_
#define _SCREEN_KEYBOARD_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "basescreen.h"

class CScreenKeyboard : public CBaseScreen
{
public:
	CScreenKeyboard();
	virtual ~CScreenKeyboard();

	// Build the screen
    LTBOOL   Build();
    void OnFocus(LTBOOL bFocus);

protected:
	void	ClearBindings();
    uint32  OnCommand(uint32 dwCommand, std::uintptr_t dwParam1, std::uintptr_t dwParam2) override;

	CLTGUIToggle	*m_pLookCtrl;

    LTBOOL          m_bAutoCenter;
	int				m_nNormalTurn;
	int	  			m_nFastTurn;
	int	  			m_nVehicleTurn;
	int				m_nLookUp;


};

#endif // _SCREEN_KEYBOARD_H_