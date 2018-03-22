#ifndef __S_DX8_H__
#define __S_DX8_H__


#include "stdafx.h"

// ---
#include "dsound.h"
#include "mmsystem.h"
#include "mmreg.h"
#include "msacm.h"
// ---

#include "bibendovsky_spul_file_stream.h"
#include "bibendovsky_spul_riff_reader.h"
#include "bibendovsky_spul_wave_format.h"
#include "ltjs_audio_decoder.h"
#include "iltsound.h"

#include "winsync.h"


namespace ul = bibendovsky::spul;


enum DS3DAlgo
{
	DS3D_NO_VIRTUALIZATION,
	DS3D_HRTF_LIGHT,
	DS3D_HRTF_FULL,
	DS3D_TOTAL_NUM
};

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
//  we can loop as many samples as we can create
// events to wait for
#define MAX_LOOPED_SAMPLES		(MAXIMUM_WAIT_OBJECTS - 1)

#define STR_BUFFER_SIZE			8192

//! WaveFile

// Constants
#ifndef SUCCESS
#define SUCCESS TRUE        // Error returns for all member functions
#define FAILURE FALSE
#endif // SUCCESS

#define DOUT	(void)//OutputDebugString

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

	bool IsMP3() const
	{
		return m_bMP3Compressed;
	}

	std::uint32_t Read(
		std::uint8_t* pbDest,
		const std::uint32_t cbSize);

	std::uint32_t ReadCompressed(
		std::uint8_t* pbDest,
		const std::uint32_t cbSize,
		const std::uint8_t* pCompressedBuffer,
		std::uint8_t* pbDecompressedBuffer);

	std::uint32_t GetAvgDataRate() const
	{
		return m_nAvgDataRate;
	}

	std::uint32_t GetDataSize() const
	{
		return m_nDataSize;
	}

	std::uint32_t GetNumBytesRead() const
	{
		return m_nBytesRead;
	}

	std::uint32_t GetNumBytesCopied() const
	{
		return m_nBytesCopied;
	}

	std::uint32_t GetMaxBytesRead() const
	{
		return m_nMaxBytesRead;
	}

	std::uint32_t GetMaxBytesCopied() const
	{
		return m_nMaxBytesCopied;
	}

	std::uint32_t GetDuration() const
	{
		return m_nDuration;
	}

	std::uint8_t GetSilenceData() const;

	void SetBytesPerSample(
		const std::uint32_t nBytesPerSample)
	{
		m_nBytesPerSample = nBytesPerSample;
	}

	std::uint32_t SeekFromStart(
		const std::uint32_t nBytes);

	std::uint32_t SeekFromStartCompressed(
		const std::uint32_t nBytes);

	// Integration functions
	bool IsActive() const
	{
		return m_hStream != nullptr;
	}

	void SetStream(
		LHSTREAM hStream)
	{
		m_hStream = hStream;
	}

	LHSTREAM GetStream() const
	{
		return m_hStream;
	}

	// decompression related functions
	HACMSTREAM GetAcmStream() const
	{
		return m_hAcmStream;
	}

	void SetAcmStream(
		HACMSTREAM hAcmStream)
	{
		m_hAcmStream = hAcmStream;
	}

	ACMSTREAMHEADER* GetAcmStreamHeader()
	{
		return &m_acmStreamHeader;
	}

	void SetSrcBufferSize(
		std::uint32_t ulSrcBufferSize)
	{
		m_ulSrcBufferSize = ulSrcBufferSize;
	}

	ul::WaveFormatEx* m_pwfmt;


