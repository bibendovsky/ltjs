// ----------------------------------------------------------------------- //
//
// MODULE  : MenuSystem.h
//
// PURPOSE : In-game system menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_MENU_SYSTEM_H_)
#define _MENU_SYSTEM_H_

#include "basemenu.h"


class CMenuSystem : public CBaseMenu
{
public:

	CMenuSystem( );

	virtual LTBOOL	Init();

	virtual void OnFocus(LTBOOL bFocus);

	// Handle a command
    uint32 OnCommand(uint32 nCommand, std::uintptr_t nParam1, std::uintptr_t nParam2) override;

private:

	CLTGUITextCtrl*	m_pSaveCtrl;
	CLTGUITextCtrl*	m_pLoadCtrl;
	CLTGUITextCtrl*	m_pPlayerCtrl;
	CLTGUITextCtrl*	m_pTeamCtrl;
	CLTGUITextCtrl*	m_pHostCtrl;
	CLTGUITextCtrl*	m_pServerCtrl;
};

#endif //!defined(_MENU_SYSTEM_H_)