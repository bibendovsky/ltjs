// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenGame.h
//
// PURPOSE : Interface screen for setting gameplay options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_GAME_H_
#define _SCREEN_GAME_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "basescreen.h"

class CScreenGame : public CBaseScreen
{
public:
	CScreenGame();
	virtual ~CScreenGame();

	// Build the folder
    LTBOOL   Build();
    void    OnFocus(LTBOOL bFocus);


protected:


    uint32  OnCommand(uint32 dwCommand, std::uintptr_t dwParam1, std::uintptr_t dwParam2) override;

protected:
    uint8	            m_nSubtitles;
    LTBOOL				m_bGore;
	uint8				m_nDifficulty;
	LTBOOL				m_bAlwaysRun;
	uint8				m_nLayout;
	int					m_nHeadBob;
	int					m_nWeaponSway;
	int					m_nMsgDur;
	LTBOOL				m_bAutoWeaponSwitch;
	LTBOOL				m_bLoadScreenTips;
	LTBOOL				m_bVehicleContour;

	CLTGUICycleCtrl		*m_pDifficultyCtrl;			// The difficulty control

};

#endif // _SCREEN_GAME_H_