protected:
	ul::FileStream file_stream_;
	ul::RiffReader riff_reader_;
	ul::RiffReader::Chunk data_chunk_;

	std::uint32_t m_nDuration; // duration of sound in msec
	std::uint32_t m_nBlockAlign; // wave data block alignment spec
	std::uint32_t m_nAvgDataRate; // average wave data rate
	std::uint32_t m_nDataSize; // size of data chunk
	std::uint32_t m_nBytesRead; // Number of uncompressed bytes read.
	std::uint32_t m_nMaxBytesRead; // Maximum number of bytes read, regardless of looping back.
	std::uint32_t m_nBytesCopied; // Number of uncompressed bytes copied.
	std::uint32_t m_nMaxBytesCopied; // Maximum number of bytes copied, regardless of looping back.
	std::uint32_t m_nBytesPerSample; // number of bytes in each sample

	// Integration data
	LHSTREAM m_hStream;

	// for files that need to be decompressed
	bool m_bMP3Compressed;
	ACMSTREAMHEADER m_acmStreamHeader;
	HACMSTREAM m_hAcmStream;
	std::uint32_t m_ulSrcBufferSize;
	std::uint32_t m_nRemainderBytes;
	std::uint32_t m_nRemainderOffset;
}; // WaveFile


bool ParseWaveFile(
	void* pWaveFileBlock,
	void*& rpWaveFormat,
	std::uint32_t& ruiWaveFormatSize,
	void*& rpSampleData,
	std::uint32_t& ruiSampleDataSize);

//! I3DObject

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

//! C3DListener

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


//! CSample

class CSample
{
public:
	CSample( );
	virtual ~CSample( );
	void Reset( );
	bool Init( HRESULT& hResult, LPDIRECTSOUND pDS, uint32 uiNumSamples, 
		bool b3DBuffer, ul::WaveFormatEx* pWaveFormat = NULL, LTSOUNDFILTERDATA* pFilterData = NULL );
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
};

//! C3DSample

class C3DSample : public I3DObject
{
public:
	C3DSample( );
	virtual ~C3DSample( );
	void Reset( );
	bool Init( HRESULT& hResult, LPDIRECTSOUND pDS, uint32 uiNumSamples, ul::WaveFormatEx* pWaveFormat, LTSOUNDFILTERDATA* pFilterData );
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


//! CStream

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


//! CFileStream

class CFileStream
{
public:
	


};

//! CDx8SoundSys

class CDx8SoundSys : public ILTSoundSys
{
public:
	CDx8SoundSys( );
	virtual ~CDx8SoundSys( );

public:
	virtual bool		Init( );
	virtual void		Term( );

public:
	virtual void*		GetDDInterface( uint uiDDInterfaceId );

public:
	// system wide functions
	virtual void		Lock( void );
	virtual void		Unlock( void );
	virtual S32			Startup( void );
	virtual void		Shutdown( void );
	virtual U32			MsCount( void );
	virtual S32			SetPreference( U32 uiNumber, S32 siValue );
	virtual S32			GetPreference( U32 uiNumber );
	virtual void		MemFreeLock( void* ptr );
	virtual void*		MemAllocLock( U32 uiSize );
	virtual char*		LastError( void );

	// digital sound driver functions
	virtual S32			WaveOutOpen( LHDIGDRIVER* phDriver, PHWAVEOUT* pphWaveOut, S32 siDeviceId, ul::WaveFormat* pWaveFormat );
	virtual void		WaveOutClose( LHDIGDRIVER hDriver );
	virtual void		SetDigitalMasterVolume( LHDIGDRIVER hDig, S32 siMasterVolume );
	virtual S32			GetDigitalMasterVolume( LHDIGDRIVER hDig );
	virtual S32			DigitalHandleRelease( LHDIGDRIVER hDriver );
	virtual S32			DigitalHandleReacquire( LHDIGDRIVER hDriver );
#ifdef USE_EAX20_HARDWARE_FILTERS
	virtual bool		SetEAX20Filter( bool bEnable, LTSOUNDFILTERDATA* pFilterData );
	virtual bool		SupportsEAX20Filter();
	virtual	bool		SetEAX20BufferSettings( LH3DSAMPLE h3DSample, LTSOUNDFILTERDATA* pFilterData );
#endif

