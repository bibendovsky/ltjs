// ----------------------------------------------------------------------- //
//
// MODULE  : MenuMission.h
//
// PURPOSE : In-game system menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_MENU_MISSION_H_)
#define _MENU_MISSION_H_

#include "basemenu.h"


class CMenuMission : public CBaseMenu
{
public:
	CMenuMission();

	virtual LTBOOL	Init();
	virtual void	Term();


	// Handle a command
    uint32 OnCommand(uint32 nCommand, std::uintptr_t nParam1, std::uintptr_t nParam2) override;

	virtual void OnFocus(LTBOOL bFocus);

	uint16 AddObjectiveControl (int stringID, LTBOOL bCompleted = LTFALSE, LTBOOL bOptional = LTFALSE);

private:
	void RemoveObjectives();

	CLTGUITextCtrl* m_pNameCtrl;
	CLTGUITextCtrl* m_pLevelCtrl;
	CLTGUITextCtrl*	m_pObjLabel;
	CLTGUITextCtrl*	m_pParameters;

	uint16 m_nFirstObj;

	ControlArray	m_Objectives;

	HTEXTURE		m_BulletTex;
	HTEXTURE		m_CompletedTex;

};

#endif //!defined(_MENU_MISSION_H_)