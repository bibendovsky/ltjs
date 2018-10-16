// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenJoinLAN.h
//
// PURPOSE : Interface screen to search for and join LAN games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef _SCREEN_JOIN_LAN_H_
#define _SCREEN_JOIN_LAN_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "basescreen.h"

class CScreenJoinLAN : public CBaseScreen
{
public:
	CScreenJoinLAN();
	virtual ~CScreenJoinLAN();

	// Build the screen
    LTBOOL   Build();

	LTBOOL	 Render(HSURFACE hDestSurf);

    void    OnFocus(LTBOOL bFocus);
	LTBOOL	HandleKeyDown(int key, int rep);

	void	CheckPassword();
	void	DoJoin();

protected:
    uint32  OnCommand(uint32 dwCommand, std::uintptr_t dwParam1, std::uintptr_t dwParam2) override;

	LTBOOL	InitSessions();
	void	FindServers();
	LTBOOL	SetService();
	LTBOOL	IsCurrentGame(CString sAddress);

	LTBOOL	IsValidPort(uint16 nPort);

	int		m_nFrameDelay;

	char m_szPort[8];

	CLTGUITextCtrl*		m_pFind;
	CLTGUIColumnCtrl*	m_pPort;
	CLTGUIListCtrl*		m_pServers;

	CMoArray<CString>	m_lstSessions;

	CString				m_sPassword;

};

inline LTBOOL CScreenJoinLAN::IsValidPort(uint16 nPort)
{
	return (nPort > 0);
}


#endif // _SCREEN_MULTI_H_