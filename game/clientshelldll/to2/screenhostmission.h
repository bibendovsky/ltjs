// ScreenHostMission.h: interface for the ScreenHostMission class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SCREEN_HOST_MISSION_H
#define SCREEN_HOST_MISSION_H


#include "basescreen.h"
#include "profilemgr.h"


#pragma warning (disable : 4503)
#pragma warning( disable : 4786 )
#include <vector>
#include <string>


class CScreenHostMission : public CBaseScreen
{
public:
	
	CScreenHostMission();
	virtual ~CScreenHostMission();

	// Build the screen
    LTBOOL   Build();

    uint32	OnCommand(uint32 dwCommand, std::uintptr_t dwParam1, std::uintptr_t dwParam2) override;
	void			Escape();
	void			OnFocus(LTBOOL bFocus);
	LTBOOL			OnRButtonUp(int x, int y);
	LTBOOL			OnLButtonUp(int x, int y);

private:
    void	HandleDlgCommand(uint32 dwCommand, uint16 nIndex);
	void	CreateCampaignList();
	void	UpdateCampaignName();
	void	DeleteCampaign(const std::string& campaignName);
	void	NewCampaign(const std::string& campaignName);
	void	RenameCampaign(const std::string& oldName,const std::string& newName);
	void	SetCampaignName(const char* newName);
	
	std::string	m_sOldCampaign;

	CLTGUITextCtrl	*m_pCurrent;
	CLTGUITextCtrl	*m_pLoad;
	CLTGUITextCtrl	*m_pDelete;
	CLTGUITextCtrl	*m_pRename;
	CLTGUITextCtrl	*m_pLevels;
	CLTGUITextCtrl	*m_pDefaultTextCtrl;


	StringSet			m_CampaignList;
	CLTGUIListCtrl*		m_pListCtrl;
	CLTGUIWindow*		m_pDlg;

	CUserProfile*		m_pProfile;

	uint16		m_nDefaultCampaign;
	GameType	m_eGameType;

};

#endif // SCREEN_HOST_MISSION_H