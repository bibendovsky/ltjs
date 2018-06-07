#ifndef __ILTSOUND_H__
#define __ILTSOUND_H__


#include "bibendovsky_spul_wave_format.h"


#ifndef __LTBASETYPES_H__
#include "ltbasetypes.h"
#endif

#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif


namespace ul = bibendovsky::spul;


#ifndef NULL
#define NULL			0x0
#endif	// NULL

#ifndef MAKE_BOOL
#define MAKE_BOOL( ltBoolVal )	( ( ( ltBoolVal ) == 0 ) ? false : true )
#define MAKE_LTBOOL( bBoolVal )	( ( ( bBoolVal ) == false ) ? LTFALSE : LTTRUE )
#endif	// MAKE_BOOL

#define LS_8_MONO_PCM			0
#define LS_16					1
#define LS_STEREO				2
#define	LS_ADPCM				4
#define LS_ASI					16

#define LS_FREE					0x0001		// Sample is available for allocation
#define LS_DONE					0x0002		// Sample has finished playing, or has never been started
#define LS_PLAYING				0x0004		// Sample is playing
#define LS_STOPPED				0x0008		// Sample has been stopped

#define LS_PROENUM_FIRST		0

#define LS_ROOM_GENERIC			0			// factory default
#define LS_ROOM_TYPE_COUNT		26			// total number of environments

#define LS_PREF_MIXER_CHANNELS		1
#define LS_PREF_USE_WAVEOUT			15
#define LS_PREF_LOCK_PROTECTION		18
#define LS_PREF_ENABLE_MMX			27		// Enable MMX support if present
#define LS_PREF_REVERB_BUFFER_SIZE	40

// Uncomment one of these to use filtering, only one can be active
//#define USE_DX8_SOFTWARE_FILTERS
#ifndef USE_EAX20_HARDWARE_FILTERS
#define USE_EAX20_HARDWARE_FILTERS
#endif

#ifdef USE_EAX20_HARDWARE_FILTERS
// Multiplies EAX attributes (room, reflections and reverb).
#define LTJS_EAX20_SCALE_ATTRIBUTES
// The multipliers.
#define LTJS_EAX20_ROOM_SCALE_FACTOR (1.5F)
#define LTJS_EAX20_REFLECTIONS_SCALE_FACTOR (2.5F)
#define LTJS_EAX20_REVERB_SCALE_FACTOR (2.0F)
#endif // USE_EAX20_HARDWARE_FILTERS

typedef short			sint16;				//S16;
typedef unsigned short	uint16;				//U16;
typedef long			sint32;				//S32;
typedef unsigned int	uint32;				//U32;
typedef void*			lpvoid;				//PTR;

#define LS_OK			0x0
#define LS_ERROR		0x1

typedef sint32			( *LTLENGTHYCB )	( uint32 uiState, uint32 uiUser );	//AILLENGTHYCB

typedef uint32			LHPROVIDER;			//HPROVIDER;
typedef uint32			LHPROENUM;			//HPROENUM;

typedef lpvoid			LHSAMPLE;			//HSAMPLE;
typedef lpvoid			LH3DPOBJECT;		//H3DPOBJECT;
typedef lpvoid			LH3DSAMPLE;			//H3DSAMPLE;
typedef lpvoid			LHDIGDRIVER;		//HDIGDRIVER;
typedef lpvoid			LHSTREAM;			//HSTREAM;

typedef lpvoid			PHWAVEOUT;
typedef lpvoid			PTDIRECTSOUND;
typedef lpvoid			PTDIRECTSOUNDBUFFER;

//! LTSOUNDINFO

typedef struct
{
	sint32 format;
	lpvoid data_ptr;
	uint32 data_len;
	uint32 rate;
	sint32 bits;
	sint32 channels;
	uint32 samples;
	uint32 block_size;
	lpvoid initial_ptr;
} LTSOUNDINFO;			//AILSOUNDINFO;
typedef LTSOUNDINFO*	PTSOUNDINFO;

class LTSOUNDFILTER
{
public:
	virtual void SetParam( const char* pszParam, float fValue ) = 0;
	uint32	uiFilterParamFlags;
};

#ifdef USE_DX8_SOFTWARE_FILTERS

