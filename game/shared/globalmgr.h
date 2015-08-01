// ----------------------------------------------------------------------- //
//
// MODULE  : GlobalMgr.h
//
// PURPOSE : Definition of global definitions
//
// CREATED : 7/07/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GLOBAL_MGR_H__
#define __GLOBAL_MGR_H__

#include "surfacemgr.h"
#include "missionbutemgr.h"
#include "skillsbutemgr.h"

#include "soundmgr.h"
#include "debrismgr.h"
#include "soundfiltermgr.h"
#include "soundbutemgr.h"
#include "damagetypes.h"


class CFXButeMgr;

class CGlobalMgr
{
	protected :

		CGlobalMgr();
		virtual ~CGlobalMgr();

        virtual LTBOOL Init();
		virtual void Term( );

		CFXButeMgr*		m_pFXButeMgr;		// Same as g_pFXButeMgr
		CDebrisMgr		m_DebrisMgr;		// Same as g_pDebrisMgr
		CSoundFilterMgr	m_SoundFilterMgr;	// Same as g_pSoundFilterMgr

		CSurfaceMgr		m_SurfaceMgr;		// Same as g_pSurfaceMgr
		CSkillsButeMgr	m_SkillsButeMgr;	// Same as g_pSkillsButeMgr
		CSoundButeMgr	m_SoundButeMgr;		// Same as g_pSoundButeMgr

		CDTButeMgr		m_DTButeMgr;	

		virtual void	ShutdownWithError(char* pMgrName, char* pButeFilePath) = 0;
};

#endif // __GLOBAL_MGR_H__