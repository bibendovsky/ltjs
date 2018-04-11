#ifndef LTJS_S_DX8_INCLUDED
#define LTJS_S_DX8_INCLUDED


#include <array>
#include <mutex>
#include <thread>
#include <dsound.h>
#include "bibendovsky_spul_file_stream.h"
#include "bibendovsky_spul_riff_reader.h"
#include "bibendovsky_spul_wave_format.h"
#include "ltjs_audio_decoder.h"
#include "iltsound.h"


namespace ul = bibendovsky::spul;


typedef sint16	S16;
typedef uint16	U16;
typedef sint32	S32;
typedef uint32	U32;
typedef lpvoid	PTR;

#define uint	unsigned int
#define ulong	unsigned long

#define MAX_USER_DATA_INDEX		7
#define MAX_USER_PREF_INDEX		255
#define	MAX_WAVE_STREAMS		16
#define STR_BUFFER_SIZE			8192


class CDx8SoundSys;


class WaveFile
{
public:
	WaveFile();

	~WaveFile();

	void Clear();

	bool Open(
		const char* pszFilename,
		const std::uint32_t nFilePos);

	void Close();

	bool Cue();

	bool IsMP3() const;

	std::uint32_t Read(
		std::uint8_t* pbDest,
		const std::uint32_t cbSize);

	std::uint32_t ReadCompressed(
		std::uint8_t* pbDest,
		const std::uint32_t cbSize,
		const std::uint8_t* pCompressedBuffer,
		std::uint8_t* pbDecompressedBuffer);

	std::uint32_t GetDataSize() const;

	std::uint32_t GetMaxBytesCopied() const;

	std::uint32_t GetDuration() const;

	std::uint8_t GetSilenceData() const;

	std::uint32_t SeekFromStart(
		const std::uint32_t nBytes);

	// Integration functions
	bool IsActive() const;

	void SetStream(
		LHSTREAM hStream);

	LHSTREAM GetStream() const;

	const ul::WaveFormatEx& get_format() const;


protected:
	static constexpr auto default_bit_depth = 16;
	static constexpr auto max_channel_count = 2;


	ul::FileStream file_stream_;
	ul::Substream file_substream_;
	ltjs::AudioDecoder audio_decoder_;
	ul::WaveFormatEx wave_format_ex_;

	std::uint32_t m_nDuration; // duration of sound in msec
	std::uint32_t m_nAvgDataRate; // average wave data rate
	std::uint32_t m_nBytesCopied; // Number of uncompressed bytes copied.
	std::uint32_t m_nMaxBytesCopied; // Maximum number of bytes copied, regardless of looping back.

	// Integration data
	LHSTREAM m_hStream;
}; // WaveFile


class I3DObject
{
public:
	I3DObject( );
	virtual ~I3DObject( );
	void Reset( );
	bool Init( );
	void Term( );
	virtual void SetPosition( LTVector& pos );
	virtual void SetVelocity( LTVector& vel );
	virtual void SetOrientation( LTVector& up, LTVector& face );

public:
	LTVector	m_position;
	LTVector	m_velocity;
	LTVector	m_up;
	LTVector	m_face;
	S32			m_userData[ MAX_USER_DATA_INDEX + 1 ];
};


class C3DListener : public I3DObject
{
public:
	C3DListener( );
	virtual ~C3DListener( );
	void Reset( );
	bool Init( HRESULT& hResult, LPDIRECTSOUNDBUFFER pDSPrimaryBuffer );
	void Term( );
	virtual void SetPosition( LTVector& pos );
	virtual void SetVelocity( LTVector& vel );
	virtual void SetOrientation( LTVector& up, LTVector& face );

public:
	LPDIRECTSOUND3DLISTENER8	m_pDS3DListener;
	float						m_fDopplerSetting;
};


class CSample
{
public:
	CSample( );
	virtual ~CSample( );
	void Reset( );
	bool Init( HRESULT& hResult, LPDIRECTSOUND pDS, const uint32 uiNumSamples, 
		const bool b3DBuffer, const ul::WaveFormatEx* pWaveFormat = NULL, const LTSOUNDFILTERDATA* pFilterData = NULL );
	void Term( );
	void Restore( );
	bool Fill( );
	bool UsesLoopingBlock() { return m_bLoopBlock; }

//	===========================================================================
//	Incorporation of DSMStrm* required functionality
public:
	bool IsPlaying( );
	virtual void SetLooping( CDx8SoundSys* pSoundSys, bool bLoop );
	bool IsLooping( ) { return m_bLooping; }