	// 3d sound provider functions
	virtual void		Set3DProviderMinBuffers( U32 uiMinBuffers );
	virtual S32			Open3DProvider( LHPROVIDER hLib );
	virtual void		Close3DProvider( LHPROVIDER hLib );
	virtual void		Set3DProviderPreference( LHPROVIDER hLib, char* sName, void* pVal );
	virtual void		Get3DProviderAttribute( LHPROVIDER hLib, char* sName, void* pVal );
	virtual S32			Enumerate3DProviders( LHPROENUM* phNext, LHPROVIDER* phDest, char** psName);

	// 3d listener functions
	virtual LH3DPOBJECT	Open3DListener( LHPROVIDER hLib );
	virtual void		Close3DListener( LH3DPOBJECT hListener );
	virtual void		SetListenerDoppler( LH3DPOBJECT hListener, float fDoppler );
	virtual void		CommitDeferred();


	// 3d sound object functions
	virtual void		Set3DPosition( LH3DPOBJECT hObj, float fX, float fY, float fZ);
	virtual void		Set3DVelocityVector( LH3DPOBJECT hObj, float fDX_per_ms, float fDY_per_ms, float fDZ_per_ms );
	virtual void		Set3DOrientation( LH3DPOBJECT hObj, float fX_face, float fY_face, float fZ_face, float fX_up, float fY_up, float fZ_up );
	virtual void		Set3DUserData( LH3DPOBJECT hObj, U32 uiIndex, S32 siValue );
	virtual void		Get3DPosition( LH3DPOBJECT hObj, float* pfX, float* pfY, float* pfZ);
	virtual void		Get3DVelocity( LH3DPOBJECT hObj, float* pfDX_per_ms, float* pfDY_per_ms, float* pfDZ_per_ms );
	virtual void		Get3DOrientation( LH3DPOBJECT hObj, float* pfX_face, float* pfY_face, float* pfZ_face, float* pfX_up, float* pfY_up, float* pfZ_up );
	virtual S32			Get3DUserData( LH3DPOBJECT hObj, U32 uiIndex);

	// 3d sound sample functions
	virtual LH3DSAMPLE	Allocate3DSampleHandle( LHPROVIDER hLib );
	virtual void		Release3DSampleHandle( LH3DSAMPLE hS );
	virtual void		Stop3DSample( LH3DSAMPLE hS );
	virtual void		Start3DSample( LH3DSAMPLE hS );
	virtual void		Resume3DSample( LH3DSAMPLE hS );
	virtual void		End3DSample( LH3DSAMPLE hS );
	virtual S32			Init3DSampleFromAddress( LH3DSAMPLE hS, void* pStart, U32 uiLen, ul::WaveFormatEx* pWaveFormat, S32 nPitchShift, LTSOUNDFILTERDATA* pFilterData );
	virtual S32			Init3DSampleFromFile( LH3DSAMPLE hS, void* pFile_image, S32 siBlock, S32 siPlaybackRate, LTSOUNDFILTERDATA* pFilterData );
	virtual S32			Get3DSampleVolume( LH3DSAMPLE hS );
	virtual void		Set3DSampleVolume( LH3DSAMPLE hS, S32 siVolume );
	virtual uint32		Get3DSampleStatus( LH3DSAMPLE hS );
	virtual void		Set3DSampleMsPosition( LHSAMPLE hS, sint32 siMilliseconds );
	virtual S32			Set3DSampleInfo( LH3DSAMPLE hS, LTSOUNDINFO* pInfo );
	virtual void		Set3DSampleDistances( LH3DSAMPLE hS, float fMax_dist, float fMin_dist );
	virtual void		Set3DSamplePreference( LH3DSAMPLE hSample, char* sName, void* pVal );
	virtual void		Set3DSampleLoopBlock( LH3DSAMPLE hS, S32 siLoop_start_offset, S32 siLoop_end_offset, bool bEnable );
	virtual void		Set3DSampleLoop( LH3DSAMPLE hS, bool bLoop );
	virtual void		Set3DSampleObstruction( LH3DSAMPLE hS, float fObstruction );
	virtual float		Get3DSampleObstruction( LH3DSAMPLE hS );
	virtual void		Set3DSampleOcclusion( LH3DSAMPLE hS, float fOcclusion );
	virtual float		Get3DSampleOcclusion( LH3DSAMPLE hS );

