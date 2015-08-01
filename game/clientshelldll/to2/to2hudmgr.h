// ----------------------------------------------------------------------- //
//
// MODULE  : TO2HUDMgr.h
//
// PURPOSE : Definition of CTO2HUDMgr class
//
// CREATED : 07/17/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TO2_HUDMGR_H
#define __TO2_HUDMGR_H

#include "hudmgr.h"
#include "hudammo.h"
#include "hudair.h"
#include "hudcrosshair.h"
#include "hudcompass.h"
#include "hudhiding.h"
#include "hudcarrying.h"
#include "hudhealth.h"
#include "huddamagedir.h"
#include "hudchooser.h"
#include "huddistance.h"
#include "hudobjectives.h"
#include "hudradio.h"
#include "hudprogressbar.h"
#include "hudrespawn.h"
#include "huddoomsday.h"


//******************************************************************************************
//** HUD Manager
//******************************************************************************************
class CTO2HUDMgr : public CHUDMgr
{
public:

	CTO2HUDMgr();
	~CTO2HUDMgr();

    virtual	LTBOOL		Init();

protected:

	CHUDAmmo			m_Ammo;
	CHUDAir				m_Air;
	CHUDCrosshair		m_Crosshair;
	CHUDCompass			m_Compass;
	CHUDHiding			m_Hiding;
	CHUDCarrying		m_Carrying;
	CHUDObjectives		m_Objectives;
	CHUDHealth			m_Health;
	CHUDDamageDir		m_DamageDir;
	CHUDWpnChooser		m_WpnChooser;
	CHUDAmmoChooser		m_AmmoChooser;
	CHUDDistance		m_Distance;
	CHUDRadio			m_Radio;
	CHUDProgressBar		m_ProgressBar;
	CHUDRespawn			m_Respawn;
	CHUDDoomsday		m_Doomsday;

};


extern CHUDCrosshair*		g_pCrosshair;
extern CHUDCompass*			g_pCompass;
extern CHUDObjectives*		g_pObjectives;
extern CHUDRadio*			g_pRadio;

#endif