	BOOL GetCurrentPosition( DWORD* pdwPlayPos, DWORD* pdwWritePos );
	virtual BOOL SetCurrentPosition( DWORD dwStartOffset );
	BOOL Play( );
	BOOL Stop( bool bReset = true );
	BOOL Lock( DWORD dwStartOffset, DWORD dwLockAmount, void** ppChunk1, DWORD* pdwChunkSize1, 
		void** ppChunk2, DWORD* pdwChunkSize2, DWORD dwFlags );
	void Unlock( void* pChunk1, DWORD dwChunkSize1, void* pChunk2, DWORD dwChunkSize2 );
	void HandleLoop( CDx8SoundSys* pSoundSys );

public:
	DWORD m_dwPlayFlags;
//	===========================================================================

public:
	ul::WaveFormatEx		m_waveFormat;
	DSBUFFERDESC			m_dsbDesc;
	LPDIRECTSOUNDBUFFER		m_pDSBuffer;
	void*					m_pSoundData;
	uint32					m_uiSoundDataLen;
	S32						m_userData[ MAX_USER_DATA_INDEX + 1 ];
	bool					m_bAllocatedSoundData;
	S32						m_nLoopStart;
	S32						m_nLoopEnd;
	bool					m_bLoopBlock;
	uint32					m_nLastPlayPos;
	LTLink					m_lnkLoopNotify;
	bool					m_bLooping;

	static 	LTLink			m_lstSampleLoopHead;

	int lt_volume_;
	int lt_pan_;
};


class C3DSample : public I3DObject
{
public:
	C3DSample( );
	virtual ~C3DSample( );
	void Reset( );
	bool Init( HRESULT& hResult, LPDIRECTSOUND pDS, const uint32 uiNumSamples, const ul::WaveFormatEx* pWaveFormat, const LTSOUNDFILTERDATA* pFilterData );
	void Term( );
	virtual void SetPosition( LTVector& pos );
	virtual void SetVelocity( LTVector& vel );
	virtual void SetOrientation( LTVector& up, LTVector& face );
	virtual void SetRadiusData( float& fInnerRadius, float& fOuterRadius );
	virtual void GetRadiusData( float* fInnerRadius, float* fInnerRadiusSquared );
	virtual void SetDSMinDist( float& fDSMinDist ) { m_DSMinDist = fDSMinDist; }
	virtual void GetDSMinDist( float* fDSMinDist ) { *fDSMinDist = m_DSMinDist; }
public:
	float					m_innerRadius;
	float					m_outerRadius;
	float					m_innerRadiusSquared;
	float					m_DSMinDist;
	U32						m_status;
	CSample					m_sample;
	LPDIRECTSOUND3DBUFFER8	m_pDS3DBuffer;
};


class CStream : public CSample
{
public:
	CStream( CStream* pPrev, CStream* pNext );
	virtual ~CStream( );
	void HandleUpdate( CDx8SoundSys* pSoundSys );
	uint32 FillBuffer( CDx8SoundSys* pSoundSys );
	virtual void SetLooping( CDx8SoundSys* pSoundSys, bool bLoop );

	// Indicates whether sound is finished playing.
	bool		IsDone( );

	void ReadStreamIntoBuffer( CDx8SoundSys* pSoundSys, BYTE* pBuffer, int32 nBufferSize );

	BOOL SetCurrentPosition( DWORD dwStartOffset );

public:
	streamBufferParams_t m_streamBufferParams;
	uint32		m_uiNextWriteOffset;
	uint32		m_uiBufferSize;
	uint32		m_uiLastPlayPos;
	uint32		m_uiTotalPlayed;
	CStream*	m_pPrev;
	CStream*	m_pNext;
	WaveFile*   m_pWaveFile;
	int8		m_nEventNum;
};


