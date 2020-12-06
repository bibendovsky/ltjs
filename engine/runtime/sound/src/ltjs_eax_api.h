#ifndef LTJS_EAX_API_INCLUDED
#define LTJS_EAX_API_INCLUDED


#include <cstdint>

#include <array>

#ifdef AL_VERSION_1_1
#include "al.h"
#endif // AL_VERSION_1_1


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#ifndef GUID_DEFINED
#define GUID_DEFINED

typedef struct _GUID
{
	std::uint32_t Data1;
	std::uint16_t Data2;
	std::uint16_t Data3;
	std::uint8_t Data4[8];
} GUID;

#endif // !GUID_DEFINED

struct EAXGUID
{
	std::uint32_t Data1;
	std::uint16_t Data2;
	std::uint16_t Data3;
	std::uint8_t Data4[8];
};

struct EAXVECTOR
{
	float x;
	float y;
	float z;
}; // EAXVECTOR

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

extern "C" const GUID DSPROPSETID_EAX20_ListenerProperties;
#define DSPROPSETID_EAX_ListenerProperties DSPROPSETID_EAX20_ListenerProperties


enum : std::uint32_t
{
	DSPROPERTY_EAXLISTENER_NONE,
	DSPROPERTY_EAXLISTENER_ALLPARAMETERS,
	DSPROPERTY_EAXLISTENER_ROOM,
	DSPROPERTY_EAXLISTENER_ROOMHF,
	DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR,
	DSPROPERTY_EAXLISTENER_DECAYTIME,
	DSPROPERTY_EAXLISTENER_DECAYHFRATIO,
	DSPROPERTY_EAXLISTENER_REFLECTIONS,
	DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY,
	DSPROPERTY_EAXLISTENER_REVERB,
	DSPROPERTY_EAXLISTENER_REVERBDELAY,
	DSPROPERTY_EAXLISTENER_ENVIRONMENT,
	DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE,
	DSPROPERTY_EAXLISTENER_ENVIRONMENTDIFFUSION,
	DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF,
	DSPROPERTY_EAXLISTENER_FLAGS,
};

enum : std::uint32_t
{
	EAX_ENVIRONMENT_GENERIC,
	EAX_ENVIRONMENT_PADDEDCELL,
	EAX_ENVIRONMENT_ROOM,
	EAX_ENVIRONMENT_BATHROOM,
	EAX_ENVIRONMENT_LIVINGROOM,
	EAX_ENVIRONMENT_STONEROOM,
	EAX_ENVIRONMENT_AUDITORIUM,
	EAX_ENVIRONMENT_CONCERTHALL,
	EAX_ENVIRONMENT_CAVE,
	EAX_ENVIRONMENT_ARENA,
	EAX_ENVIRONMENT_HANGAR,
	EAX_ENVIRONMENT_CARPETEDHALLWAY,
	EAX_ENVIRONMENT_HALLWAY,
	EAX_ENVIRONMENT_STONECORRIDOR,
	EAX_ENVIRONMENT_ALLEY,
	EAX_ENVIRONMENT_FOREST,
	EAX_ENVIRONMENT_CITY,
	EAX_ENVIRONMENT_MOUNTAINS,
	EAX_ENVIRONMENT_QUARRY,
	EAX_ENVIRONMENT_PLAIN,
	EAX_ENVIRONMENT_PARKINGLOT,
	EAX_ENVIRONMENT_SEWERPIPE,
	EAX_ENVIRONMENT_UNDERWATER,
	EAX_ENVIRONMENT_DRUGGED,
	EAX_ENVIRONMENT_DIZZY,
	EAX_ENVIRONMENT_PSYCHOTIC,

	EAX_ENVIRONMENT_UNDEFINED,

	EAX_ENVIRONMENT_COUNT,
};


struct EAXLISTENERPROPERTIES
{
	std::int32_t lRoom;
	std::int32_t lRoomHF;
	float flRoomRolloffFactor;
	float flDecayTime;
	float flDecayHFRatio;
	std::int32_t lReflections;
	float flReflectionsDelay;
	std::int32_t lReverb;
	float flReverbDelay;
	std::uint32_t dwEnvironment;
	float flEnvironmentSize;
	float flEnvironmentDiffusion;
	float flAirAbsorptionHF;
	std::uint32_t dwFlags;
}; // EAXLISTENERPROPERTIES

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

