// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenJoinLAN.cpp
//
// PURPOSE : Interface screen to search for and join LAN games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "screenjoinlan.h"
#include "screenmgr.h"
#include "screencommands.h"
#include "clientres.h"
#include "clientmultiplayermgr.h"
#include "missionmgr.h"

#include "gameclientshell.h"

#ifdef LTJS_SDL_BACKEND
#include "SDL.h"
#endif // LTJS_SDL_BACKEND

void JoinLANCallBack(LTBOOL bReturn, void *pData);

namespace
{
	int kLockWidth = 50;
	int kColumnWidth = 200;
	int kAddressWidth = 150;
	char	szOldPort[8];
	int kListFontSize = 12;
	LTBOOL bIsInitting = LTFALSE;
	void EditPortCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenJoinLAN *pThisScreen = (CScreenJoinLAN *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_JOIN_LAN);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,reinterpret_cast<std::uintptr_t>(pData),CMD_EDIT_PORT);
	};

	void EditPassCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenJoinLAN *pThisScreen = (CScreenJoinLAN *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_JOIN_LAN);
		if (bReturn && pThisScreen)
		{
			pThisScreen->SendCommand(CMD_OK,reinterpret_cast<std::uintptr_t>(pData),CMD_EDIT_PASS);
		}
	};

	std::string s_Name = "";
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenJoinLAN::CScreenJoinLAN()
{
}

CScreenJoinLAN::~CScreenJoinLAN()
{

}

// Build the screen
LTBOOL CScreenJoinLAN::Build()
{
	if (!CBaseScreen::Build()) return LTFALSE;

	CreateTitle(IDS_TITLE_JOIN);
	kLockWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN_LAN,"LockWidth");
	kColumnWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN_LAN,"ColumnWidth");
	kAddressWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN_LAN,"AddressWidth");
	kListFontSize = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN_LAN,"ListFontSize");

	m_pFind = AddTextItem(IDS_FIND_SERVERS,CMD_SEARCH,IDS_HELP_FIND_SERVERS);

	char szTmp[64];
	m_pPort = AddColumnCtrl(CMD_EDIT_PORT, IDS_HELP_ENTER_PORT);
	FormatString(IDS_PORT,szTmp,sizeof(szTmp));
	m_pPort->AddColumn(szTmp, static_cast<uint16>(kColumnWidth));
	m_pPort->AddColumn("27888", static_cast<uint16>(kColumnWidth), LTTRUE);
	SAFE_STRCPY(m_szPort,"27888");


	LTIntPt pos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_JOIN_LAN,"ListPos");
	CLTGUIColumnCtrl* pCtrl = AddColumnCtrl(LTNULL, LTNULL, pos, LTTRUE);
	pCtrl->AddColumn("Lock", static_cast<uint16>(kLockWidth));
	pCtrl->AddColumn(LoadTempString(IDS_SERVER_NAME), static_cast<uint16>(kColumnWidth));
	pCtrl->AddColumn(LoadTempString(IDS_SERVER_ADDRESS), static_cast<uint16>(kAddressWidth));
	pCtrl->Enable(LTFALSE);

	uint16 height = static_cast<uint16>(GetPageBottom() - m_nextPos.y);
	m_pServers = AddList(m_nextPos,height,LTTRUE,static_cast<uint16>(kColumnWidth+kAddressWidth+kLockWidth));
	m_pServers->SetScrollWrap(LTFALSE);
	m_pServers->SetFrameWidth(1);
	m_pServers->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);


 	// Make sure to call the base class
	return LTTRUE;
}

