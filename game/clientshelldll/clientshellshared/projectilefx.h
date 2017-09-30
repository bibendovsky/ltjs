// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileFX.h
//
// PURPOSE : Projectile special fx class - Definition
//
// CREATED : 7/6/98
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILE_FX_H__
#define __PROJECTILE_FX_H__

#include "specialfx.h"
#include "weaponmgr.h"
#include "polylinefx.h"
#include "particletrailfx.h"

struct PROJECTILECREATESTRUCT : public SFXCREATESTRUCT
{
    PROJECTILECREATESTRUCT();

    uint8   nWeaponId;
    uint8   nAmmoId;
    uint8   nShooterId;
    LTBOOL   bLocal;
    LTBOOL   bAltFire;
};

inline PROJECTILECREATESTRUCT::PROJECTILECREATESTRUCT()
{
	nShooterId	= static_cast<uint8>(-1);
	nWeaponId	= WMGR_INVALID_ID;
	nAmmoId		= WMGR_INVALID_ID;
    bLocal      = LTFALSE;
    bAltFire    = LTFALSE;
}


class CProjectileFX : public CSpecialFX
{
	public :

		CProjectileFX() : CSpecialFX()
		{
			m_nWeaponId		= WMGR_INVALID_ID;
			m_nAmmoId		= WMGR_INVALID_ID;
			m_nFX			= 0;

            m_pSmokeTrail   = LTNULL;
            m_hFlare        = LTNULL;
            m_hLight        = LTNULL;
            m_hFlyingSound  = LTNULL;

			m_nShooterId	= static_cast<uint8>(-1);
            m_bLocal        = LTFALSE;

			m_fStartTime	= 0.0f;
            m_bDetonated    = LTFALSE;
            m_bAltFire      = LTFALSE;

            m_pProjectileFX = LTNULL;
		}

		~CProjectileFX()
		{
			RemoveFX();
		}

        virtual LTBOOL Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();

		void HandleTouch(CollisionInfo *pInfo);

		virtual uint32 GetSFXID() { return SFX_PROJECTILE_ID; }

	protected :

        void CreateFlare(const LTVector & vPos, const LTRotation & rRot);
        void CreateLight(const LTVector & vPos, const LTRotation & rRot);
        void CreateProjectile(const LTVector & vPos, const LTRotation & rRot);
        void CreateFlyingSound(const LTVector & vPos, const LTRotation & rRot);
        void CreateSmokeTrail(const LTVector & vPos, const LTRotation & rRot);

		void RemoveFX();

        LTBOOL MoveServerObj();
		void  Detonate(CollisionInfo* pInfo);

		static CBankedList<CParticleTrailFX> *GetParticleTrailBank();

		CSpecialFX*		m_pSmokeTrail;		// Smoke trail fx
		PROJECTILEFX*	m_pProjectileFX;
		HOBJECT			m_hFlare;			// Flare fx
		HOBJECT			m_hLight;			// Light fx
        HLTSOUND        m_hFlyingSound;     // Sound of the projectile

        uint8           m_nWeaponId;        // Id of weapon fired
        uint8           m_nAmmoId;          // Type of ammo fired
        uint8           m_nFX;              // FX associated with projectile
        uint8           m_nShooterId;       // Client Id of shooter
        LTBOOL           m_bLocal;           // Did local client create this fx
        LTBOOL           m_bAltFire;         // Alt-fire?
        LTBOOL           m_bDetonated;

        LTFLOAT          m_fStartTime;

        LTVector         m_vFirePos;
        LTVector         m_vPath;

};

#endif // __PROJECTILE_FX_H__