extern "C" const GUID DSPROPSETID_EAX20_BufferProperties;
#define DSPROPSETID_EAX_BufferProperties DSPROPSETID_EAX20_BufferProperties


enum : std::uint32_t
{
	DSPROPERTY_EAXBUFFER_NONE,
	DSPROPERTY_EAXBUFFER_ALLPARAMETERS,
	DSPROPERTY_EAXBUFFER_DIRECT,
	DSPROPERTY_EAXBUFFER_DIRECTHF,
	DSPROPERTY_EAXBUFFER_ROOM,
	DSPROPERTY_EAXBUFFER_ROOMHF,
	DSPROPERTY_EAXBUFFER_ROOMROLLOFFFACTOR,
	DSPROPERTY_EAXBUFFER_OBSTRUCTION,
	DSPROPERTY_EAXBUFFER_OBSTRUCTIONLFRATIO,
	DSPROPERTY_EAXBUFFER_OCCLUSION,
	DSPROPERTY_EAXBUFFER_OCCLUSIONLFRATIO,
	DSPROPERTY_EAXBUFFER_OCCLUSIONROOMRATIO,
	DSPROPERTY_EAXBUFFER_OUTSIDEVOLUMEHF,
	DSPROPERTY_EAXBUFFER_AIRABSORPTIONFACTOR,
	DSPROPERTY_EAXBUFFER_FLAGS,
};

