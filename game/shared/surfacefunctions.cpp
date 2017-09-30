// ----------------------------------------------------------------------- //
//
// MODULE  : SurfaceFunctions.cpp
//
// PURPOSE : Implementation of shared surface functions
//
// CREATED : 7/13/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "containercodes.h"
#include "surfacefunctions.h"
#include "clientservershared.h"
#include "commonutilities.h"
#include "weaponmgr.h"
#include "fxbutemgr.h"

extern char s_FileBuffer[MAX_CS_FILENAME_LEN];

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShowsMark
//
//	PURPOSE:	Does this type of surface show marks
//
// ----------------------------------------------------------------------- //

LTBOOL ShowsMark(SurfaceType eSurfType)
{
	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
	if (pSurf)
	{
		return pSurf->bShowsMark;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShowsTracks
//
//	PURPOSE:	Does this type of surface show tracks
//
// ----------------------------------------------------------------------- //

LTBOOL ShowsTracks(SurfaceType eSurfType)
{
	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
	if (pSurf)
	{
		return (pSurf->szLtFootPrintSpr[0] && pSurf->szRtFootPrintSpr[0]);
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetImpactSprite()
//
//	PURPOSE:	Get impact sprite with this surface
//
// ----------------------------------------------------------------------- //

LTBOOL GetImpactSprite(SurfaceType eSurfType, LTFLOAT & fScale, int nAmmoId,
					  char* pBuf, int nBufLen)
{
	if (!g_pWeaponMgr || !ShowsMark(eSurfType) || !pBuf)
	{
        return LTFALSE;
	}

	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);
    if (!pAmmo || !pAmmo->pImpactFX) return LTFALSE;

	// Get the impact mark filename...

	strncpy(pBuf, pAmmo->pImpactFX->szMark, nBufLen);
    if (!pBuf[0]) return LTFALSE;

	if (stricmp(pBuf, "SURFACE") == 0)
	{
		// Use the surface to dtermine the bullet hole...

		SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
		if (pSurf && pSurf->szBulletHoleSpr[0])
		{
			strcpy(pBuf, pSurf->szBulletHoleSpr);
			fScale = GetRandom(pSurf->fBulletHoleMinScale,
							   pSurf->fBulletHoleMaxScale);
		}
		else
		{
            return LTFALSE;
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UserFlagToSurface()
//
//	PURPOSE:	Convert a user flag to a surface type
//
// ----------------------------------------------------------------------- //

SurfaceType UserFlagToSurface(uint32 dwUserFlag)
{
	// Top byte contains surface flag

	SurfaceType eSurfType = (SurfaceType) (dwUserFlag >> 24);

	return eSurfType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SurfaceToUserFlag()
//
//	PURPOSE:	Convert surface type to a user flag
//
// ----------------------------------------------------------------------- //

uint32 SurfaceToUserFlag(SurfaceType eSurfType)
{
	// Top byte should contain surface flag

    uint32 dwUserFlag = (uint32)eSurfType;
	dwUserFlag = (dwUserFlag << 24);

	return dwUserFlag;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetMovementSound()
//
//	PURPOSE:	Get movement sounds associated with this surface
//
// ----------------------------------------------------------------------- //

char* GetMovementSound(SurfaceType eSurfType, LTBOOL bLeftFoot,
					   PlayerPhysicsModel eModel)
{
	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
#ifndef __PSX2
    _ASSERT(pSurf);
#endif
    if (!pSurf) return LTNULL;

	switch (eModel)
	{
		case PPM_SNOWMOBILE :
		{
			int nIndex = GetRandom(0, SRF_MAX_SNOWMOBILE_SNDS-1);
			return (pSurf->szSnowmobileSnds[nIndex]);
		}
		break;

		case PPM_NORMAL :
		default :
		{
			// Footstep sounds...

			int nIndex = GetRandom(0, SRF_MAX_FOOTSTEP_SNDS-1);
			return (bLeftFoot ? pSurf->szLtFootStepSnds[nIndex] :
				pSurf->szRtFootStepSnds[nIndex]);
		}
		break;
	}

#if 0
	return LTNULL;
#endif // 0
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetImpactSound()
//
//	PURPOSE:	Get impact sounds associated with this surface
//
// ----------------------------------------------------------------------- //

char* GetImpactSound(SurfaceType eSurfType, int nAmmoId)
{
	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);
	_ASSERT(pAmmo);
    if (!pAmmo || !pAmmo->pImpactFX) return LTNULL;

    if (!(pAmmo->pImpactFX->szSound[0])) return LTNULL;
	SAFE_STRCPY(s_FileBuffer, pAmmo->pImpactFX->szSound);

	if (_stricmp(s_FileBuffer, "SURFACE") == 0)
	{
		SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
		_ASSERT(pSurf);
        if (!pSurf) return LTNULL;

		if (pAmmo->eType == VECTOR)
		{
			if( (pAmmo->eInstDamageType == DT_MELEE) || (pAmmo->eInstDamageType == DT_SWORD) )
			{
			return pSurf->szMeleeImpactSnds[GetRandom(0, SRF_MAX_IMPACT_SNDS-1)];
			}
			else
			{
				return pSurf->szBulletImpactSnds[GetRandom(0, SRF_MAX_IMPACT_SNDS-1)];
			}
		}
		else if (pAmmo->eType == PROJECTILE)
		{
			return pSurf->szProjectileImpactSnds[GetRandom(0, SRF_MAX_IMPACT_SNDS-1)];
		}
	}

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsMoveable()
//
//	PURPOSE:	Determine if the passed in object is moveable
//
// ----------------------------------------------------------------------- //

LTBOOL IsMoveable(HOBJECT hObj)
{
    if (!hObj) return LTFALSE;

    uint32 dwUserFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_User, dwUserFlags);

	return !!(dwUserFlags & USRFLG_MOVEABLE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CanMarkObject()
//
//	PURPOSE:	Determine if the passed in object can be marked
//
// ----------------------------------------------------------------------- //

LTBOOL CanMarkObject(HOBJECT hObj)
{
    if (!hObj) return LTFALSE;

	// Objects that don't move can be marked...

    if (!IsMoveable(hObj)) return LTTRUE;

#ifndef _CLIENTBUILD

	// Can also mark world models on the server...

    if (GetObjectType(hObj) == OT_WORLDMODEL)
	{
        return LTTRUE;
	}

#endif

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with the info
//				returned from an intersection...
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(IntersectInfo & iInfo)
{
	SurfaceType eType = ST_UNKNOWN;

	// Get the flags from the object, if none is specified, fall through
	//and try and get it from the polygon
	if (iInfo.m_hObject)
	{
		eType = GetSurfaceType(iInfo.m_hObject);
	}

	if (iInfo.m_hPoly != INVALID_HPOLY && (eType == ST_UNKNOWN))
	{
		eType = GetSurfaceType(iInfo.m_hPoly);
	}

	return eType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with the info
//				returned from a collision...
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(CollisionInfo & iInfo)
{
	SurfaceType eType = ST_UNKNOWN;

	// Next get the flags from the poly...This will not work
	// for the first poly in the world (i.e., poly = 0, however
	// there are many more cases where this is the right thing
	// to do)

	if (iInfo.m_hPoly != INVALID_HPOLY)
	{
		eType = GetSurfaceType(iInfo.m_hPoly);
	}

	// Try the object...

	if (eType == ST_UNKNOWN && iInfo.m_hObject)
	{
		eType = GetSurfaceType(iInfo.m_hObject);
	}

	return eType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with a poly
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(HPOLY hPoly)
{
	SurfaceType eType = ST_UNKNOWN;

	if (hPoly != INVALID_HPOLY)
	{
		// Get the flags associated with the poly...

        uint32 dwTextureFlags;

        g_pCommonLT->GetPolyTextureFlags(hPoly, &dwTextureFlags);

		eType = (SurfaceType)dwTextureFlags;
	}

	return eType;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with an object
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(HOBJECT hObj)
{
	if (!hObj) return ST_UNKNOWN;

	SurfaceType eType = ST_UNKNOWN;

    if (!IsMainWorld(hObj))
	{
		uint32 dwUserFlags;
		g_pCommonLT->GetObjectFlags(hObj, OFT_User, dwUserFlags);

		eType = UserFlagToSurface(dwUserFlags);
	}

	return eType;
}