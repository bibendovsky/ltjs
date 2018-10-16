// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenSave.h
//
// PURPOSE : Interface screen for saving games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_SCREEN_SAVE_H_)
#define _SCREEN_SAVE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "basescreen.h"

class CScreenSave : public CBaseScreen
{
public:
	CScreenSave();
	virtual ~CScreenSave();

    LTBOOL Build();
    void OnFocus(LTBOOL bFocus);

	LTBOOL HandleKeyDown(int key, int rep);

	LTBOOL SaveGame(uint32 slot);

protected:
    uint32  OnCommand(uint32 dwCommand, std::uintptr_t dwParam1, std::uintptr_t dwParam2) override;
	void	BuildSavedLevelList();
	void	ClearSavedLevelList();

	void	NameSaveGame(uint32 slot, int index);

	char				m_szSaveName[40];

};

#endif // !defined(_SCREEN_SAVE_H_)