class LTFILTERCOMPRESSOR : public LTSOUNDFILTER
{
public:
	void SetParam( const char* pszParam, float fValue );

	float  fGain;
	float  fAttack;
	float  fRelease;
	float  fThreshold;
	float  fRatio;
	float  fPredelay;
};

class LTFILTERCHORUS : public LTSOUNDFILTER
{
public:
  void SetParam( const char* pszParam, float fValue );

  float   fWetDryMix;
  float   fDepth;
  float   fFeedback;
  float   fFrequency;
  int	  lWaveform;
  float   fDelay;
  int	  lPhase;
};

class LTFILTERDISTORTION : public LTSOUNDFILTER
{
public:
  void SetParam( const char* pszParam, float fValue );

  float  fGain;
  float  fEdge;
  float  fPostEQCenterFrequency;
  float  fPostEQBandwidth;
  float  fPreLowpassCutoff;
};

class LTFILTERECHO : public LTSOUNDFILTER
{
public:
  void SetParam( const char* pszParam, float fValue );

  float  fWetDryMix;
  float  fFeedback;
  float  fLeftDelay;
  float  fRightDelay;
  int	 lPanDelay;
};

class LTFILTERFLANGE : public LTSOUNDFILTER
{
public:
  void SetParam( const char* pszParam, float fValue );

  float  fWetDryMix;
  float  fDepth;
  float  fFeedback;
  float  fFrequency;
  int    lWaveform;
  float  fDelay;
  int    lPhase;
};

class LTFILTERPARAMEQ : public LTSOUNDFILTER
{
public:
  void SetParam( const char* pszParam, float fValue );

  float  fCenter;
  float  fBandwidth;
  float  fGain;
};

class LTFILTERREVERB : public LTSOUNDFILTER
{
public:
  void SetParam( const char* pszParam, float fValue );

  int	lRoom;
  int	lRoomHF; 
  float fRoomRolloffFactor;
  float fDecayTime;
  float fDecayHFRatio;
  int   lReflections;
  float fReflectionsDelay;
  int	lReverb;
  float fReverbDelay; 
  float fDiffusion;
  float fDensity;
  float fHFReference;
};

#endif

#ifdef USE_EAX20_HARDWARE_FILTERS

class LTFILTERREVERB : public LTSOUNDFILTER
{
public:
  void SetParam( const char* pszParam, float fValue );

  int	lEnvironment;
  int	lRoom;
  int	lRoomHF; 
  float fRoomRolloffFactor;
  float fDecayTime;
  float fDecayHFRatio;
  int   lReflections;
  float fReflectionsDelay;
  int	lReverb;
  float fReverbDelay; 
  float fDiffusion;
  float fSize;
  float fAirAbsorptionHF;
  int	lDirect;
};

#endif

#define NULL_FILTER	-1

// doppler values
// these come from DirectSound, they are multiples of real-world Doppler response
// so 0 is no Doppler, while 2 is twice the normal Doppler effect
#define MIN_DOPPLER	0.0f
#define MAX_DOPPLER 10.0f


// filter parameter flags

#ifdef USE_DX8_SOFTWARE_FILTERS