	// 2d sound sample functions
	virtual LHSAMPLE	AllocateSampleHandle( LHDIGDRIVER hDig );
	virtual void		ReleaseSampleHandle( LHSAMPLE hS );
	virtual void		InitSample( LHSAMPLE hS );
	virtual void		StopSample( LHSAMPLE hS );
	virtual void		StartSample( LHSAMPLE hS );
	virtual void		ResumeSample( LHSAMPLE hS );
	virtual void		EndSample( LHSAMPLE hS );
	virtual void		SetSampleVolume( LHSAMPLE hS, S32 siVolume );
	virtual void		SetSamplePan( LHSAMPLE hS, S32 siPan );
	virtual S32			GetSampleVolume( LHSAMPLE hS );
	virtual S32			GetSamplePan( LHSAMPLE hS );
	virtual void		SetSampleUserData( LHSAMPLE hS, U32 uiIndex, S32 siValue );
	virtual void		GetDirectSoundInfo( LHSAMPLE hS, PTDIRECTSOUND* ppDS, PTDIRECTSOUNDBUFFER* ppDSB );
	virtual void		SetSampleReverb( LHSAMPLE hS, float fReverb_level, float fReverb_reflect_time, float fReverb_decay_time );
	virtual S32			InitSampleFromAddress( LHSAMPLE hS, void* pStart, U32 uiLen, ul::WaveFormatEx* pWaveFormat, S32 siPlaybackRate, LTSOUNDFILTERDATA* pFilterData );
	virtual S32			InitSampleFromFile( LHSAMPLE hS, void* pFile_image, S32 siBlock, S32 siPlaybackRate, LTSOUNDFILTERDATA* pFilterData );
	virtual void		SetSampleLoopBlock( LHSAMPLE hS, S32 siLoop_start_offset, S32 siLoop_end_offset, bool bEnable );
	virtual void		SetSampleLoop( LHSAMPLE hS, bool bLoop );
	virtual void		SetSampleMsPosition( LHSAMPLE hS, S32 siMilliseconds );
	virtual S32			GetSampleUserData( LHSAMPLE hS, U32 uiIndex );
	virtual uint32		GetSampleStatus( LHSAMPLE hS );


	// old 2d sound stream functions
	virtual LHSTREAM	OpenStream( char* sFilename, uint32 nFilePos, LHDIGDRIVER hDig, char* sStream, sint32 siStream_mem );
	virtual void		SetStreamLoop( LHSTREAM hStream, bool bLoop );
	virtual void		SetStreamPlaybackRate( LHSTREAM hStream, S32 siRate );
	virtual void		SetStreamMsPosition( LHSTREAM hStream, S32 siMilliseconds );
	virtual void		SetStreamUserData( LHSTREAM hS, U32 uiIndex, S32 siValue);
	virtual S32			GetStreamUserData( LHSTREAM hS, U32 uiIndex);

	// new 2d sound stream functions
	virtual LHSTREAM	OpenStream( streamBufferParams_t* pStreamBufferParams, WaveFile* pWaveFile, uint8 nEventNum );
	virtual void		CloseStream( LHSTREAM hStream );
	virtual void		StartStream( LHSTREAM hStream );
	virtual void		PauseStream( LHSTREAM hStream, sint32 siOnOff );
	virtual void		ResetStream( LHSTREAM hStream );
	virtual void		SetStreamVolume( LHSTREAM hStream, sint32 siVolume );
	virtual void		SetStreamPan( LHSTREAM hStream, sint32 siPan );
	virtual sint32		GetStreamVolume( LHSTREAM hStream );
	virtual sint32		GetStreamPan( LHSTREAM hStream );
	virtual uint32		GetStreamStatus( LHSTREAM hStream );
	virtual sint32		GetStreamBufferParam( LHSTREAM hStream, uint32 uiParam );
	virtual void		ClearStreamBuffer( LHSTREAM hStream, bool bClearStreamDataQueue = true );

