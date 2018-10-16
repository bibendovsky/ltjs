// ----------------------------------------------------------------------- //
//
// MODULE  : MenuKeys.h
//
// PURPOSE : In-game key item menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_MENU_KEYS_H_)
#define _MENU_KEYS_H_

#include "basemenu.h"

class CMenuKeys : public CBaseMenu
{
public:

	virtual LTBOOL	Init();
	virtual void	Term();

	virtual void OnFocus(LTBOOL bFocus);


	// Handle a command
    uint32 OnCommand(uint32 nCommand, std::uintptr_t nParam1, std::uintptr_t nParam2) override;

protected:

	CSubMenu		m_Popup;
	CLTGUITextCtrl	m_Name;
	CLTGUITextCtrl	m_Description;
	CLTGUIButton	m_Image;

};

#endif //!defined(_MENU_KEYS_H_)