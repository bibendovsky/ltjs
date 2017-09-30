// ----------------------------------------------------------------------- //
//
// MODULE  : SmokeFX.cpp
//
// PURPOSE : Smoke special FX - Implementation
//
// CREATED : 3/2/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "smokefx.h"
#include "iltclient.h"
#include "clientutilities.h"
#include "clientservershared.h"

extern LTVector g_vWorldWindVel;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokeFX::Init
//
//	PURPOSE:	Init the smoke trail
//
// ----------------------------------------------------------------------- //

LTBOOL CSmokeFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return LTFALSE;

	m_pTextureName	= "SFX\\Particle\\Smoke1.dtx";

	SMCREATESTRUCT* pSM = (SMCREATESTRUCT*)psfxCreateStruct;

	m_dwFlags = pSM->dwSystemFlags;

	m_vPos					= pSM->vPos;
	m_vColor1				= pSM->vColor1;
	m_vColor2				= pSM->vColor1;
	m_vMinDriftVel			= pSM->vMinDriftVel;
	m_vMaxDriftVel			= pSM->vMaxDriftVel;
	m_fVolumeRadius			= pSM->fVolumeRadius;
	m_fLifeTime				= pSM->fLifeTime;
	m_fRadius				= pSM->fRadius;
	m_fParticleCreateDelta	= pSM->fParticleCreateDelta;
	m_fMinParticleLife		= pSM->fMinParticleLife;
	m_fMaxParticleLife		= pSM->fMaxParticleLife;
	m_nNumParticles			= pSM->nNumParticles;
	m_bIgnoreWind			= pSM->bIgnoreWind;
	m_hstrTexture			= pSM->hstrTexture;

	//reset our elapsed time, and our emission time
	m_fElapsedTime			= 0.0f;
	m_fElapsedEmissionTime	= m_fParticleCreateDelta;

	m_fGravity		= 0.0f;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokeFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

LTBOOL CSmokeFX::CreateObject(ILTClient *pClientDE)
{
    if (!pClientDE ) return LTFALSE;

	if (m_hstrTexture)
	{
		m_pTextureName = pClientDE->GetStringData(m_hstrTexture);
	}

    LTBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokeFX::Update
//
//	PURPOSE:	Update the smoke
//
// ----------------------------------------------------------------------- //

LTBOOL CSmokeFX::Update()
{
    if (!m_hObject || !m_pClientDE ) return LTFALSE;

	if( g_pGameClientShell->IsServerPaused() )
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_PAUSED, FLAG_PAUSED);
		return LTTRUE;
	}
	
	//make sure we aren't paused
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_PAUSED);

    LTFLOAT fFrameTime = m_pClientDE->GetFrameTime();

	m_fElapsedTime += fFrameTime;
	m_fElapsedEmissionTime += fFrameTime;

	// Hide/show the particle system if necessary...

	if (m_hServerObject)
	{
        uint32 dwUserFlags;
		g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_User, dwUserFlags);

		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
			uint32 dwFlags;
			g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);

			// Once last puff as disappeared, hide the system (no new puffs
			// will be added...)

			if (dwFlags & FLAG_VISIBLE)
			{
				if (m_fElapsedEmissionTime > m_fMaxParticleLife)
				{
					g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_VISIBLE);
				}
			}
			else
			{
				m_fElapsedEmissionTime = 0.0f;
			}

            return LTTRUE;
		}
		else
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
		}
	}



	// Check to see if we should just wait for last smoke puff to go away...

	if (m_fElapsedTime > m_fLifeTime)
	{
		if (m_fElapsedEmissionTime > m_fMaxParticleLife)
		{
            return LTFALSE;
		}

        return LTTRUE;
	}


	// See if it is time to add some more smoke...

	if (m_fElapsedEmissionTime >= m_fParticleCreateDelta)
	{
        LTVector vDriftVel, vColor, vPos;

		// What is the range of colors?

        LTFLOAT fRange = m_vColor2.x - m_vColor1.x;
        static_cast<void>(fRange);


		// Determine how many particles to add...

		int nNumParticles = GetNumParticles(m_nNumParticles);

		// Build the individual smoke puffs...

		for (int j=0; j < nNumParticles; j++)
		{
			VEC_SET(vPos,  GetRandom(-m_fVolumeRadius, m_fVolumeRadius),
					-2.0f, GetRandom(-m_fVolumeRadius, m_fVolumeRadius));

			VEC_SET(vDriftVel,
					GetRandom(m_vMinDriftVel.x, m_vMaxDriftVel.x),
					GetRandom(m_vMinDriftVel.y, m_vMaxDriftVel.y),
					GetRandom(m_vMinDriftVel.z, m_vMaxDriftVel.z));

			if (!m_bIgnoreWind)
			{
				VEC_ADD(vDriftVel, vDriftVel, g_vWorldWindVel);
			}

			GetRandomColorInRange(vColor);

            LTFLOAT fLifeTime = GetRandom(m_fMinParticleLife, m_fMaxParticleLife);

			vDriftVel -= (m_vVel * 0.1f);

			m_pClientDE->AddParticle(m_hObject, &vPos, &vDriftVel, &vColor, fLifeTime);
		}

		m_fElapsedEmissionTime = 0.0f;
	}


	return CBaseParticleSystemFX::Update();
}