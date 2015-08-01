// ----------------------------------------------------------------------- //
//
// MODULE  : GadgetTargetFX.cpp
//
// PURPOSE : GadgetTarget - Implementation
//
// CREATED : 8/20/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "gadgettargetfx.h"
#include "iltclient.h"
#include "clientutilities.h"
#include "clientservershared.h"
#include "gameclientshell.h"
#include "sfxmsgids.h"

extern CGameClientShell* g_pGameClientShell;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGadgetTargetFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CGadgetTargetFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	GADGETTARGETCREATESTRUCT* pGTCS = (GADGETTARGETCREATESTRUCT*)psfxCreateStruct;

	m_eType = pGTCS->eType;
	m_bSwitchWeapons = pGTCS->bSwitchWeapons;
	m_bPowerOn = pGTCS->bPowerOn;
	m_nTeamID = pGTCS->nTeamID;

    return LTTRUE;
}


LTBOOL CGadgetTargetFX::OnServerMessage(ILTMessage_Read *pMsg)
{
	if (!CSpecialFX::OnServerMessage(pMsg)) return LTFALSE;
	m_eType = (GadgetTargetType)pMsg->Readuint8();
	m_bSwitchWeapons = pMsg->Readbool();
	m_bPowerOn = pMsg->Readbool();
	m_nTeamID = pMsg->Readuint8();

	return LTTRUE;
};