constexpr auto EAXBUFFER_MINDIRECT = std::int32_t{-10'000};
constexpr auto EAXBUFFER_MAXDIRECT = std::int32_t{1'000};
constexpr auto EAXBUFFER_DEFAULTDIRECT = std::int32_t{0};


struct EAXBUFFERPROPERTIES
{
	std::int32_t lDirect;
	std::int32_t lDirectHF;
	std::int32_t lRoom;
	std::int32_t lRoomHF;
	float flRoomRolloffFactor;
	std::int32_t lObstruction;
	float flObstructionLFRatio;
	std::int32_t lOcclusion;
	float flOcclusionLFRatio;
	float flOcclusionRoomRatio;
	std::int32_t lOutsideVolumeHF;
	float flAirAbsorptionFactor;
	std::uint32_t dwFlags;
}; // EAXBUFFERPROPERTIES

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

constexpr auto EAXREVERB_MINENVIRONMENT = std::uint32_t{0};
constexpr auto EAX20REVERB_MAXENVIRONMENT = std::uint32_t{EAX_ENVIRONMENT_COUNT - 2};
constexpr auto EAX30REVERB_MAXENVIRONMENT = std::uint32_t{EAX_ENVIRONMENT_COUNT - 1};
constexpr auto EAXREVERB_DEFAULTENVIRONMENT = std::uint32_t{EAX_ENVIRONMENT_GENERIC};

constexpr auto EAXREVERB_MINENVIRONMENTSIZE = 1.0F;
constexpr auto EAXREVERB_MAXENVIRONMENTSIZE = 100.0F;
constexpr auto EAXREVERB_DEFAULTENVIRONMENTSIZE = 7.5F;

constexpr auto EAXREVERB_MINENVIRONMENTDIFFUSION = 0.0F;
constexpr auto EAXREVERB_MAXENVIRONMENTDIFFUSION = 1.0F;
constexpr auto EAXREVERB_DEFAULTENVIRONMENTDIFFUSION = 1.0F;

constexpr auto EAXREVERB_MINROOM = std::int32_t{-10'000};
constexpr auto EAXREVERB_MAXROOM = std::int32_t{0};
constexpr auto EAXREVERB_DEFAULTROOM = std::int32_t{-1'000};

constexpr auto EAXREVERB_MINROOMHF = std::int32_t{-10'000};
constexpr auto EAXREVERB_MAXROOMHF = std::int32_t{0};
constexpr auto EAXREVERB_DEFAULTROOMHF = std::int32_t{-100};

constexpr auto EAXREVERB_MINROOMLF = std::int32_t{-10'000};
constexpr auto EAXREVERB_MAXROOMLF = std::int32_t{0};
constexpr auto EAXREVERB_DEFAULTROOMLF = std::int32_t{0};

constexpr auto EAXREVERB_MINDECAYTIME = 0.1F;
constexpr auto EAXREVERB_MAXDECAYTIME = 20.0F;
constexpr auto EAXREVERB_DEFAULTDECAYTIME = 1.49F;

constexpr auto EAXREVERB_MINDECAYHFRATIO = 0.1F;
constexpr auto EAXREVERB_MAXDECAYHFRATIO = 2.0F;
constexpr auto EAXREVERB_DEFAULTDECAYHFRATIO = 0.83F;

constexpr auto EAXREVERB_MINDECAYLFRATIO = 0.1F;
constexpr auto EAXREVERB_MAXDECAYLFRATIO = 2.0F;
constexpr auto EAXREVERB_DEFAULTDECAYLFRATIO = 1.00F;

constexpr auto EAXREVERB_MINREFLECTIONS = std::int32_t{-10'000};
constexpr auto EAXREVERB_MAXREFLECTIONS = std::int32_t{1'000};
constexpr auto EAXREVERB_DEFAULTREFLECTIONS = std::int32_t{-2'602};

constexpr auto EAXREVERB_MINREFLECTIONSDELAY = 0.0F;
constexpr auto EAXREVERB_MAXREFLECTIONSDELAY = 0.3F;
constexpr auto EAXREVERB_DEFAULTREFLECTIONSDELAY = 0.007F;

constexpr auto EAXREVERB_DEFAULTREFLECTIONSPAN = EAXVECTOR{};

constexpr auto EAXREVERB_MINREVERB = std::int32_t{-10'000};
constexpr auto EAXREVERB_MAXREVERB = std::int32_t{2'000};
constexpr auto EAXREVERB_DEFAULTREVERB = std::int32_t{200};

constexpr auto EAXREVERB_MINREVERBDELAY = 0.0F;
constexpr auto EAXREVERB_MAXREVERBDELAY = 0.1F;
constexpr auto EAXREVERB_DEFAULTREVERBDELAY = 0.011F;

constexpr auto EAXREVERB_DEFAULTREVERBPAN = EAXVECTOR{};

constexpr auto EAXREVERB_MINECHOTIME = 0.075F;
constexpr auto EAXREVERB_MAXECHOTIME = 0.25F;
constexpr auto EAXREVERB_DEFAULTECHOTIME = 0.25F;

constexpr auto EAXREVERB_MINECHODEPTH = 0.0F;
constexpr auto EAXREVERB_MAXECHODEPTH = 1.0F;
constexpr auto EAXREVERB_DEFAULTECHODEPTH = 0.0F;

constexpr auto EAXREVERB_MINMODULATIONTIME = 0.04F;
constexpr auto EAXREVERB_MAXMODULATIONTIME = 4.0F;
constexpr auto EAXREVERB_DEFAULTMODULATIONTIME = 0.25F;

constexpr auto EAXREVERB_MINMODULATIONDEPTH = 0.0F;
constexpr auto EAXREVERB_MAXMODULATIONDEPTH = 1.0F;
constexpr auto EAXREVERB_DEFAULTMODULATIONDEPTH = 0.0F;

constexpr auto EAXREVERB_MINAIRABSORPTIONHF = -100.0F;
constexpr auto EAXREVERB_MAXAIRABSORPTIONHF = 0.0F;
constexpr auto EAXREVERB_DEFAULTAIRABSORPTIONHF = -5.0F;

constexpr auto EAXREVERB_MINHFREFERENCE = 1'000.0F;
constexpr auto EAXREVERB_MAXHFREFERENCE = 20'000.0F;
constexpr auto EAXREVERB_DEFAULTHFREFERENCE = 5'000.0F;

constexpr auto EAXREVERB_MINLFREFERENCE = 20.0F;
constexpr auto EAXREVERB_MAXLFREFERENCE = 1'000.0F;
constexpr auto EAXREVERB_DEFAULTLFREFERENCE = 250.0F;

constexpr auto EAXREVERB_MINROOMROLLOFFFACTOR = 0.0F;
constexpr auto EAXREVERB_MAXROOMROLLOFFFACTOR = 10.0F;
constexpr auto EAXREVERB_DEFAULTROOMROLLOFFFACTOR = 0.0F;


constexpr auto EAXREVERBFLAGS_DECAYTIMESCALE = std::uint32_t{0x00000001};
constexpr auto EAXREVERBFLAGS_REFLECTIONSSCALE = std::uint32_t{0x00000002};
constexpr auto EAXREVERBFLAGS_REFLECTIONSDELAYSCALE = std::uint32_t{0x00000004};
constexpr auto EAXREVERBFLAGS_REVERBSCALE = std::uint32_t{0x00000008};
constexpr auto EAXREVERBFLAGS_REVERBDELAYSCALE = std::uint32_t{0x00000010};
constexpr auto EAXREVERBFLAGS_ECHOTIMESCALE = std::uint32_t{0x00000040};
constexpr auto EAXREVERBFLAGS_MODULATIONTIMESCALE = std::uint32_t{0x00000080};
constexpr auto EAXREVERBFLAGS_DECAYHFLIMIT = std::uint32_t{0x00000020};


constexpr auto EAXREVERB_DEFAULTFLAGS =
	EAXREVERBFLAGS_DECAYTIMESCALE |
	EAXREVERBFLAGS_REFLECTIONSSCALE |
	EAXREVERBFLAGS_REFLECTIONSDELAYSCALE |
	EAXREVERBFLAGS_REVERBSCALE |
	EAXREVERBFLAGS_REVERBDELAYSCALE |
	EAXREVERBFLAGS_DECAYHFLIMIT;

struct EAXREVERBPROPERTIES
{
	std::uint32_t ulEnvironment;
	float flEnvironmentSize;
	float flEnvironmentDiffusion;
	std::int32_t lRoom;
	std::int32_t lRoomHF;
	std::int32_t lRoomLF;
	float flDecayTime;
	float flDecayHFRatio;
	float flDecayLFRatio;
	std::int32_t lReflections;
	float flReflectionsDelay;
	EAXVECTOR vReflectionsPan;
	std::int32_t lReverb;
	float flReverbDelay;
	EAXVECTOR vReverbPan;
	float flEchoTime;
	float flEchoDepth;
	float flModulationTime;
	float flModulationDepth;
	float flAirAbsorptionHF;
	float flHFReference;
	float flLFReference;
	float flRoomRolloffFactor;
	std::uint32_t ulFlags;
}; // EAXREVERBPROPERTIES

// --------------------------------------------------------------------------

using EaxReverbPresets = std::array<EAXREVERBPROPERTIES, EAX_ENVIRONMENT_UNDEFINED>;


extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_GENERIC;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_PADDEDCELL;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_ROOM;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_BATHROOM;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_LIVINGROOM;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_STONEROOM;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_AUDITORIUM;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_CONCERTHALL;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_CAVE;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_ARENA;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_HANGAR;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_CARPETTEDHALLWAY;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_HALLWAY;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_STONECORRIDOR;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_ALLEY;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_FOREST;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_CITY;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_MOUNTAINS;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_QUARRY;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_PLAIN;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_PARKINGLOT;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_SEWERPIPE;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_UNDERWATER;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_DRUGGED;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_DIZZY;
extern const EAXREVERBPROPERTIES EAXREVERB_PRESET_PSYCHOTIC;

extern const EaxReverbPresets EAXREVERB_PRESETS;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#ifdef AL_VERSION_1_1

using EAXSet = ALenum (AL_APIENTRY *)(
	const GUID* property_set_guid,
	ALuint property_id,
	ALuint property_source,
	ALvoid* property_buffer,
	ALuint property_size);

using EAXGet = ALenum (AL_APIENTRY *)(
	const GUID* property_set_guid,
	ALuint property_id,
	ALuint property_source,
	ALvoid* property_buffer,
	ALuint property_size);

#endif // AL_VERSION_1_1

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


#endif // !LTJS_EAX_API_INCLUDED