// chorus params
#define SET_CHORUS_WETDRYMIX	1
#define SET_CHORUS_DEPTH		2
#define SET_CHORUS_FEEDBACK		4
#define SET_CHORUS_FREQUENCY	8
#define SET_CHORUS_WAVEFORM		16
#define SET_CHORUS_DELAY		32
#define SET_CHORUS_PHASE		64
// compressor params
#define SET_COMPRESSOR_GAIN			1
#define SET_COMPRESSOR_ATTACK		2
#define SET_COMPRESSOR_RELEASE		4
#define SET_COMPRESSOR_THRESHOLD	8
#define SET_COMPRESSOR_RATIO		16
#define SET_COMPRESSOR_PREDELAY		32
// distortion params
#define SET_DISTORTION_GAIN				1
#define SET_DISTORTION_EDGE				2	
#define	SET_DISTORTION_POSTEQCENTERFREQ	4
#define SET_DISTORTION_POSTEQBANDWIDTH	8
#define SET_DISTORTION_PRELOWPASSCUTOFF	16
// echo params
#define SET_ECHO_WETDRYMIX	1
#define SET_ECHO_FEEDBACK	2
#define SET_ECHO_LEFTDELAY	4
#define SET_ECHO_RIGHTDELAY	8
#define SET_ECHO_PANDELAY	16
// flange params
#define	SET_FLANGE_DEPTH		1 
#define	SET_FLANGE_FEEDBACK		2
#define SET_FLANGE_FREQUENCY	4
#define SET_FLANGE_WAVEFORM		8
#define	SET_FLANGE_DELAY		16
#define SET_FLANGE_PHASE		32
#define SET_FLANGE_WETDRYMIX	64
// parametric eq params
#define SET_PARAMEQ_CENTER		1
#define SET_PARAMEQ_BANDWIDTH	2
#define SET_PARAMEQ_GAIN		4
// reverb params
#define SET_REVERB_ENVIRONMENT			1
#define SET_REVERB_ROOM					1
#define SET_REVERB_ROOMHF				2
#define SET_REVERB_ROOMROLLOFFFACTOR	4
#define SET_REVERB_DECAYTIME			8
#define SET_REVERB_DECAYHFRATIO			16
#define SET_REVERB_REFLECTIONS			32
#define SET_REVERB_REFLECTIONSDELAY		64
#define SET_REVERB_REVERB				128
#define SET_REVERB_REVERBDELAY			256
#define SET_REVERB_DIFFUSION			512
#define SET_REVERB_DENSITY				1024
#define SET_REVERB_HFREFERENCE			2048

// filter types
enum
{
	FilterChorus = 0,
	FilterCompressor,
	FilterDistortion,
	FilterEcho,
	FilterFlange,
	FilterParamEQ,
	FilterReverb,
	NUM_SOUND_FILTER_TYPES
};

#endif

#ifdef USE_EAX20_HARDWARE_FILTERS

// EAX 2.0 reverb params
#define SET_REVERB_ENVIRONMENT			(1<<0)
#define SET_REVERB_ROOM					(1<<1)
#define SET_REVERB_ROOMHF				(1<<2)
#define SET_REVERB_ROOMROLLOFFFACTOR	(1<<3)
#define SET_REVERB_DECAYTIME			(1<<4)
#define SET_REVERB_DECAYHFRATIO			(1<<5)
#define SET_REVERB_REFLECTIONS			(1<<6)
#define SET_REVERB_REFLECTIONSDELAY		(1<<7)
#define SET_REVERB_REVERB				(1<<8)
#define SET_REVERB_REVERBDELAY			(1<<9)
#define SET_REVERB_DIFFUSION			(1<<10)
#define SET_REVERB_SIZE					(1<<11)
#define SET_REVERB_AIRABSORPTIONHF		(1<<12)
#define SET_REVERB_DIRECT				(1<<13)

enum
{
	FilterReverb = 0,
	NUM_SOUND_FILTER_TYPES
};

#endif


typedef struct
{
	bool	bUseFilter;
	uint32	uiFilterType;
	LTSOUNDFILTER* pSoundFilter;
} LTSOUNDFILTERDATA;

#ifndef DDI_ID_LPDIRECTSOUND

#define DDI_ID_LPDIRECTSOUND	0x1
#define DDI_ID_HDIGDRIVER		0x2

#endif	// DDIID_LPDIRECTSOUND

//! streamBufferParams_t

enum
{
	SBP_BUFFER_SIZE,
	SBP_BITS_PER_CHANNEL,
	SBP_CHANNELS_PER_SAMPLE,
	SBP_SAMPLES_PER_SEC,
	//
	SBP_NUM_PARAMS

};

struct streamBufferParams_t
{
	sint32	m_siParams[ SBP_NUM_PARAMS ];
};

// sound system abstract base class
// used by the CSoundMgr class to access and control the platform dependent sound system

//! ILTSoundSys

class ILTSoundSys
{
protected:
	ILTSoundSys( ) {}
	virtual ~ILTSoundSys( ) {}

public:
	virtual bool		Init( ) = 0;
	virtual void		Term( ) = 0;

public:
	virtual void*		GetDDInterface( const uint uiDDInterfaceId ) = 0;

public:
	using GenericStreamHandle = void*;