class CDx8SoundSys : public ILTSoundSys
{
public:
	CDx8SoundSys( );
	virtual ~CDx8SoundSys( );

public:
	bool		Init( ) override;
	void		Term( ) override;

public:
	void*		GetDDInterface( const uint uiDDInterfaceId ) override;

public:
	// system wide functions
	void		Lock( void ) override;
	void		Unlock( void ) override;
	S32			Startup( void ) override;
	void		Shutdown( void ) override;
	U32			MsCount( void ) override;
	S32			SetPreference( const U32 uiNumber, const S32 siValue ) override;
	S32			GetPreference( const U32 uiNumber ) override;
	void		MemFreeLock( void* ptr ) override;
	void*		MemAllocLock( const U32 uiSize ) override;
	const char*	LastError( void ) override;

	// digital sound driver functions
	S32			WaveOutOpen( LHDIGDRIVER& phDriver, PHWAVEOUT& pphWaveOut, const S32 siDeviceId, const ul::WaveFormatEx& pWaveFormat ) override;
	void		WaveOutClose( LHDIGDRIVER hDriver ) override;
	void		SetDigitalMasterVolume( LHDIGDRIVER hDig, const S32 siMasterVolume ) override;
	S32			GetDigitalMasterVolume( LHDIGDRIVER hDig ) override;
	S32			DigitalHandleRelease( LHDIGDRIVER hDriver ) override;
	S32			DigitalHandleReacquire( LHDIGDRIVER hDriver ) override;
#ifdef USE_EAX20_HARDWARE_FILTERS
	bool		SetEAX20Filter( const bool bEnable, const LTSOUNDFILTERDATA& pFilterData ) override;
	bool		SupportsEAX20Filter() override;
	virtual	bool		SetEAX20BufferSettings( LH3DSAMPLE h3DSample, const LTSOUNDFILTERDATA& pFilterData ) override;
#endif

	// 3d sound provider functions
	void		Set3DProviderMinBuffers( const U32 uiMinBuffers ) override;
	S32			Open3DProvider( LHPROVIDER hLib ) override;
	void		Close3DProvider( LHPROVIDER hLib ) override;
	void		Set3DProviderPreference( LHPROVIDER hLib, const char* sName, const void* pVal ) override;
	void		Get3DProviderAttribute( LHPROVIDER hLib, const char* sName, void* pVal ) override;
	S32			Enumerate3DProviders( LHPROENUM& phNext, LHPROVIDER& phDest, const char*& psName) override;

	// 3d listener functions
	LH3DPOBJECT	Open3DListener( LHPROVIDER hLib ) override;
	void		Close3DListener( LH3DPOBJECT hListener ) override;
	void		SetListenerDoppler( LH3DPOBJECT hListener, const float fDoppler ) override;
	void		CommitDeferred() override;


	// 3d sound object functions
	void		Set3DPosition( LH3DPOBJECT hObj, const float fX, const float fY, const float fZ) override;
	void		Set3DVelocityVector( LH3DPOBJECT hObj, const float fDX_per_ms, const float fDY_per_ms, const float fDZ_per_ms ) override;
	void		Set3DOrientation( LH3DPOBJECT hObj, const float fX_face, const float fY_face, const float fZ_face, const float fX_up, const float fY_up, const float fZ_up ) override;
	void		Set3DUserData( LH3DPOBJECT hObj, const U32 uiIndex, const S32 siValue ) override;
	void		Get3DPosition( LH3DPOBJECT hObj, float& pfX, float& pfY, float& pfZ) override;
	void		Get3DVelocity( LH3DPOBJECT hObj, float& pfDX_per_ms, float& pfDY_per_ms, float& pfDZ_per_ms ) override;
	void		Get3DOrientation( LH3DPOBJECT hObj, float& pfX_face, float& pfY_face, float& pfZ_face, float& pfX_up, float& pfY_up, float& pfZ_up ) override;
	S32			Get3DUserData( LH3DPOBJECT hObj, const U32 uiIndex) override;

