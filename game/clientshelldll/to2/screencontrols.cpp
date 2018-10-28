// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenControls.cpp
//
// PURPOSE : Interface screen for setting control options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "screencontrols.h"
#include "screenmgr.h"
#include "screencommands.h"
#include "gamesettings.h"
#include "interfacemgr.h"
#include "gamesettings.h"
#include "gameclientshell.h"

namespace
{
	int kGap = 200;
	int kWidth = 200;

	void ConfirmCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenControls *pThisScreen = (CScreenControls *)pData;
		if (pThisScreen)
		{
			pThisScreen->ConfirmSetting(bReturn);
		}
	}


	bool bHasJoystick = true;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenControls::CScreenControls()
{
    m_pJoystickCtrl = LTNULL;

    m_bUseJoystick=LTFALSE;
	m_nConfirm = 0;

	uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();
	bHasJoystick = (dwAdvancedOptions & AO_JOYSTICK) != 0;
	if (bHasJoystick)
	{
		bHasJoystick = g_pGameClientShell->HasJoystick() || g_pGameClientShell->HasGamepad();
	}
}

CScreenControls::~CScreenControls()
{

}

// Build the screen
LTBOOL CScreenControls::Build()
{
	CreateTitle(IDS_TITLE_CONTROLS);
	
	kGap = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_CONTROLS,"ColumnWidth");
	kWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_CONTROLS,"SliderWidth");

	//customize
	AddTextItem(IDS_CONFIGURE, CMD_CONFIGURE, IDS_HELP_CUSTOMCONTROLS);

	// Mouse
	AddTextItem(IDS_MOUSE, CMD_MOUSE, IDS_HELP_MOUSE);

	// keyboard
	AddTextItem(IDS_KEYBOARD, CMD_KEYBOARD, IDS_HELP_KEYBOARD);

	// use joystick
	CLTGUIToggle* pToggle = AddToggle(IDS_JOYSTICK_USE, IDS_HELP_USEJOYSTICK, kGap, &m_bUseJoystick);
	pToggle->NotifyOnChange(CMD_UPDATE,this);
	char szYes[16];
	FormatString(IDS_YES,szYes,sizeof(szYes));
	char szNo[16];
	FormatString(IDS_NO,szNo,sizeof(szNo));
	pToggle->SetOnString(szYes);
	pToggle->SetOffString(szNo);
	pToggle->Enable( bHasJoystick ? LTTRUE : LTFALSE );
//	pToggle->Enable(LTFALSE);


	// joystick
	m_pJoystickCtrl = AddTextItem(IDS_JOYSTICK, CMD_JOYSTICK, IDS_HELP_JOYSTICK);
	m_pJoystickCtrl->Enable( bHasJoystick );
//	m_pJoystickCtrl->Enable(LTFALSE);

	//restore
	AddTextItem(IDS_RESTOREDEFAULTS, CMD_RESET_DEFAULTS, IDS_HELP_RESTORE);


	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

//	CScreenJoystick *pJoy = (CScreenJoystick *)m_pScreenMgr->GetScreenFromID(SCREEN_ID_JOYSTICK);
//	pJoy->Build();

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;

}

uint32 CScreenControls::OnCommand(uint32 dwCommand, std::uintptr_t dwParam1, std::uintptr_t dwParam2)
{
	switch(dwCommand)
	{
	case CMD_CONFIGURE:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_CONFIGURE);
			break;
		}
	case CMD_UPDATE:
		{

			UpdateData();
			m_pJoystickCtrl->Enable(bHasJoystick && m_bUseJoystick);
			break;

		}
	case CMD_MOUSE:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_MOUSE);
			break;
		}
	case CMD_KEYBOARD:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_KEYBOARD);
			break;
		}
	case CMD_JOYSTICK:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_JOYSTICK);
			break;
		}
	case CMD_RESET_DEFAULTS:
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = ConfirmCallBack;
			mb.pData = this;
			m_nConfirm = IDS_CONFIRM_RESTORE;
			g_pInterfaceMgr->ShowMessageBox(IDS_CONFIRM_RESTORE,&mb);
			break;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);

	}
	return 1;
};


// Change in focus
void CScreenControls::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{

		pProfile->SetControls();

		m_bUseJoystick = pProfile->m_bUseJoystick;

		m_pJoystickCtrl->Enable(bHasJoystick && m_bUseJoystick);

        UpdateData(LTFALSE);
	}
	else
	{
		bool bJoystickChanged = !(pProfile->m_bUseJoystick == m_bUseJoystick);
		pProfile->m_bUseJoystick = m_bUseJoystick;

		pProfile->ApplyControls();
		if (bJoystickChanged)
			pProfile->ApplyJoystick();
		pProfile->Save();

		UpdateData();
	}
	CBaseScreen::OnFocus(bFocus);
}


void CScreenControls::ConfirmSetting(LTBOOL bConfirm)
{
	switch (m_nConfirm)
	{

	case IDS_JOYSTICK_UNBOUND:
/*
		if (bConfirm)
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_JOYSTICK);
		else
		{
			m_bUseJoystick = LTFALSE;
			UpdateData(LTFALSE);
		}
*/
		break;

	case IDS_CONFIRM_RESTORE:
		if (bConfirm)
		{
			CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
			pProfile->RestoreDefaults(PROFILE_CONTROLS);
		}
		break;
	}

}