	// system wide functions
	virtual void		Lock( void ) = 0;
	virtual void		Unlock( void ) = 0;
	virtual sint32		Startup( void ) = 0;
	virtual void		Shutdown( void ) = 0;
	virtual uint32		MsCount( void ) = 0;
	virtual sint32		SetPreference( const uint32 uiNumber, const sint32 siValue ) = 0;
	virtual sint32		GetPreference( const uint32 uiNumber ) = 0;
	virtual void		MemFreeLock( void* ptr ) = 0;
	virtual void*		MemAllocLock( const uint32 uiSize ) = 0;
	virtual const char*	LastError( void ) = 0;

	// digital sound driver functions
	virtual sint32		WaveOutOpen( LHDIGDRIVER& phDriver, PHWAVEOUT& pphWaveOut, const sint32 siDeviceId, const ul::WaveFormatEx& pWaveFormat ) = 0;
	virtual void		WaveOutClose( LHDIGDRIVER hDriver ) = 0;
	virtual void		SetDigitalMasterVolume( LHDIGDRIVER hDig, const sint32 siMasterVolume ) = 0;
	virtual sint32		GetDigitalMasterVolume( LHDIGDRIVER hDig ) = 0;
	virtual sint32		DigitalHandleRelease( LHDIGDRIVER hDriver ) = 0;
	virtual sint32		DigitalHandleReacquire( LHDIGDRIVER hDriver ) = 0;
#ifdef USE_EAX20_HARDWARE_FILTERS
	virtual bool		SetEAX20Filter( const bool bEnable, const LTSOUNDFILTERDATA& pFilterData ) = 0;
	virtual bool		SupportsEAX20Filter() = 0;
	virtual bool		SetEAX20BufferSettings( LHSAMPLE hSample, const LTSOUNDFILTERDATA& pFilterData ) = 0;
#endif

	// 3d sound provider functions
	virtual void		Set3DProviderMinBuffers( const uint32 uiMinBuffers ) = 0;
	virtual sint32		Open3DProvider( LHPROVIDER hLib ) = 0;
	virtual void		Close3DProvider( LHPROVIDER hLib ) = 0;
	virtual void		Set3DProviderPreference( LHPROVIDER hLib, const char* sName, const void* pVal ) = 0;
	virtual void		Get3DProviderAttribute( LHPROVIDER hLib, const char* sName, void* pVal ) = 0;
	virtual sint32		Enumerate3DProviders( LHPROENUM& phNext, LHPROVIDER& phDest, const char*& psName) = 0;

	// 3d listener functions
	virtual LH3DPOBJECT	Open3DListener( LHPROVIDER hLib ) = 0;
	virtual void		Close3DListener( LH3DPOBJECT hListener ) = 0;
	virtual void		SetListenerDoppler( LH3DPOBJECT hListener, const float fDoppler ) = 0;
	virtual void		CommitDeferred() = 0;

	// 3d sound object functions
	virtual void		Set3DPosition( LH3DPOBJECT hObj, const float fX, const float fY, const float fZ) = 0;
	virtual void		Set3DVelocityVector( LH3DPOBJECT hObj, const float fDX_per_s, const float fDY_per_s, const float fDZ_per_s ) = 0;
	virtual void		Set3DOrientation( LH3DPOBJECT hObj, const float fX_face, const float fY_face, const float fZ_face, const float fX_up, const float fY_up, const float fZ_up ) = 0;
	virtual void		Set3DUserData( LH3DPOBJECT hObj, const uint32 uiIndex, const sint32 siValue ) = 0;
	virtual void		Get3DPosition( LH3DPOBJECT hObj, float& pfX, float& pfY, float& pfZ) = 0;
	virtual void		Get3DVelocity( LH3DPOBJECT hObj, float& pfDX_per_ms, float& pfDY_per_ms, float& pfDZ_per_ms ) = 0;
	virtual void		Get3DOrientation( LH3DPOBJECT hObj, float& pfX_face, float& pfY_face, float& pfZ_face, float& pfX_up, float& pfY_up, float& pfZ_up ) = 0;
	virtual sint32		Get3DUserData( LH3DPOBJECT hObj, const uint32 uiIndex) = 0;