	// 3d sound sample functions
	LH3DSAMPLE	Allocate3DSampleHandle( LHPROVIDER hLib ) override;
	void		Release3DSampleHandle( LH3DSAMPLE hS ) override;
	void		Stop3DSample( LH3DSAMPLE hS ) override;
	void		Start3DSample( LH3DSAMPLE hS ) override;
	void		Resume3DSample( LH3DSAMPLE hS ) override;
	void		End3DSample( LH3DSAMPLE hS ) override;
	S32			Init3DSampleFromAddress( LH3DSAMPLE hS, const void* pStart, const U32 uiLen, const ul::WaveFormatEx& pWaveFormat, const S32 nPitchShift, const LTSOUNDFILTERDATA* pFilterData ) override;
	S32			Init3DSampleFromFile( LH3DSAMPLE hS, const void* pFile_image, const S32 siBlock, const S32 siPlaybackRate, const LTSOUNDFILTERDATA* pFilterData ) override;
	S32			Get3DSampleVolume( LH3DSAMPLE hS ) override;
	void		Set3DSampleVolume( LH3DSAMPLE hS, const S32 siVolume ) override;
	uint32		Get3DSampleStatus( LH3DSAMPLE hS ) override;
	void		Set3DSampleMsPosition( LHSAMPLE hS, const sint32 siMilliseconds ) override;
	S32			Set3DSampleInfo( LH3DSAMPLE hS, const LTSOUNDINFO& pInfo ) override;
	void		Set3DSampleDistances( LH3DSAMPLE hS, const float fMax_dist, const float fMin_dist ) override;
	void		Set3DSamplePreference( LH3DSAMPLE hSample, const char* sName, const void* pVal ) override;
	void		Set3DSampleLoopBlock( LH3DSAMPLE hS, const S32 siLoop_start_offset, const S32 siLoop_end_offset, const bool bEnable ) override;
	void		Set3DSampleLoop( LH3DSAMPLE hS, const bool bLoop ) override;
	void		Set3DSampleObstruction( LH3DSAMPLE hS, const float fObstruction ) override;
	float		Get3DSampleObstruction( LH3DSAMPLE hS ) override;
	void		Set3DSampleOcclusion( LH3DSAMPLE hS, const float fOcclusion ) override;
	float		Get3DSampleOcclusion( LH3DSAMPLE hS ) override;

	// 2d sound sample functions
	LHSAMPLE	AllocateSampleHandle( LHDIGDRIVER hDig ) override;
	void		ReleaseSampleHandle( LHSAMPLE hS ) override;
	void		InitSample( LHSAMPLE hS ) override;
	void		StopSample( LHSAMPLE hS ) override;
	void		StartSample( LHSAMPLE hS ) override;
	void		ResumeSample( LHSAMPLE hS ) override;
	void		EndSample( LHSAMPLE hS ) override;
	void		SetSampleVolume( LHSAMPLE hS, const S32 siVolume ) override;
	void		SetSamplePan( LHSAMPLE hS, const S32 siPan ) override;
	S32			GetSampleVolume( LHSAMPLE hS ) override;
	S32			GetSamplePan( LHSAMPLE hS ) override;
	void		SetSampleUserData( LHSAMPLE hS, const U32 uiIndex, const S32 siValue ) override;
	void		GetDirectSoundInfo( LHSAMPLE hS, PTDIRECTSOUND& ppDS, PTDIRECTSOUNDBUFFER& ppDSB ) override;
	void		SetSampleReverb( LHSAMPLE hS, const float fReverb_level, const float fReverb_reflect_time, const float fReverb_decay_time ) override;
	S32			InitSampleFromAddress( LHSAMPLE hS, const void* pStart, const U32 uiLen, const ul::WaveFormatEx& pWaveFormat, const S32 siPlaybackRate, const LTSOUNDFILTERDATA* pFilterData ) override;
	S32			InitSampleFromFile( LHSAMPLE hS, const void* pFile_image, const S32 siBlock, const S32 siPlaybackRate, const LTSOUNDFILTERDATA* pFilterData ) override;
	void		SetSampleLoopBlock( LHSAMPLE hS, const S32 siLoop_start_offset, const S32 siLoop_end_offset, const bool bEnable ) override;
	void		SetSampleLoop( LHSAMPLE hS, const bool bLoop ) override;
	void		SetSampleMsPosition( LHSAMPLE hS, const S32 siMilliseconds ) override;
	S32			GetSampleUserData( LHSAMPLE hS, const U32 uiIndex ) override;
	uint32		GetSampleStatus( LHSAMPLE hS ) override;


	// old 2d sound stream functions
	LHSTREAM	OpenStream( const char* sFilename, const uint32 nFilePos, LHDIGDRIVER hDig, const char* sStream, const sint32 siStream_mem ) override;
	void		SetStreamLoop( LHSTREAM hStream, const bool bLoop ) override;
	void		SetStreamPlaybackRate( LHSTREAM hStream, const S32 siRate ) override;
	void		SetStreamMsPosition( LHSTREAM hStream, const S32 siMilliseconds ) override;
	void		SetStreamUserData( LHSTREAM hS, const U32 uiIndex, const S32 siValue) override;
	S32			GetStreamUserData( LHSTREAM hS, const U32 uiIndex) override;

