// ----------------------------------------------------------------------- //
//
// MODULE  : TronHUDMgr.h
//
// PURPOSE : Definition of derived CHUDMgr class
//
// CREATED : 11/5/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TRONHUDMGR_H
#define __TRONHUDMGR_H

#include "hudmgr.h"		// base class

#include "tronplayerstats.h"

// TO2 dependencies
#include "hudcrosshair.h"
#include "huddamage.h"
#include "hudweapons.h"

// Tron dependencies
#include "hudpermissions.h"
#include "hudversion.h"
#include "hudhealthenergy.h"
#include "hudarmor.h"
#include "hudenergytransfer.h"
#include "hudprogress.h"
#include "hudchooser.h"
#include "hudweapon.h"

//******************************************************************************************
//** Tron HUD Manager
//******************************************************************************************
class CTronHUDMgr: public CHUDMgr
{
public:

	CTronHUDMgr();
	~CTronHUDMgr();

	LTBOOL		Init();

protected:

	CHUDCrosshair		m_Crosshair;
	CHUDWeapons			m_Weapons;

	// New Tron items
	CHUDPermissions		m_Permissions;
	CHUDVersion			m_Version;
	CHUDHealthEnergy	m_HealthEnergy;
	CHUDArmor			m_Armor;
	CHUDEnergyTransfer	m_EnergyTrans;
	CHUDProgress		m_Progress;
	CHUDWpnChooser		m_WpnChooser;
	CHUDAmmoChooser		m_AmmoChooser;
	CHUDWeapon			m_Weapon;
};
extern CHUDCrosshair*		g_pCrosshair;

#endif // __TRONHUDMGR_H