	// 3d sound sample functions
	virtual LH3DSAMPLE	Allocate3DSampleHandle( LHPROVIDER hLib ) = 0;
	virtual void		Release3DSampleHandle( LH3DSAMPLE hS ) = 0;
	virtual void		Stop3DSample( LH3DSAMPLE hS ) = 0;
	virtual void		Start3DSample( LH3DSAMPLE hS ) = 0;
	virtual void		Resume3DSample( LH3DSAMPLE hS ) = 0;
	virtual void		End3DSample( LH3DSAMPLE hS ) = 0;
	virtual sint32		Init3DSampleFromAddress( LH3DSAMPLE hS, const void* pStart, const uint32 uiLen, const ul::WaveFormatEx& pWaveFormat, const sint32 siPlaybackRate, const LTSOUNDFILTERDATA* pFilterData  ) = 0;
	virtual sint32		Init3DSampleFromFile( LH3DSAMPLE hS, const void* pFile_image, const sint32 siBlock, const sint32 siPlaybackRate, const LTSOUNDFILTERDATA* pFilterData ) = 0;
	virtual sint32		Get3DSampleVolume( LH3DSAMPLE hS ) = 0;
	virtual void		Set3DSampleVolume( LH3DSAMPLE hS, const sint32 siVolume ) = 0;
	virtual uint32		Get3DSampleStatus( LH3DSAMPLE hS ) = 0;
	virtual void		Set3DSampleMsPosition( LHSAMPLE hS, const sint32 siMilliseconds ) = 0;
	virtual sint32		Set3DSampleInfo( LH3DSAMPLE hS, const LTSOUNDINFO& pInfo ) = 0;
	virtual void		Set3DSampleDistances( LH3DSAMPLE hS, const float fMax_dist, const float fMin_dist ) = 0;
	virtual void		Set3DSamplePreference( LH3DSAMPLE hSample, const char* sName, const void* pVal ) = 0;
	virtual void		Set3DSampleLoopBlock( LH3DSAMPLE hS, const sint32 siLoop_start_offset, const sint32 siLoop_end_offset, const bool bEnable ) = 0;
	virtual void		Set3DSampleLoop( LH3DSAMPLE hS, const bool bLoop ) = 0;
	virtual void		Set3DSampleObstruction( LH3DSAMPLE hS, const float fObstruction ) = 0;
	virtual float		Get3DSampleObstruction( LH3DSAMPLE hS ) = 0;
	virtual void		Set3DSampleOcclusion( LH3DSAMPLE hS, const float fOcclusion ) = 0;
	virtual float		Get3DSampleOcclusion( LH3DSAMPLE hS ) = 0;

	// 2d sound sample functions
	virtual LHSAMPLE	AllocateSampleHandle( LHDIGDRIVER hDig ) = 0;
	virtual void		ReleaseSampleHandle( LHSAMPLE hS ) = 0;
	virtual void		InitSample( LHSAMPLE hS ) = 0;
	virtual void		StopSample( LHSAMPLE hS ) = 0;
	virtual void		StartSample( LHSAMPLE hS ) = 0;
	virtual void		ResumeSample( LHSAMPLE hS ) = 0;
	virtual void		EndSample( LHSAMPLE hS ) = 0;
	virtual void		SetSampleVolume( LHSAMPLE hS, const sint32 siVolume ) = 0;
	virtual void		SetSamplePan( LHSAMPLE hS, const sint32 siPan ) = 0;
	virtual sint32		GetSampleVolume( LHSAMPLE hS ) = 0;
	virtual sint32		GetSamplePan( LHSAMPLE hS ) = 0;
	virtual void		SetSampleUserData( LHSAMPLE hS, const uint32 uiIndex, const sint32 siValue ) = 0;
	virtual void		GetDirectSoundInfo( LHSAMPLE hS, PTDIRECTSOUND& ppDS, PTDIRECTSOUNDBUFFER& ppDSB ) = 0;
	virtual void		SetSampleReverb( LHSAMPLE hS, const float fReverb_level, const float fReverb_reflect_time, const float fReverb_decay_time ) = 0;
	virtual sint32		InitSampleFromAddress( LHSAMPLE hS, const void* pStart, const uint32 uiLen, const ul::WaveFormatEx& pWaveFormat, const sint32 siPlaybackRate, const LTSOUNDFILTERDATA* pFilterData ) = 0;
	virtual sint32		InitSampleFromFile( LHSAMPLE hS, const void* pFile_image, const sint32 siBlock, const sint32 siPlaybackRate, const LTSOUNDFILTERDATA* pFilterData ) = 0;
	virtual void		SetSampleLoopBlock( LHSAMPLE hS, const sint32 siLoop_start_offset, const sint32 siLoop_end_offset, const bool bEnable ) = 0;
	virtual void		SetSampleLoop( LHSAMPLE hS, const bool bLoop ) = 0;
	virtual void		SetSampleMsPosition( LHSAMPLE hS, const sint32 siMilliseconds ) = 0;
	virtual sint32		GetSampleUserData( LHSAMPLE hS, const uint32 uiIndex ) = 0;
	virtual uint32		GetSampleStatus( LHSAMPLE hS ) = 0;

