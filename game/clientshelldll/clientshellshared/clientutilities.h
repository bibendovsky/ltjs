// ----------------------------------------------------------------------- //
//
// MODULE  : ClientUtilities.h
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_UTILITIES_H__
#define __CLIENT_UTILITIES_H__

#include "soundtypes.h"
#include "iltclient.h"
#include "commonutilities.h"
#include "soundmgr.h"
#include "clientservershared.h"

#ifdef LTJS_SDL_BACKEND
#include "ltjs_shell_resource_mgr.h"
#include "ltjs_shell_string_formatter.h"
#endif // LTJS_SDL_BACKEND

#define DEG2RAD(x)		(((x)*MATH_PI)/180.0f)
#define RAD2DEG(x)		(((x)*180.0f)/MATH_PI)

struct DSize
{
	DSize()		{ cx = 0; cy = 0; }

	unsigned long	cx;
	unsigned long	cy;
};

HSURFACE CropSurface(HSURFACE hSurf, HLTCOLOR hBorderColor);

bool	GetConsoleBool(char const* sKey, bool bDefault);
int		GetConsoleInt(char const* sKey, int nDefault);
void	GetConsoleString(char const* sKey, char* sDest, char const* sDefault);
char*	GetConsoleTempString(char const* sKey, char const* sDefault);
LTFLOAT GetConsoleFloat(char const* sKey, LTFLOAT fDefault);
void	WriteConsoleBool(char const* sKey, bool bValue);
void	WriteConsoleInt(char const* sKey, int nValue);
void	WriteConsoleString(char const* sKey, char const* sValue);
void	WriteConsoleFloat(char const* sKey, LTFLOAT fValue);


// The following two functions should be used to determine how long a block
// of code takes to execute.  For example:
//
// StartTimingCounter();
// float p1 = 30.0f, p2 = 50.0f;
// Function(p1, p2);
// EndTimingCounter("Function(%.2f, %.2f)", p1, p2);
//
// If "Function" took 1000 ticks to execute, the above code would print in
// the console:
//		Function(30.00, 50.00) : 1000 ticks
//
// NOTE:  The timing information is only printed to the console if the server
// console variable "ShowTiming" is set to 1. (i.e., showtiming 1)
void StartTimingCounter();
void EndTimingCounter(char *msg, ...);

inline LTBOOL GetAttachmentSocketTransform(HOBJECT hObj, const char* pSocketName,
                                          LTVector & vPos, LTRotation & rRot)
{
    if (!hObj || !pSocketName) return LTFALSE;

	HOBJECT hAttachList[30];
    uint32 dwListSize, dwNumAttachments;

    if (g_pCommonLT->GetAttachments(hObj, hAttachList,
		ARRAY_LEN(hAttachList), dwListSize, dwNumAttachments) == LT_OK)
	{
        for (uint32 i=0; i < dwListSize; i++)
		{
			if (hAttachList[i])
			{
				HMODELSOCKET hSocket;

				if (g_pModelLT->GetSocket(hAttachList[i], pSocketName, hSocket) == LT_OK)
				{
					LTransform transform;
                    if (g_pModelLT->GetSocketTransform(hAttachList[i], hSocket, transform, LTTRUE) == LT_OK)
					{
						vPos = transform.m_Pos;
						rRot = transform.m_Rot;
                        return LTTRUE;
					}
				}
			}
		}
	}

    return LTFALSE;
}

struct WEAPON;
enum PlayerSoundId;

void PlayWeaponSound(WEAPON const *pWeapon, const LTVector &vPos, PlayerSoundId eSoundId,
					 LTBOOL bLocal=LTFALSE);

#define IsKeyDown(key)		(GetAsyncKeyState(key) & 0x80000000)

// Send an empty message to the server
LTRESULT SendEmptyServerMsg(uint32 nMsgID, uint32 nFlags = MESSAGE_GUARANTEED);

//load and/or format a string from CRes
const int kMaxStringBuffer = 2048;

#ifndef LTJS_SDL_BACKEND
void FormatString(int messageCode, char *outBuf, int outBufLen,  ...);
#endif // LTJS_SDL_BACKEND

void LoadString(int messageCode, char *outBuf, int outBufLen);

//load and format a string from CRes and return a pointer to a static buffer containing that string
#ifndef LTJS_SDL_BACKEND
char* FormatTempString(int messageCode, ...);
#endif // LTJS_SDL_BACKEND

char* LoadTempString(int messageCode);


bool IsMultiplayerGame();

// Helper for finding the normal and point of intersection beneath a given point
bool GetIntersectionUnderPoint( LTVector &vInPt, HOBJECT *pFilterList, LTVector &vOutNormal, LTVector &vOutPt );

// Find the normal of the plane we would like to contour to given a position and dims of an object.
LTVector GetContouringNormal( LTVector &vPos, LTVector &vDims, LTVector &vForward, LTVector &vRight, HOBJECT *pFilterList );

// Get the pitch amount and percents to apply for pitch and roll based on the forward direction and plane normal.
void GetContouringInfo( LTVector &vForward, LTVector &vNormal, float &fOutAmount, float &fOutPitchPercent, float &fOutRollPercent );


#ifdef LTJS_SDL_BACKEND
void ltjs_format_string(
	int messageCode,
	char* outBuf,
	int outBufLen,
	ltjs::ShellStringFormatter& formatter) noexcept;

template<
	typename ...TArgs
>
inline void FormatString(
	int messageCode,
	char* outBuf,
	int outBufLen,
	TArgs... args) noexcept
{
	auto formatter = ltjs::ShellStringFormatter{std::forward<TArgs>(args)...};
	ltjs_format_string(messageCode, outBuf, outBufLen, formatter);
}


char* ltjs_format_temp_string(
	int messageCode,
	ltjs::ShellStringFormatter& formatter) noexcept;

template<
	typename ...TArgs
>
inline char* FormatTempString(
	int messageCode,
	TArgs... args) noexcept
{
	auto formatter = ltjs::ShellStringFormatter{std::forward<TArgs>(args)...};
	return ltjs_format_temp_string(messageCode, formatter);
}


ltjs::ShellResourceMgr* ltjs_get_cres_mgr();

void ltjs_initialize_cres_mgr(
	const char* game_net_name);
#endif // LTJS_SDL_BACKEND


#endif // __CLIENT_UTILITIES_H__
