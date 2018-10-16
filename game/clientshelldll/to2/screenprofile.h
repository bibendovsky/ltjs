// ScreenProfile.h: interface for the ScreenProfile class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SCREEN_PROFILE_H
#define SCREEN_PROFILE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "basescreen.h"

#include "profilemgr.h"

#pragma warning (disable : 4503)
#pragma warning( disable : 4786 )
#include <vector>
#include <string>

// BBi Conflicts with Microsoft's macro
//const int MAX_PROFILE_LEN = 16;
const int MAX_PROFILE_LENGTH = 16;

class CScreenProfile : public CBaseScreen
{
public:
	
	CScreenProfile();
	virtual ~CScreenProfile();

	// Build the screen
    LTBOOL   Build();

    uint32			OnCommand(uint32 dwCommand, std::uintptr_t dwParam1, std::uintptr_t dwParam2) override;
	void			Escape();
	void			OnFocus(LTBOOL bFocus);
	LTBOOL			OnRButtonUp(int x, int y);
	LTBOOL			OnLButtonUp(int x, int y);

private:
    void	HandleDlgCommand(uint32 dwCommand, uint16 nIndex);
	void	CreateProfileList();
	void	UpdateProfileName();

	char	m_szProfile[MAX_PROFILE_LENGTH];
	char	m_szOldProfile[MAX_PROFILE_LENGTH];

	CLTGUITextCtrl	*m_pCurrent;
	CLTGUITextCtrl	*m_pLoad;
	CLTGUITextCtrl	*m_pDelete;
	CLTGUITextCtrl	*m_pRename;


	StringSet			m_ProfileList;
	CLTGUIListCtrl*		m_pListCtrl;
	CLTGUIWindow*		m_pDlg;

	CUserProfile*		m_pProfile;

};

#endif // SCREEN_PROFILE_H