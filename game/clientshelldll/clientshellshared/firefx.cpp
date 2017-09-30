// ----------------------------------------------------------------------- //
//
// MODULE  : FireFX.cpp
//
// PURPOSE : FireFX special FX - Implementation
//
// CREATED : 5/06/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "firefx.h"
#include "randomsparksfx.h"
#include "gameclientshell.h"
#include "clientbutemgr.h"

#define FFX_DEFAULT_RADIUS					100.0f
#define	FFX_MIN_RADIUS						20.0f
#define	FFX_MAX_RADIUS						500.0f
#define	FFX_INNER_CAM_RADIUS				300.0f
#define	FFX_CAM_FALLOFF_RANGE				300.0f
#define FFX_DEFAULT_SMOKE_PARTICLE_RADIUS	7000.0f
#define FFX_DEFAULT_FIRE_PARTICLE_RADIUS	4000.0f
#define FFX_MAX_SMOKE_PARTICLE_RADIUS		(FFX_DEFAULT_SMOKE_PARTICLE_RADIUS * 1.3f)
#define FFX_MAX_FIRE_PARTICLE_RADIUS		(FFX_DEFAULT_FIRE_PARTICLE_RADIUS)
#define FFX_MIN_FIRE_PARTICLE_LIFETIME		0.25f
#define FFX_MAX_FIRE_PARTICLE_LIFETIME		2.0f
#define FFX_MIN_SMOKE_PARTICLE_LIFETIME		0.5f
#define FFX_MAX_SMOKE_PARTICLE_LIFETIME		6.0f
#define FFX_MAX_LIGHT_RADIUS				300.0f


extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFireFX::Init
//
//	PURPOSE:	Init the lightning fx
//
// ----------------------------------------------------------------------- //

LTBOOL CFireFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
    if (!CSpecialFX::Init(hServObj, pMsg)) return LTFALSE;
    if (!pMsg) return LTFALSE;

	// Read in the init info from the message...

	FIRECREATESTRUCT fire;
	fire.hServerObj		= hServObj;
    fire.fRadius        = pMsg->Readfloat();
    fire.fSoundRadius   = pMsg->Readfloat();
    fire.fLightRadius   = pMsg->Readfloat();
    fire.fLightPhase    = pMsg->Readfloat();
    fire.fLightFreq     = pMsg->Readfloat();
    fire.vLightOffset	= pMsg->ReadLTVector();
    fire.vLightColor	= pMsg->ReadLTVector();
    fire.bCreateSmoke   = (LTBOOL)pMsg->Readuint8();
    fire.bCreateLight   = (LTBOOL)pMsg->Readuint8();
    fire.bCreateSparks  = (LTBOOL)pMsg->Readuint8();
    fire.bCreateSound   = (LTBOOL)pMsg->Readuint8();
    fire.bBlackSmoke    = (LTBOOL)pMsg->Readuint8();
    fire.bSmokeOnly     = (LTBOOL)pMsg->Readuint8();

	return Init(&fire);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFireFX::Init
//
//	PURPOSE:	Init the Fire fx
//
// ----------------------------------------------------------------------- //