	// new 2d sound stream functions
	LHSTREAM	OpenStream( streamBufferParams_t* pStreamBufferParams, WaveFile* pWaveFile, uint8 nEventNum );
	void		CloseStream( LHSTREAM hStream ) override;
	void		StartStream( LHSTREAM hStream ) override;
	void		PauseStream( LHSTREAM hStream, const sint32 siOnOff ) override;
	void		ResetStream( LHSTREAM hStream ) override;
	void		SetStreamVolume( LHSTREAM hStream, const sint32 siVolume ) override;
	void		SetStreamPan( LHSTREAM hStream, const sint32 siPan ) override;
	sint32		GetStreamVolume( LHSTREAM hStream ) override;
	sint32		GetStreamPan( LHSTREAM hStream ) override;
	uint32		GetStreamStatus( LHSTREAM hStream ) override;
	sint32		GetStreamBufferParam( LHSTREAM hStream, const uint32 uiParam ) override;
	void		ClearStreamBuffer( LHSTREAM hStream, const bool bClearStreamDataQueue = true ) override;

	// wave file decompression functons
	S32			DecompressADPCM( const LTSOUNDINFO& pInfo, void*& ppOutData, U32& puiOutSize ) override;
	S32			DecompressASI( const void* pInData, const U32 uiInSize, const char* sFilename_ext, void*& ppWav, U32& puiWavSize, LTLENGTHYCB fnCallback ) override;
	UINT				ReadStream( WaveFile* pStream, BYTE* pOutBuffer, const int nSize );

	bool		HasOnBoardMemory( ) override;

//	===========================================================================
//	Incorporation of DSMStrm* required functionality
public:
	CSample*			CreateBuffer( ul::WaveFormatEx* pWaveFormat, DWORD dwBufferSize, DWORD dwFlags );
	void				DestroyBuffer( CSample* pSoundBuffer );
//	===========================================================================

	// Gets the ticks spent in sound thread.
	uint32				GetThreadedSoundTicks( ) override;

	bool SetSampleNotify( CSample* pSample, bool bEnable );


private:
	void Reset( );
#ifdef USE_EAX20_HARDWARE_FILTERS
	void InitEAX20Filtering(void);
#endif
	bool SupportsDS3DHardware(void);

	// Gets the propertyset interface to do eax operations.
	bool GetPropertySetForEAX( );

public:
	static CDx8SoundSys m_Dx8SoundSys;
	static const char*	m_pcDx8SoundSysDesc;

public:
	bool				m_bLocked;
	LPDIRECTSOUND8		m_pDirectSound;
	LPDIRECTSOUNDBUFFER m_pDSPrimaryBuffer;
	LPKSPROPERTYSET     m_pKSPropertySet;
	ul::WaveFormatEx	m_waveFormat;
	DSCAPS				m_dscaps;
	HRESULT				m_hResult;
	const char*			m_pcLastError;
	sint32				m_iCur3DProvider;
	DWORD				m_dwMinHardwareBuffers;

#ifdef USE_EAX20_HARDWARE_FILTERS
	bool				m_bSupportsEAX20Filtering;
#endif

public:
	S32					m_userPrefs[ MAX_USER_PREF_INDEX + 1 ];

public:
	CStream*			m_pStreams;

	WaveFile			m_WaveStream[MAX_WAVE_STREAMS];
	
private:
	using Thread = std::thread;

	using MtMutex = std::mutex;
	using MtRMutex = std::recursive_mutex;

	using MtLockGuard = std::lock_guard<MtMutex>;
	using MtRLockGuard = std::lock_guard<MtRMutex>;


	void				Thread_Func();
	Thread				m_cThread_Handle;

	MtRMutex			m_cCS_SoundThread;

	bool is_mt_sample_loop_active_;
	bool is_mt_streaming_active_;
	bool is_mt_shutdown_;

	MtMutex				m_cCS_ThreadedTickCounts;
	uint32				m_nThreadedTickCounts;

	int lt_master_volume_;
	ltjs::AudioDecoder audio_decoder_;
};


#endif // LTJS_S_DX8_INCLUDED