	// old 2d sound stream functions
	virtual LHSTREAM	OpenStream( const char* sFilename, const uint32 nOffset, LHDIGDRIVER hDig, const char* sStream, const sint32 siStream_mem ) = 0;
	virtual void		SetStreamLoop( LHSTREAM hStream, const bool bLoop ) = 0;
	virtual void		SetStreamPlaybackRate( LHSTREAM hStream, const sint32 siRate ) = 0;
	virtual void		SetStreamMsPosition( LHSTREAM hS, const sint32 siMilliseconds ) = 0;
	virtual void		SetStreamUserData( LHSTREAM hS, const uint32 uiIndex, const sint32 siValue) = 0;
	virtual sint32		GetStreamUserData( LHSTREAM hS, const uint32 uiIndex) = 0;

	// new 2d sound stream functions
//	virtual LHSTREAM	OpenStream( streamBufferParams_t* pStreamBufferParams ) = 0;
	virtual void		CloseStream( LHSTREAM hStream ) = 0;
	virtual void		StartStream( LHSTREAM hStream ) = 0;
	virtual void		PauseStream( LHSTREAM hStream, const sint32 siOnOff ) = 0;
	virtual void		ResetStream( LHSTREAM hStream ) = 0;
	virtual void		SetStreamVolume( LHSTREAM hStream, const sint32 siVolume ) = 0;
	virtual void		SetStreamPan( LHSTREAM hStream, const sint32 siPan ) = 0;
	virtual sint32		GetStreamVolume( LHSTREAM hStream ) = 0;
	virtual sint32		GetStreamPan( LHSTREAM hStream ) = 0;
	virtual uint32		GetStreamStatus( LHSTREAM hStream ) = 0;
	virtual sint32		GetStreamBufferParam( LHSTREAM hStream, const uint32 uiParam ) = 0;
	virtual void		ClearStreamBuffer( LHSTREAM hStream, const bool bClearStreamDataQueue = true) = 0;

	// wave file decompression functons
	virtual sint32		DecompressADPCM( const LTSOUNDINFO& pInfo, void*& ppOutData, uint32& puiOutSize ) = 0;
	virtual sint32		DecompressASI( const void* pInData, const uint32 uiInSize, const char* sFilename_ext, void*& ppWav, uint32& puiWavSize, LTLENGTHYCB fnCallback ) = 0;

	// Gets the ticks spent in sound thread.
	virtual uint32		GetThreadedSoundTicks( ) = 0;

	virtual bool		HasOnBoardMemory( ) = 0;

	virtual void handle_focus_lost(
		const bool is_focus_lost) = 0;


	//
	// Generic stereo stream (thread-safe).
	//

	//
	// Opens a generic stream.
	//
	// Parameters:
	//    - sample_rate - destination sample rate.
	//    - buffer_size - a size of the one buffer in the queue.
	//                    Must be multiple of the sample format.
	//
	// Returns:
	//    - Non-null value on success.
	//    - Null otherwise.
	//
	// Notes:
	//    - Thread-safe.
	//    - Expected sample format: 16 bits signed integer, stereo.
	//
	virtual GenericStreamHandle open_generic_stream(
		const int sample_rate,
		const int buffer_size) = 0;