LTBOOL CFireFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	m_cs = *((FIRECREATESTRUCT*)psfxCreateStruct);
	m_cs.fRadius = m_cs.fRadius < FFX_MIN_RADIUS ? FFX_MIN_RADIUS :
		(m_cs.fRadius > FFX_MAX_RADIUS ? FFX_MAX_RADIUS : m_cs.fRadius);

	m_fSizeAdjust = m_cs.fRadius / FFX_DEFAULT_RADIUS;

	// If we're creating smoke, don't do light, sound or sparks...

	if (m_cs.bSmokeOnly)
	{
        m_cs.bCreateLight = m_cs.bCreateSound = m_cs.bCreateSparks = LTFALSE;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFireFX::CreateObject
//
//	PURPOSE:	Create the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CFireFX::CreateObject(ILTClient* pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE) || !g_pGameClientShell) return LTFALSE;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
    if (!psfxMgr) return LTFALSE;

    LTVector vOffset(0, 0, 0);
	char szStr[128];

	// Get our initial pos...

    if ((m_cs.vPos.MagSqr() < 0.01f) && m_hServerObject)
    {
		g_pLTClient->GetObjectPos(m_hServerObject, &(m_cs.vPos));
	}


	SMCREATESTRUCT sm;
	sm.hServerObj		= m_hServerObject;
    sm.bRelToCameraPos  = LTTRUE;
	sm.fInnerCamRadius	= FFX_INNER_CAM_RADIUS;
	sm.fOuterCamRadius	= FFX_INNER_CAM_RADIUS + (FFX_CAM_FALLOFF_RANGE * m_fSizeAdjust);

	// Create the smoke particles...

	if (m_cs.bCreateSmoke)
	{
		vOffset.Init(0, 20.0f * m_fSizeAdjust, 0);
		vOffset.y = vOffset.y > 50.0f ? 50.0f : vOffset.y;

		sm.vColor1.Init(100.0f, 100.0f, 100.0f);
		sm.vColor2.Init(150.0f, 150.0f, 150.0f);
		sm.vMinDriftVel.Init(-2.0f, 15.0f, -2.0f);
		sm.vMaxDriftVel.Init(2.0f, 50.0f, 2.0f);
		sm.fVolumeRadius		= 5.0f * (1.5f * m_fSizeAdjust);
		sm.fLifeTime			= 100000.0f;
		sm.fRadius				= FFX_DEFAULT_SMOKE_PARTICLE_RADIUS * m_fSizeAdjust;
		sm.fParticleCreateDelta	= 0.25f;
		sm.fMinParticleLife		= 2.0f * m_fSizeAdjust;
		sm.fMaxParticleLife		= 4.0f * m_fSizeAdjust;
		sm.nNumParticles		= 2;
		//sm.bMultiply			= m_cs.bBlackSmoke;
        sm.bIgnoreWind          = LTFALSE;
		sm.vPos					= m_cs.vPos + vOffset;

        sm.bAdjustParticleScale = LTTRUE;
		sm.fStartParticleScale	= 1.0f;
		sm.fEndParticleScale	= 0.5f;
        sm.bAdjustParticleAlpha = LTTRUE;
		sm.fStartParticleAlpha	= 1.0f;
		sm.fEndParticleAlpha	= 0.0f;

		sm.fMinParticleLife	= sm.fMinParticleLife < FFX_MIN_SMOKE_PARTICLE_LIFETIME ? FFX_MIN_SMOKE_PARTICLE_LIFETIME :
			(sm.fMinParticleLife > FFX_MAX_SMOKE_PARTICLE_LIFETIME ? FFX_MAX_SMOKE_PARTICLE_LIFETIME : sm.fMinParticleLife);
		sm.fMaxParticleLife	= sm.fMaxParticleLife > FFX_MAX_SMOKE_PARTICLE_LIFETIME ? FFX_MAX_SMOKE_PARTICLE_LIFETIME :
			(sm.fMaxParticleLife < FFX_MIN_SMOKE_PARTICLE_LIFETIME ? FFX_MIN_SMOKE_PARTICLE_LIFETIME : sm.fMaxParticleLife);
		sm.fRadius = sm.fRadius > FFX_MAX_SMOKE_PARTICLE_RADIUS ? FFX_MAX_SMOKE_PARTICLE_RADIUS : sm.fRadius;

		g_pClientButeMgr->GetSpecialFXAttributeString("FireSmokeTex",szStr,sizeof(szStr));
        if (!strlen(szStr)) return LTFALSE;

        sm.hstrTexture = g_pLTClient->CreateString(szStr);

		if (!m_Smoke1.Init(&sm) || !m_Smoke1.CreateObject(m_pClientDE))
		{
            return LTFALSE;
		}
        g_pLTClient->FreeString(sm.hstrTexture);

		m_Smoke1.Update();
	}

	// Create the fire particles...

	if (!m_cs.bSmokeOnly)
	{
        LTFLOAT fVolumeAdjust = m_fSizeAdjust < 1.0 ? m_fSizeAdjust / 1.5f : m_fSizeAdjust * 1.5f;

		sm.vColor1.Init(100.0f, 100.0f, 100.0f);
		sm.vColor2.Init(150.0f, 150.0f, 150.0f);
		sm.vMinDriftVel.Init(-2.0f, 8.0f, -2.0f);
		sm.vMaxDriftVel.Init(2.0f, 15.0f, 2.0f);
		sm.fVolumeRadius		= 10.0f * fVolumeAdjust;
		sm.fLifeTime			= 100000.0f;
		sm.fRadius				= FFX_DEFAULT_FIRE_PARTICLE_RADIUS * m_fSizeAdjust;
		sm.fParticleCreateDelta	= 0.1f;
		sm.fMinParticleLife		= 1.0f * m_fSizeAdjust;
		sm.fMaxParticleLife		= 2.0f * m_fSizeAdjust;
		sm.nNumParticles		= 3;
        sm.bAdditive            = LTTRUE;
		sm.vPos					= m_cs.vPos;

		//sm.fStartParticleScale	= 1.0f;
		//sm.fEndParticleScale	= 0.0f;

		sm.fMinParticleLife	= sm.fMinParticleLife < FFX_MIN_FIRE_PARTICLE_LIFETIME ? FFX_MIN_FIRE_PARTICLE_LIFETIME :
			(sm.fMinParticleLife > FFX_MAX_FIRE_PARTICLE_LIFETIME ? FFX_MAX_FIRE_PARTICLE_LIFETIME : sm.fMinParticleLife);
		sm.fMaxParticleLife	= sm.fMaxParticleLife > FFX_MAX_FIRE_PARTICLE_LIFETIME ? FFX_MAX_FIRE_PARTICLE_LIFETIME :
			(sm.fMaxParticleLife < FFX_MIN_FIRE_PARTICLE_LIFETIME ? FFX_MIN_FIRE_PARTICLE_LIFETIME : sm.fMaxParticleLife);
		sm.fRadius = sm.fRadius > FFX_MAX_FIRE_PARTICLE_RADIUS ? FFX_MAX_FIRE_PARTICLE_RADIUS : sm.fRadius;

		g_pClientButeMgr->GetSpecialFXAttributeString("FireTex",szStr,sizeof(szStr));
        if (!strlen(szStr)) return LTFALSE;

        sm.hstrTexture = g_pLTClient->CreateString(szStr);

		if (!m_Fire1.Init(&sm) || !m_Fire1.CreateObject(m_pClientDE))
		{
            return LTFALSE;
		}

		m_Fire1.Update();
        g_pLTClient->FreeString(sm.hstrTexture);


		// Create inner fire particles...

		sm.vColor1.Init(100.0f, 100.0f, 100.0f);
		sm.vColor2.Init(150.0f, 150.0f, 150.0f);
		sm.vMinDriftVel.Init(-2.0f, 25.0f, -2.0f);
		sm.vMaxDriftVel.Init(2.0f, 35.0f, 2.0f);
		sm.fRadius			= FFX_DEFAULT_FIRE_PARTICLE_RADIUS * 0.75f * m_fSizeAdjust;
		sm.nNumParticles	= 5;
		sm.fVolumeRadius	= 5.0f * fVolumeAdjust;
		sm.fLifeTime		= 100000.0f;
		sm.fMinParticleLife	= 0.5f * m_fSizeAdjust;
		sm.fMaxParticleLife	= 1.25f * m_fSizeAdjust;

		//sm.fStartParticleScale	= 1.0f;
		//sm.fEndParticleScale	= 0.5f;

		sm.fMinParticleLife	= sm.fMinParticleLife < FFX_MIN_FIRE_PARTICLE_LIFETIME ? FFX_MIN_FIRE_PARTICLE_LIFETIME :
			(sm.fMinParticleLife > FFX_MAX_FIRE_PARTICLE_LIFETIME ? FFX_MAX_FIRE_PARTICLE_LIFETIME : sm.fMinParticleLife);
		sm.fMaxParticleLife	= sm.fMaxParticleLife > FFX_MAX_FIRE_PARTICLE_LIFETIME ? FFX_MAX_FIRE_PARTICLE_LIFETIME :
			(sm.fMaxParticleLife < FFX_MIN_FIRE_PARTICLE_LIFETIME ? FFX_MIN_FIRE_PARTICLE_LIFETIME : sm.fMaxParticleLife);
		sm.fRadius = sm.fRadius > FFX_MAX_FIRE_PARTICLE_RADIUS ? FFX_MAX_FIRE_PARTICLE_RADIUS : sm.fRadius;

   		g_pClientButeMgr->GetSpecialFXAttributeString("FireTex2",szStr,sizeof(szStr));
        if (!strlen(szStr)) return LTFALSE;

        sm.hstrTexture = g_pLTClient->CreateString(szStr);

		if (!m_Fire2.Init(&sm) || !m_Fire2.CreateObject(m_pClientDE))
		{
            return LTFALSE;
		}

		m_Fire2.Update();
        g_pLTClient->FreeString(sm.hstrTexture);


		// Create the sound...

		if (m_cs.bCreateSound)
		{
			g_pClientButeMgr->GetSpecialFXAttributeString("FireSnd",szStr,sizeof(szStr));

			m_hSound = g_pClientSoundMgr->PlaySoundFromPos(m_cs.vPos, szStr,
				m_cs.fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM, PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP);
		}


		// Create the dynamic light...

		if (m_cs.bCreateLight)
		{
			LIGHTCREATESTRUCT light;

            LTFLOAT fRadiusMin = m_cs.fLightRadius;
			fRadiusMin = fRadiusMin < 20.0f ? 20.0f : (fRadiusMin > FFX_MAX_LIGHT_RADIUS ? FFX_MAX_LIGHT_RADIUS : fRadiusMin);

			light.vColor				= m_cs.vLightColor;
			light.hServerObj			= m_hServerObject;
			light.vOffset				= m_cs.vLightOffset;
			light.dwLightFlags			= FLAG_DONTLIGHTBACKFACING;
			light.fIntensityMin			= 1.0f;
			light.fIntensityMax			= 1.0f;
			light.nIntensityWaveform	= WAVE_NONE;
			light.fIntensityFreq		= 1.0f;
			light.fIntensityPhase		= 0.0f;
			light.fRadiusMin			= fRadiusMin;
			light.fRadiusMax			= fRadiusMin * 1.1f;
			light.nRadiusWaveform		= WAVE_FLICKER2;
			light.fRadiusFreq			= m_cs.fLightFreq;
			light.fRadiusPhase			= m_cs.fLightPhase;

			if (!m_Light.Init(&light) || !m_Light.CreateObject(m_pClientDE))
			{
                return LTFALSE;
			}
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFireFX::Update
//
//	PURPOSE:	Update the Fire Fx
//
// ----------------------------------------------------------------------- //

LTBOOL CFireFX::Update()
{
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
    if (!psfxMgr || !m_pClientDE || !m_hServerObject) return LTFALSE;

    LTFLOAT fTime = m_pClientDE->GetTime();
    static_cast<void>(fTime);

	// Check to see if we should go away...

	if (m_bWantRemove)
	{
        return LTFALSE;
	}


	// Update FX...

	if (m_cs.bCreateSmoke)
	{
		m_Smoke1.Update();
	}

	if (m_cs.bCreateLight)
	{
		m_Light.Update();
	}

	m_Fire1.Update();
	m_Fire2.Update();


	// Hide/show the fire if necessary...

	if (m_hServerObject)
	{
        uint32 dwUserFlags;
		g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_User, dwUserFlags);

		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
			if (m_hSound)
			{
                g_pLTClient->SoundMgr()->KillSound(m_hSound);
                m_hSound = LTNULL;
			}

            return LTTRUE;
		}
		else
		{
			if (m_cs.bCreateSound && !m_hSound)
			{
				char szStr[128] = "";
				g_pClientButeMgr->GetSpecialFXAttributeString("FireSnd",szStr,sizeof(szStr));

				m_hSound = g_pClientSoundMgr->PlaySoundFromPos(m_cs.vPos, szStr,
					m_cs.fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM, PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP);
			}
		}
	}

	// Create the random spark particles...

	if (m_cs.bCreateSparks && GetRandom(1, 10) == 1)
	{
		CSFXMgr* psfxMgr2 = g_pGameClientShell->GetSFXMgr();
        if (!psfxMgr2) return LTFALSE;

		RANDOMSPARKSCREATESTRUCT sparks;
		sparks.hServerObj = m_hServerObject;

        LTFLOAT fVel = m_fSizeAdjust * GetRandom(50.0f, 70.0f);
		fVel = (fVel < 30.0f ? 30.0f : (fVel > 100.0f ? 100.0f : fVel));

        LTVector vDir(0.0, 1.0, 0.0);
		sparks.vMinVelAdjust.Init(1, 3, 1);
		sparks.vMaxVelAdjust.Init(1, 6, 1);
		sparks.vDir	= vDir * fVel;
		sparks.nSparks = static_cast<uint8>(GetRandom(1, 5));
		sparks.fDuration = m_fSizeAdjust * GetRandom(1.0f, 2.0f);
        sparks.bRelToCameraPos = LTTRUE;
		sparks.fInnerCamRadius = FFX_INNER_CAM_RADIUS;
		sparks.fOuterCamRadius = FFX_INNER_CAM_RADIUS + (FFX_CAM_FALLOFF_RANGE * m_fSizeAdjust);
		sparks.fRadius = 300.0f * m_fSizeAdjust;
		sparks.fRadius = sparks.fRadius < 100.0f ? 100.0f : (sparks.fRadius > 500.0f ? 500.0f : sparks.fRadius);

		psfxMgr2->CreateSFX(SFX_RANDOMSPARKS_ID, &sparks);
	}

    return LTTRUE;
}