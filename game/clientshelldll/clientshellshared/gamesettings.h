// ----------------------------------------------------------------------- //
//
// MODULE  : GameSettings.h
//
// PURPOSE : Handles implementation of various game settings
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAMESETTINGS_H
#define __GAMESETTINGS_H

#include "clientutilities.h"

// Values for 3 position options..

#define RS_LOW						0
#define	RS_MED						1
#define RS_HIGH						2


class CGameClientShell;

// ***********************************************
//
//		Class Definition
//
// ***********************************************

class CGameSettings
{

public:

	CGameSettings();
	~CGameSettings()	{}

    LTBOOL       Init (ILTClient* pClientDE, CGameClientShell* pClientShell);

    LTBOOL       GetBoolVar(const char *pVar);
    void        SetBoolVar(const char *pVar, LTBOOL bVal);
    uint8       GetByteVar(const char *pVar);
    void        SetByteVar(const char *pVar, uint8 nVal);
	float		GetFloatVar(const char *pVar);
	void		SetFloatVar(const char *pVar, float fVal);

	// misc access functions

    LTBOOL       ScreenFlash() {return GetBoolVar("ScreenFlash");}

	// control access functions

    LTBOOL       MouseLook()                 { return GetBoolVar("MouseLook"); }
    LTBOOL       MouseInvertY()              { return GetBoolVar("MouseInvertY");}
    void        SetMouseLook(LTBOOL bVal)    { SetBoolVar("MouseLook",bVal); }
    void        SetMouseInvertY(LTBOOL bVal) { SetBoolVar("MouseInvertY",bVal); }

	float		MouseSensitivity()			{ return GetFloatVar("MouseSensitivity");}
    LTBOOL       UseJoystick()               { return GetBoolVar("UseJoystick"); }
    LTBOOL       Lookspring()                { return GetBoolVar("LookSpring"); }

	// sound access functions

    LTBOOL       MusicEnabled()              { return GetBoolVar("MusicEnable"); }
	float		MusicVolume()				{ return GetFloatVar("MusicVolume");}
    LTBOOL       SoundEnabled()              { return GetBoolVar("SoundEnable"); }
	float		SoundVolume()				{ return GetFloatVar("SoundVolume");}
	float		SoundChannels()				{ return GetFloatVar("SoundChannels");}
    LTBOOL       Sound16Bit()                { return GetBoolVar("Sound16Bit"); }
	float		WeaponsMultiplier()			{ return GetFloatVar( "WeaponsSoundMultiplier" ); }
	float		SpeechMultiplier()			{ return GetFloatVar( "SpeechSoundMultiplier" ); }
	float		DefaultMultiplier()			{ return GetFloatVar( "DefaultSoundMultiplier" ); }


	// low-level detail access functions

	float		ModelLOD()					{ return GetFloatVar("ModelLOD");}
    uint8       Shadows()                   { return GetByteVar("MaxModelShadows"); }
	float		NumBulletHoles()			{ return GetFloatVar("BulletHoles");}
	float		TextureDetailSetting()		{ return GetFloatVar("TextureDetail");}
	float		DynamicLightSetting()		{ return GetFloatVar("DynamicLightSetting");}
    LTBOOL       LightMap()                 { return GetBoolVar("LightMap");}
    uint8       SpecialFXSetting()          { return static_cast<uint8>(GetConsoleInt("PerformanceLevel",1)); }
    LTBOOL       EnvironmentMapping()       { return GetBoolVar("EnvMapEnable"); }
    LTBOOL       ModelFullBrights()         { return GetBoolVar("ModelFullbrite"); }
    LTBOOL       CloudMapLight()            { return GetBoolVar("CloudMapLight"); }
    uint8       PlayerViewWeaponSetting()   { return GetByteVar("PVWeapons"); }
    LTBOOL       PolyGrids()                { return GetBoolVar("PolyGrids"); }
    LTBOOL       DrawSky()                  { return GetBoolVar("DrawSky"); }
    LTBOOL       FogEnable()                { return GetBoolVar("FogEnable"); }

	// settings implementation functions

	void		ImplementMusicSource();
	void		ImplementMusicVolume();
	void		ImplementSoundVolume();
	void		ImplementSoundQuality();
	void		ImplementMouseSensitivity();


private:

    ILTClient*          m_pClientDE;
	CGameClientShell*	m_pClientShell;
	RMode				CurrentRenderer;


	HCONSOLEVAR m_hTmpVar;
	char		m_tmpStr[128];


};

inline LTBOOL    CGameSettings::GetBoolVar(const char *pVar)
{
	m_hTmpVar = m_pClientDE->GetConsoleVar(pVar);
	if (m_hTmpVar)
        return (LTBOOL)m_pClientDE->GetVarValueFloat(m_hTmpVar);
	else
        return LTFALSE;
};


inline  void    CGameSettings::SetBoolVar(const char *pVar, LTBOOL bVal)
{
	sprintf (m_tmpStr, "+%s %d", pVar, bVal ? 1 : 0);
	m_pClientDE->RunConsoleString (m_tmpStr);
};

inline  uint8   CGameSettings::GetByteVar(const char *pVar)
{
	m_hTmpVar = m_pClientDE->GetConsoleVar(pVar);
	if (m_hTmpVar)
        return (uint8)m_pClientDE->GetVarValueFloat(m_hTmpVar);
	else
		return 0;
};

inline  void    CGameSettings::SetByteVar(const char *pVar, uint8 nVal)
{
	sprintf (m_tmpStr, "+%s %d", pVar, nVal);
	m_pClientDE->RunConsoleString (m_tmpStr);
}


inline	float	CGameSettings::GetFloatVar(const char *pVar)
{
	m_hTmpVar = m_pClientDE->GetConsoleVar(pVar);
	if (m_hTmpVar)
		return m_pClientDE->GetVarValueFloat(m_hTmpVar);
	else
		return 0.0f;
};
inline	void	CGameSettings::SetFloatVar(const char *pVar, float fVal)
{
	sprintf (m_tmpStr, "+%s %f", pVar, fVal);
	m_pClientDE->RunConsoleString (m_tmpStr);
}


#endif //__GAMESETTINGS_H