	//
	// Closes a generic stream.
	//
	// Parameters:
	//    - stream_handle - a handle to generic stream.
	//
	// Notes:
	//    - Thread-safe.
	//
	virtual void close_generic_stream(
		GenericStreamHandle stream_handle) = 0;

	//
	// Gets a queue size.
	//
	// Returns:
	//    - A queue size.
	//    - Zero on error.
	//
	// Notes:
	//    - Thread-safe.
	//
	virtual int get_generic_stream_queue_size() = 0;

	//
	// Gets information about buffer queue for generic stream.
	//
	// Parameters:
	//    - stream_handle - a handle to generic stream.
	//
	// Returns:
	//    - A free buffer count.
	//    - Zero on error.
	//
	// Notes:
	//    - Thread-safe.
	//
	virtual int get_generic_stream_free_buffer_count(
		GenericStreamHandle stream_handle) = 0;

	//
	// Enqueues data for generic stream.
	//
	// Parameters:
	//    - stream_handle - a handle to generic stream.
	//    - buffer - a buffer with data.
	//               A size of the data must match passed one on open.
	//
	// Returns:
	//    - "true" on sucess.
	//    - "false" otherwise.
	//
	// Notes:
	//    - Thread-safe.
	//    - Expected sample format: 16 bits signed integer, stereo.
	//
	virtual bool enqueue_generic_stream_buffer(
		GenericStreamHandle stream_handle,
		const void* buffer) = 0;

	//
	// Changes a playback state of a generic stream.
	//
	// Parameters:
	//    - stream_handle - a handle to generic stream.
	//    - is_pause - is pause.
	//                 Pass "true" to pause the playback or "false" to resume.
	//
	// Returns:
	//    - "true" on sucess.
	//    - "false" otherwise.
	//
	// Notes:
	//    - Thread-safe.
	//
	virtual bool set_generic_stream_pause(
		GenericStreamHandle stream_handle,
		const bool is_pause) = 0;

	//
	// Gets a playback state of a generic stream.
	//
	// Parameters:
	//    - stream_handle - a handle to generic stream.
	//
	// Returns:
	//    - "true" if a stream is paused.
	//    - "false" if stream is not paused or on error.
	//
	// Notes:
	//    - Thread-safe.
	//
	virtual bool get_generic_stream_pause(
		GenericStreamHandle stream_handle) = 0;

	//
	// Changes a volume of a generic stream.
	//
	// Parameters:
	//    - stream_handle - a handle to generic stream.
	//    - ds_volume - a volume in DirectSound scale [-10000..0].
	//
	// Returns:
	//    - "true" on sucess.
	//    - "false" otherwise.
	//
	// Notes:
	//    - Thread-safe.
	//
	virtual bool set_generic_stream_volume(
		GenericStreamHandle stream_handle,
		const int ds_volume) = 0;

	//
	// Gets a volume of a generic stream.
	//
	// Parameters:
	//    - stream_handle - a handle to generic stream.
	//
	// Returns:
	//    - A volume in DirectSound scale [-10000..0].
	//    - Zero on error.
	//
	// Notes:
	//    - Thread-safe.
	//
	virtual int get_generic_stream_volume(
		GenericStreamHandle stream_handle) = 0;
};

// sound factory abstract base class
// used to generate ILTSoundSys platform dependent interface instances

//! FnEnumSoundSysCallback

typedef bool ( *FnEnumSoundSysCallback )( const char* pcSoundSysName, const char* pcSoundSysDesc, void* pUserData );

//! ILTSoundFactory

class ILTSoundFactory
{
protected:
	ILTSoundFactory( ) {}
	virtual ~ILTSoundFactory( ) {}

public:
	virtual bool EnumSoundSystems( FnEnumSoundSysCallback fnEnumCallback, void* pUserData = NULL );
	virtual bool FillSoundSystems( char* pcSoundSysNames, uint uiMaxStringLen ) = 0;
	virtual ILTSoundSys* MakeSoundSystem( const char* pcSoundSystemName ) = 0;

public:
	static ILTSoundFactory* GetSoundFactory( );

protected:
	static ILTSoundFactory* m_pSoundFactory;
};

#endif	// __ILTSOUND_H_
