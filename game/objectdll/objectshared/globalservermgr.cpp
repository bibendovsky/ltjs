// ----------------------------------------------------------------------- //
//
// MODULE  : GlobalServerMgr.cpp
//
// PURPOSE : Implementations of server global definitions
//
// CREATED : 7/07/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "globalservermgr.h"
#include "serversoundmgr.h"
#include "aibutemgr.h"
#include "aigoalbutemgr.h"
#include "attachbutemgr.h"
#include "animationmgr.h"
#include "proptypemgr.h"
#include "intelmgr.h"
#include "keymgr.h"
#include "searchitemmgr.h"
#include "gadgettargetmgr.h"
#include "commandbutemgr.h"
#include "relationmgr.h"
#include "inventorybutemgr.h"
#include "transitionmgr.h"
#include "radartypemgr.h"
#include "activatetypemgr.h"
#include "triggertypemgr.h"

// These includes necessary to establish correct linkage.  They rely on the header
// using a LINKTO_MODULE macro, which places a global variable into the cpp file.  This
// variable then references a global variable defined in the cpp associated with the
// header.  The other cpp must have a LINKFROM_MODULE macro in it to define the referenced
// global variable.  This is needed because the BEGIN_CLASS/END_CLASS engine macros
// create global variables that rely on the constructors to get called.  If the global
// variable is inside a static lib and isn't referenced outside the module, the module's
// obj want be part of the link with the dll.  Each new engine based object that must
// use the LINKTO_MODULE/LINKFROM_MODULE pair and put the header in this file.
#include "ai.h"
#include "aihuman.h"
#include "ainode.h"
#include "ainodeguard.h"
#include "ainodesensing.h"
#include "ainodedisturbance.h"
#include "airegion.h"
#include "aivolume.h"
#include "activeworldmodel.h"
#include "alarm.h"
#include "ammobox.h"
#include "body.h"
#include "bombable.h"
#include "breakable.h"
#include "camera.h"
#include "character.h"
#include "characterhitbox.h"
#include "clientlightfx.h"
#include "clientsfx.h"
#include "commandobject.h"
#include "controller.h"
#include "debuglinesystem.h"
#include "decisionobject.h"
#include "dialogue.h"
#include "displaymeter.h"
#include "displaytimer.h"
#include "doomsdaydevice.h"
#include "doomsdaypiece.h"
#include "door.h"
#include "doorknob.h"
#include "dynamicoccludervolume.h"
#include "eventcounter.h"
#include "exittrigger.h"
#include "explosion.h"
#include "fire.h"
#include "gadgettarget.h"
#include "gamebase.h"
#include "gamestartpoint.h"
#include "gearitems.h"
#include "group.h"
#include "hhweaponmodel.h"
#include "intelligence.h"
#include "jumpvolume.h"
#include "key.h"
#include "keyframer.h"
#include "keyitem.h"
#include "keypad.h"
#include "lasertrigger.h"
#include "lightgroup.h"
#include "lightning.h"
#include "lock.h"
#include "mine.h"
#include "moditem.h"
#include "nodeline.h"
#include "noplayertrigger.h"
#include "objectremover.h"
#include "objectivesprite.h"
#include "particlesystem.h"
#include "pickupitem.h"
#include "playerlure.h"
#include "playerobj.h"
#include "playertrigger.h"
#include "playervehicle.h"
#include "point.h"
#include "polygrid.h"
#include "projectile.h"
#include "projectiletypes.h"
#include "prop.h"
#include "proptype.h"
#include "radarobject.h"
#include "randomspawner.h"
#include "rotatingdoor.h"
#include "rotatingswitch.h"
#include "rotatingworldmodel.h"
#include "scalesprite.h"
#include "scanner.h"
#include "screenshake.h"
#include "searchlight.h"
#include "searchprop.h"
#include "securitycamera.h"
#include "servermark.h"
#include "serversoundfx.h"
#include "serverspecialfx.h"
#include "slidingdoor.h"
#include "slidingswitch.h"
#include "slidingworldmodel.h"
#include "snowvolume.h"
#include "scattervolume.h"
#include "soundbutefx.h"
#include "spawner.h"
#include "speaker.h"
#include "spinningworldmodel.h"
#include "sprinkles.h"
#include "startupcommand.h"
#include "steam.h"
#include "switch.h"
#include "teleportpoint.h"
#include "texturefx.h"
#include "transitionarea.h"
#include "trigger.h"
#include "volumebrush.h"
#include "volumebrushtypes.h"
#include "volumeeffect.h"
#include "weaponitems.h"
#include "worldmodel.h"
#include "worldmodeldebris.h"
#include "worldproperties.h"
#include "powerarmor.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalServerMgr::CGlobalServerMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CGlobalServerMgr::CGlobalServerMgr()
{
	m_pServerSoundMgr = NULL;	
	m_pAIButeMgr = NULL;		
	m_pAIGoalButeMgr = NULL;	
	m_pAttachButeMgr = NULL;	
	m_pAnimationMgrList = NULL;
	m_pPropTypeMgr = NULL;		
	m_pIntelMgr = NULL;		
	m_pKeyMgr = NULL;			
	m_pSearchItemMgr = NULL;			
	m_pGadgetTargetMgr = NULL;
	m_pCommandButeMgr = NULL;	
	m_pServerButeMgr = NULL;
	m_pInventoryButeMgr = NULL;
	m_pTransitionMgr = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalServerMgr::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

LTBOOL CGlobalServerMgr::Init()
{
	if( !g_pGameServerShell )
		return LTFALSE;

	m_pServerSoundMgr = new CServerSoundMgr;
	m_pServerSoundMgr->Init();

    // Since Client & Server are the same on PS2, we only need to do this once (when called by the client)
    if (!CGlobalMgr::Init()) 
		return LTFALSE;

    m_pAIButeMgr = new CAIButeMgr;
	m_pAIButeMgr->Init();

    m_pAIGoalButeMgr = new CAIGoalButeMgr;
	m_pAIGoalButeMgr->Init();

    m_pAttachButeMgr = new CAttachButeMgr;
	m_pAttachButeMgr->Init();

    m_pServerButeMgr = new CServerButeMgr;
	m_pServerButeMgr->Init();

    m_pAnimationMgrList = new CAnimationMgrList;
	m_pAnimationMgrList->Init();

	m_pPropTypeMgr = new CPropTypeMgr;
	m_pPropTypeMgr->Init();

	m_pIntelMgr = new CIntelMgr;
	m_pIntelMgr->Init();

	m_pKeyMgr = new CKeyMgr;
	m_pKeyMgr->Init();
    	
	m_pSearchItemMgr = new CSearchItemMgr;
	m_pSearchItemMgr->Init();
    	

        CRelationMgr::GetGlobalRelationMgr()->Init();

	m_pGadgetTargetMgr = new CGadgetTargetMgr;
	m_pGadgetTargetMgr->Init();

	m_pCommandButeMgr = new CCommandButeMgr;
	m_pCommandButeMgr->Init();

	m_pInventoryButeMgr = new CInventoryButeMgr;
	m_pInventoryButeMgr->Init();

	m_pTransitionMgr = debug_new( CTransitionMgr );
	if( !m_pTransitionMgr )
		return LTFALSE;

	
	if( g_pGameServerShell->ShouldUseRadar() )
	{
		// Get the singleton instance of the radar type mgr and initialize it...
		if( !g_pRadarTypeMgr )
		{
			CRadarTypeMgr &RadarTypeMgr = CRadarTypeMgr::Instance();
			if( !RadarTypeMgr.Init() )
			{
				ShutdownWithError( "RadarTypeMgr", RTMGR_DEFAULT_FILE );
				return LTFALSE;
			}
		}
	}
	
	if( !g_pActivateTypeMgr )
	{
		CActivateTypeMgr &ActivateTypeMgr = CActivateTypeMgr::Instance();
		ActivateTypeMgr.Init();
	}
	
	if( !g_pTriggerTypeMgr )
	{
		CTriggerTypeMgr & TriggerTypeMgr = CTriggerTypeMgr::Instance();
		TriggerTypeMgr.Init();
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalServerMgr::Term()
//
//	PURPOSE:	Terminator
//
// ----------------------------------------------------------------------- //

void CGlobalServerMgr::Term()
{
	if( m_pServerSoundMgr )
	{
		delete m_pServerSoundMgr;
		m_pServerSoundMgr = NULL;
	}

	if( m_pAIButeMgr )
	{
		delete m_pAIButeMgr;
		m_pAIButeMgr = NULL;
	}

	if( m_pAIGoalButeMgr )
	{
		delete m_pAIGoalButeMgr;
		m_pAIGoalButeMgr = NULL;
	}

	if( m_pAttachButeMgr )
	{
		delete m_pAttachButeMgr;
		m_pAttachButeMgr = NULL;
	}

	if( m_pServerButeMgr )
	{
		delete m_pServerButeMgr;
		m_pServerButeMgr = NULL;
	}

	if( m_pAnimationMgrList )
	{
		delete m_pAnimationMgrList;
		m_pAnimationMgrList = NULL;
	}

	if( m_pPropTypeMgr )
	{
		delete m_pPropTypeMgr;
		m_pPropTypeMgr = NULL;
	}

	if( m_pIntelMgr )
	{
		delete m_pIntelMgr;
		m_pIntelMgr = NULL;
	}

	if( m_pKeyMgr )
	{
		delete m_pKeyMgr;
		m_pKeyMgr = NULL;
	}

	if( m_pSearchItemMgr )
	{
		delete m_pSearchItemMgr;
		m_pSearchItemMgr = NULL;
	}

	if( m_pGadgetTargetMgr )
	{
		delete m_pGadgetTargetMgr;
		m_pGadgetTargetMgr = NULL;
	}

	CRelationMgr::GetGlobalRelationMgr()->Term();

	if( m_pCommandButeMgr )
	{
		delete m_pCommandButeMgr;
		m_pCommandButeMgr = NULL;
	}

	if( m_pInventoryButeMgr )
	{
		delete m_pInventoryButeMgr;
		m_pInventoryButeMgr = NULL;
	}

	if( m_pTransitionMgr )
	{
		debug_delete( m_pTransitionMgr );
		m_pTransitionMgr = NULL;
	}

	if( g_pRadarTypeMgr )
	{
		g_pRadarTypeMgr->Term();
	}

	if( g_pActivateTypeMgr )
	{
		g_pActivateTypeMgr->Term();
	}

	if( g_pTriggerTypeMgr )
	{
		g_pTriggerTypeMgr->Term();
	}

	CGlobalMgr::Term( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalServerMgr::ShutdownWithError()
//
//	PURPOSE:	Shutdown all clients with an error
//
// ----------------------------------------------------------------------- //

void CGlobalServerMgr::ShutdownWithError(const char* pMgrName, const char* pButeFilePath)
{
	char errorBuf[256];
	sprintf(errorBuf, "ERROR in CGlobalServerMgr::Init()\n\nCouldn't initialize %s.  Make sure the %s file is valid!", pMgrName, pButeFilePath);
    g_pLTServer->CPrint(errorBuf);

	// TO DO:
	// Send a message to all clients to shut down (also send error string!!!)
	//
}
