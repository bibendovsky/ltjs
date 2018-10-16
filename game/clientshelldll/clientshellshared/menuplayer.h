// ----------------------------------------------------------------------- //
//
// MODULE  : MenuPlayer.h
//
// PURPOSE : In-game player menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_MENU_PLAYER_H_)
#define _MENU_PLAYER_H_

#include "basemenu.h"
#include "skillsbutemgr.h"
#include "skillcontrol.h"

class CSkillPopup : public CSubMenu
{
public:
    virtual LTBOOL	HandleKeyDown(int key, int rep);
    virtual LTBOOL	OnUp ( );
    virtual LTBOOL  OnDown ( );
	virtual	LTBOOL	OnMouseMove(int x, int y);

	LTBOOL	m_bWaitForUpdate;
};

class CMenuPlayer : public CBaseMenu
{
public:

	virtual LTBOOL	Init();
	virtual void	Term();

	virtual void OnFocus(LTBOOL bFocus);


	// Handle a command
    uint32 OnCommand(uint32 nCommand, std::uintptr_t nParam1, std::uintptr_t nParam2) override;

	void	UpdateControls();
	void	UpdatePopup();
	void	UpdateModText(bool bForce = false);

protected:
	CLTGUIColumnCtrl* AddColumnCtrl(uint32 nCommand = 0);

protected:
	CSkillPopup			m_Popup;
	CLTGUITextCtrl		m_Name;
	CLTGUITextCtrl		m_Level;
	CLTGUIColumnCtrl	m_Header;
	CLTGUIColumnCtrl	m_Mods[kMaxModifiers];
	CLTGUITextCtrl		m_Upgrade;
	CLTGUITextCtrl		m_Points;
	CLTGUITextCtrl		m_ModDesc;

	CLTGUIColumnCtrl* m_pRank;
	CLTGUIColumnCtrl* m_pTotal;
	CLTGUIColumnCtrl* m_pAvail;

	CSkillCtrl* m_pSkills[kNumSkills];


};

#endif //!defined(_MENU_PLAYER_H_)