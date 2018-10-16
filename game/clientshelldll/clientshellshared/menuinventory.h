// ----------------------------------------------------------------------- //
//
// MODULE  : MenuInventory.h
//
// PURPOSE : In-game inventory menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_MENU_INVENTORY_H_)
#define _MENU_INVENTORY_H_

#include "basemenu.h"
#include "weaponmgr.h"

class CAmmoMenu : public CSubMenu
{
public:
    virtual LTBOOL  OnUp ( );
    virtual LTBOOL  OnDown ( );
	virtual	LTBOOL	OnMouseMove(int x, int y);

};


class CMenuInventory : public CBaseMenu
{
public:

	virtual LTBOOL	Init();
	virtual void	Term();

	virtual void OnFocus(LTBOOL bFocus);

	// Handle a command
    uint32 OnCommand(uint32 nCommand, std::uintptr_t nParam1, std::uintptr_t nParam2) override;

	void	UpdateAmmoText(bool bForce = false);

protected:

	CAmmoMenu		m_Popup;
	CLTGUITextCtrl	m_Name;
	CLTGUITextCtrl	m_Description;
	CLTGUIButton	m_Photo;
	CLTGUIListCtrl	m_Ammo;
	CLTGUITextCtrl	m_AmmoDesc;

	uint8			m_nAmmo;
	uint8			m_nAmmoID[WMGR_MAX_AMMO_IDS];

};

#endif //!defined(_MENU_INVENTORY_H_)