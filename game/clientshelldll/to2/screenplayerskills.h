// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPlayerSkills.h
//
// PURPOSE : Interface screen for player setup
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_PLAYER_SKILLS_H_
#define _SCREEN_PLAYER_SKILLS_H_

#include "basescreen.h"
#include "layoutmgr.h"
#include "skillsbutemgr.h"

class CScreenPlayerSkills : public CBaseScreen
{
public:
	CScreenPlayerSkills();
	virtual ~CScreenPlayerSkills();


	// Build the screen
    LTBOOL   Build();
	void	Term();

    void    OnFocus(LTBOOL bFocus);

	bool	SetSkill(int nSkl, int nNew);

	virtual void	UpdateData(LTBOOL bSaveAndValidate=LTTRUE);

	virtual void	Escape();
	void			DoEscape();

protected:

	uint32 OnCommand(uint32 dwCommand, std::uintptr_t dwParam1, std::uintptr_t dwParam2) override;

	int					m_nPool;
	int					m_nLevels[kNumSkills];
	CLTGUISlider*		m_pSkill[kNumSkills];
	CLTGUICycleCtrl*	m_pLabel[kNumSkills];
	CLTGUITextCtrl*		m_pPool;


};

#endif // _SCREEN_PLAYER_SKILLS_H_