	// wave file decompression functons
	virtual S32			DecompressADPCM( LTSOUNDINFO* pInfo, void** ppOutData, U32* puiOutSize );
	virtual S32			DecompressASI( void* pInData, U32 uiInSize, char* sFilename_ext, void** ppWav, U32* puiWavSize, LTLENGTHYCB fnCallback );
	UINT				ReadStream( WaveFile* pStream, BYTE* pOutBuffer, int nSize );
	BYTE*				GetCompressedBuffer() { return m_pCompressedBuffer; }
	BYTE*				GetDecompressedBuffer() { return m_pDecompressedBuffer; }

	virtual bool		HasOnBoardMemory( );

//	===========================================================================
//	Incorporation of DSMStrm* required functionality
public:
	CSample*			CreateBuffer( ul::WaveFormatEx* pWaveFormat, DWORD dwBufferSize, DWORD dwFlags );
	void				DestroyBuffer( CSample* pSoundBuffer );
//	===========================================================================

	// Gets the ticks spent in sound thread.
	uint32				GetThreadedSoundTicks( );

	bool SetSampleNotify( CSample* pSample, bool bEnable );


private:
	void Reset( );
#ifdef USE_EAX20_HARDWARE_FILTERS
	void InitEAX20Filtering(void);
#endif
	bool SupportsDS3DHardware(void);
	bool FillStreamBuffer( LPDIRECTSOUNDBUFFER pDSB, WaveFile* pStream, int nSize );

	// Gets the propertyset interface to do eax operations.
	bool GetPropertySetForEAX( );

public:
	static CDx8SoundSys m_Dx8SoundSys;
	static char*		m_pcDx8SoundSysDesc;

public:
	bool				m_bCOMInitialized;
	bool				m_bLocked;
	LPDIRECTSOUND8		m_pDirectSound;
	LPDIRECTSOUNDBUFFER m_pDSPrimaryBuffer;
	LPKSPROPERTYSET     m_pKSPropertySet;
	ul::WaveFormatEx	m_waveFormat;
	DSCAPS				m_dscaps;
	HRESULT				m_hResult;
	char*				m_pcLastError;
	HACMDRIVERID		m_hAcmPCMDriverId;
	HACMDRIVER			m_hAcmPCMDriver;
	HACMDRIVERID		m_hAcmADPCMDriverId;
	HACMDRIVER			m_hAcmADPCMDriver;
	HACMDRIVERID		m_hAcmMP3DriverId;
	HACMDRIVER			m_hAcmMP3Driver;
	sint32				m_iCur3DProvider;
	DWORD				m_dwMinHardwareBuffers;

#ifdef USE_EAX20_HARDWARE_FILTERS
	bool				m_bSupportsEAX20Filtering;
#endif

public:
	S32					m_userPrefs[ MAX_USER_PREF_INDEX + 1 ];

public:
	CStream*			m_pStreams;

	// for decompressing streams
	BYTE*				m_pCompressedBuffer;
	BYTE*				m_pDecompressedBuffer;
	
	WaveFile			m_WaveStream[MAX_WAVE_STREAMS];
	
private:

#ifdef __MINGW32__
    static unsigned long __attribute__((stdcall)) ThreadBootstrap(void *pUserData);
#else
	static unsigned long _stdcall ThreadBootstrap(void *pUserData);
#endif

	uint32				Thread_Func();
	HANDLE				m_cThread_Handle;

	CWinSync_CS			m_cCS_SoundThread;

	CWinSync_Event		m_cEvent_SampleLoopActive;
	CWinSync_Event		m_cEvent_StreamingActive;
	CWinSync_Event		m_cEvent_Shutdown;

	CWinSync_CS			m_cCS_ThreadedTickCounts;
	uint32				m_nThreadedTickCounts;

	ltjs::AudioDecoder audio_decoder_;


	static int extract_wave_size(
		const void* raw_data);
};

#endif 