uint32 CScreenJoinLAN::OnCommand(uint32 dwCommand, std::uintptr_t dwParam1, std::uintptr_t dwParam2)
{
	switch(dwCommand)
	{
	case CMD_SEARCH:
		{
			FindServers();
		}
		break;
	case CMD_EDIT_PORT:
		{
			//show edit box here	
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditPortCallBack;
			mb.pString = m_szPort;
			mb.nMaxChars = 5;
			mb.eInput = CLTGUIEditCtrl::kInputNumberOnly;
			g_pInterfaceMgr->ShowMessageBox(IDS_PORT,&mb);
		} break;
	case CMD_OK:
		{
			if (dwParam2 == CMD_EDIT_PORT)
			{
				char *pszPort = (char *)dwParam1;
				uint16 nPort = (uint16)atoi(pszPort);
				if (IsValidPort(nPort))
				{
					SAFE_STRCPY(m_szPort,pszPort);
					m_pPort->SetString(1,m_szPort);
				}
			}
			else if (dwParam2 == CMD_EDIT_PASS)
			{
				m_sPassword = (char *)dwParam1;
				DoJoin();
			}
		}
		break;
	case CMD_JOIN:
		{
			if (dwParam1 >= m_lstSessions.GetSize()) return 0;

		    if (g_pGameClientShell->IsWorldLoaded())
		    {

				if (IsCurrentGame(m_lstSessions[dwParam1]))
				{
					HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
					if (g_pGameClientShell->IsWorldLoaded() && hPlayerObj)
					{
						g_pInterfaceMgr->ChangeState(GS_PLAYING);
					}
				}
				else
				{
					MBCreate mb;
					mb.eType = LTMB_YESNO;
					mb.pFn = JoinLANCallBack;
					g_pInterfaceMgr->ShowMessageBox(IDS_ENDCURRENTGAME,&mb);
				}
		    }
		    else
		    {
				CheckPassword();
		    }


		} break;

	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void    CScreenJoinLAN::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	if (bFocus)
	{

		// we may have gotten here after failing a join, so rebuild our history
		if (m_pScreenMgr->GetLastScreenID() == SCREEN_ID_NONE)
		{
			m_pScreenMgr->AddScreenToHistory(SCREEN_ID_MAIN);
			m_pScreenMgr->AddScreenToHistory(SCREEN_ID_MULTI);
		}

		sprintf(m_szPort,"%d",pProfile->m_nClientPort);
		m_pPort->GetColumn(1)->SetString(m_szPort);

		SetSelection(GetIndex(m_pFind));
		m_pServers->RemoveAll();

        UpdateData(LTFALSE);
	}
	else
	{
		pProfile->m_nClientPort = (uint16)atoi(m_szPort);
		pProfile->Save();

		UpdateData();
	}
	CBaseScreen::OnFocus(bFocus);
}


LTBOOL CScreenJoinLAN::InitSessions()
{
	// Remove all of the menu options
	m_pServers->RemoveAll();

	// Remove the sessions
	m_lstSessions.RemoveAll();

	CString csString;
	NetSession *pList, *pSession;
	LTIntPt pt;
	

	// We need to make sure the engine's internet driver is ready before we can get the session list
	// NOTE: The Client_de function AddInternetDriver and RemoveInternetDriver were tiny little functions that I added
	// to Sanity's version of LithTech v1.5 (you can see them under LithTechSanity in SS)
	LTRESULT res = g_pLTClient->AddInternetDriver();
	if(res == LT_ERROR)
		return LTFALSE;

	// This is a function in clientshell.cpp (cut & pasted below) that finds the engine's TCP/IP service and sets it.
	if(!g_pClientMultiplayerMgr->SetService())
	{
		// Remove the internet driver if we added it
		if(res != LT_ALREADYEXISTS)
			g_pLTClient->RemoveInternetDriver();
		return LTFALSE;
	}

	char sTemp[16];
	uint16 nPort = (uint16)atoi(m_szPort);
	sprintf(sTemp,"*:%d",nPort);

	// Find the sessions
	if((g_pLTClient->GetSessionList( pList, sTemp ) == LT_OK) && pList)
	{
		pSession = pList;
		while(pSession)
		{
			uint8 nGameType = pSession->m_nGameType;

			//since versions 1.1 and 1.1 didn't have this they'll report 0 for co-op games
			if (nGameType == 0)
				nGameType = eGameTypeCooperative;

			if (g_pGameClientShell->GetGameType() != nGameType)
			{
				pSession = pSession->m_pNext;
				continue;
			}

			csString.Format( "%s:%d", pSession->m_HostIP, pSession->m_HostPort );

			if (!m_lstSessions.Add(csString))
			{
				g_pLTClient->FreeSessionList( pList );
				// Remove the internet driver if we added it
				if(res != LT_ALREADYEXISTS)
					g_pLTClient->RemoveInternetDriver();
				return LTFALSE;
			}
			uint32 pos = m_lstSessions.GetSize()-1;

			// Add the session to our list o' sessions
			CLTGUIColumnCtrl* pCtrl = CreateColumnCtrl(CMD_JOIN, IDS_HELP_JOIN_LAN);
			if(!pCtrl)
			{
				g_pLTClient->FreeSessionList( pList );
				// Remove the internet driver if we added it
				if(res != LT_ALREADYEXISTS)
					g_pLTClient->RemoveInternetDriver();
				return LTFALSE;
			}
			pCtrl->SetFont(LTNULL,static_cast<uint8>(kListFontSize));

			if (pSession->m_bHasPassword)
				pCtrl->AddColumn("x", static_cast<uint16>(kLockWidth));
			else
				pCtrl->AddColumn(" ", static_cast<uint16>(kLockWidth));

			// Do the name
			pCtrl->AddColumn(pSession->m_sName, static_cast<uint16>(kColumnWidth));
			pCtrl->SetParam1((uint32)pos);
			pCtrl->SetParam2((uint32)pSession->m_bHasPassword);

/*
			char szPlayers[16] ="";
			sprintf(szPlayers,"%d/%d",pSession->m_dwCurPlayers,pSession->m_dwMaxPlayers);
			hString = g_pLTClient->CreateString(szPlayers);
			pCtrl->AddColumn(hString, 50, LTF_JUSTIFY_LEFT);
			TERMSTRING(hString);
*/			
			
			// Do the address
			pCtrl->AddColumn((LPCTSTR)csString, static_cast<uint16>(kAddressWidth));
			m_pServers->AddControl(pCtrl);



			pSession = pSession->m_pNext;
		}
		g_pLTClient->FreeSessionList( pList );
	}

	// Remove the internet driver if we added it
	if(res != LT_ALREADYEXISTS)
		g_pLTClient->RemoveInternetDriver();


	return LTTRUE;
}


LTBOOL CScreenJoinLAN::IsCurrentGame(CString sAddress)
{
	if (!IsMultiplayerGame( )) return LTFALSE;

	char sTemp[MAX_SGR_STRINGLEN];
	strcpy(sTemp,(char *)(LPCTSTR)sAddress);
	char *pTok = strtok(sTemp,":");
	pTok = strtok(LTNULL,":");
	int p = atoi(pTok);

	return (g_pClientMultiplayerMgr->CheckServerAddress(sTemp,p));

}


void JoinLANCallBack(LTBOOL bReturn, void *pData)
{
	CScreenJoinLAN *pThisScreen = (CScreenJoinLAN *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_JOIN_LAN);
	if (bReturn && pThisScreen)
	{
		pThisScreen->CheckPassword();
    }
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenJoinLAN::JoinGame
//
//	PURPOSE:	Joins the given game service
//
// ----------------------------------------------------------------------- //

void CScreenJoinLAN::CheckPassword()
{
	// Get the currently selected game server...
	CLTGUIColumnCtrl *pCtrl = (CLTGUIColumnCtrl *)m_pServers->GetSelectedControl();
	if (!pCtrl) return;

	uint32 nIndex = pCtrl->GetParam1();
	if (nIndex > m_lstSessions.GetSize()) return;


	if (IsCurrentGame(m_lstSessions[nIndex]))
	{
        HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		if (g_pGameClientShell->IsWorldLoaded() && hPlayerObj)
		{
			g_pInterfaceMgr->ChangeState(GS_PLAYING);
		}
		return;
	}

	m_sPassword = "";
	if (pCtrl->GetColumn(1) && pCtrl->GetColumn(1)->GetString())
		s_Name = pCtrl->GetColumn(1)->GetString()->GetText();
	else
		s_Name = "";

	if (pCtrl->GetParam2())
	{	
		//show edit box here	
		MBCreate mb;
		mb.eType = LTMB_EDIT;
		mb.pFn = EditPassCallBack;
		mb.pString = "";
		mb.nMaxChars = MAX_PASSWORD-1;
		g_pInterfaceMgr->ShowMessageBox(IDS_PASSWORD,&mb);
	}
	else
	{
		DoJoin();
	}
}

void CScreenJoinLAN::DoJoin()
{
	// Get the currently selected game server...
	CLTGUIColumnCtrl *pCtrl = (CLTGUIColumnCtrl *)m_pServers->GetSelectedControl();
	if (!pCtrl) return;

	uint32 nIndex = pCtrl->GetParam1();
	if (nIndex > m_lstSessions.GetSize()) return;

	bool bOk = g_pClientMultiplayerMgr->SetupClient(m_lstSessions[nIndex], s_Name.c_str(), m_sPassword);
	bOk = bOk && g_pMissionMgr->StartGameAsClient( );

	if( !bOk )
	{
		g_pInterfaceMgr->LoadFailed(SCREEN_ID_JOIN_LAN);
	}
}


void CScreenJoinLAN::FindServers()
{
	if (!bIsInitting)
	{
		g_pInterfaceResMgr->DrawMessage(IDS_LOOKING_FOR_SERVERS);

		g_pLTClient->CPrint("init sessions");
		bIsInitting = LTTRUE;
		InitSessions();

		if (m_lstSessions.GetSize() == 0)
		{
			CLTGUITextCtrl *pCtrl = CreateTextItem(IDS_NO_SERVERS, LTNULL, LTNULL, kDefaultPos, LTTRUE);
			pCtrl->SetFont(LTNULL,static_cast<uint8>(kListFontSize));
			m_pServers->AddControl(pCtrl);
		}
		g_pLTClient->ClearInput();
	}
}





LTBOOL CScreenJoinLAN::Render(HSURFACE hDestSurf)
{
	bIsInitting = LTFALSE;
	return CBaseScreen::Render(hDestSurf);
}


LTBOOL CScreenJoinLAN::HandleKeyDown(int key, int rep)
{
#ifdef LTJS_SDL_BACKEND
	if (key == ::SDLK_F5)
#else
	if (key == VK_F5)
#endif // LTJS_SDL_BACKEND
	{
		FindServers();
        return LTTRUE;
	}
    return CBaseScreen::HandleKeyDown(key,rep);
}