#include "s_dx8.h"
#include <array>
#include <chrono>
#include <functional>
#include <memory>
#include "bibendovsky_spul_memory_stream.h"
#include "bibendovsky_spul_scope_guard.h"
#include "eax.h"
#include "ltjs_audio_utils.h"


//	===========================================================================
// For debug logging

#include "stdio.h"
#include "syscounter.h"

#include <vector>

CDx8SoundSys* g_pSoundSys = NULL;

FILE* g_pLogFile = NULL;

//#define _LOG_OUTPUT
#ifdef _DEBUG
#ifdef _LOG_OUTPUT
#define LOG
#endif
#endif

#ifdef	LOG

#define LOG_OPEN		g_pLogFile = fopen( "snddrv.log", "wa" )
#define LOG_CLOSE		if( g_pLogFile != LTNULL ) fclose( g_pLogFile ); g_pLogFile = LTNULL
#define LOG_WRITE		if( g_pLogFile != LTNULL ) fprintf

#else

#define LOG_OPEN
#define LOG_CLOSE
#define LOG_WRITE

#endif	// LOG

//=======================================================================================

#define REPORT_DS_ERROR( hr, err )	\
	if( hr == err )					\
	{								\
		return #err;				\
	}								\


static const char* ConvertDSErrorToString( HRESULT hr )
{
	REPORT_DS_ERROR( hr, DS_OK )
	REPORT_DS_ERROR( hr, DSERR_ALLOCATED )
	REPORT_DS_ERROR( hr, DSERR_ALREADYINITIALIZED )
	REPORT_DS_ERROR( hr, DSERR_BADFORMAT )
	REPORT_DS_ERROR( hr, DSERR_BUFFERLOST )
	REPORT_DS_ERROR( hr, DSERR_CONTROLUNAVAIL )
	REPORT_DS_ERROR( hr, DSERR_GENERIC )
	REPORT_DS_ERROR( hr, DSERR_INVALIDCALL )
	REPORT_DS_ERROR( hr, DSERR_INVALIDPARAM )
	REPORT_DS_ERROR( hr, DSERR_NOAGGREGATION )
	REPORT_DS_ERROR( hr, DSERR_NODRIVER )
	REPORT_DS_ERROR( hr, DSERR_NOINTERFACE )
	REPORT_DS_ERROR( hr, DSERR_OTHERAPPHASPRIO )
	REPORT_DS_ERROR( hr, DSERR_OUTOFMEMORY )
	REPORT_DS_ERROR( hr, DSERR_PRIOLEVELNEEDED )
	REPORT_DS_ERROR( hr, DSERR_UNINITIALIZED )
	REPORT_DS_ERROR( hr, DSERR_UNSUPPORTED )
	return "unrecognized error";
}


// 3D provider info
#define SOUND3DPROVIDERNAME_DS3D_HARDWARE       "DirectSound Hardware"
#define SOUND3DPROVIDERNAME_DS3D_SOFTWARE       "DirectSound Software"
#define SOUND3DPROVIDERNAME_DS3D_DEFAULT		"DirectSound Default"

enum
{
	PROVIDER_DS3D_SOFTWARE = 0,
	PROVIDER_DS3D_HARDWARE,
	PROVIDER_DS3D_DEFAULT,
	NUM_PROVIDERS
};

// no hard limit on software mixed 3D, but
// want to put some upper limit here
#define MAX_3D_SOFTWARE_SAMPLES	255

#ifdef USE_EAX20_HARDWARE_FILTERS
// hack to let filter information be passed between sound system and samples for EAX sound buffers
static sint32 g_iCurrentEAXDirectSetting;
#endif

//! WaveFile


// WaveFile class implementation
//
////////////////////////////////////////////////////////////

WaveFile::WaveFile()
{
	Clear();
}

WaveFile::~WaveFile()
{
	Close();
}

void WaveFile::Clear()
{
	// Init data members
	wave_format_ex_ = {};
	m_nAvgDataRate = 0;
	m_nBytesCopied = 0;
	m_nMaxBytesCopied = 0;
	m_nDuration = 0;
	m_hStream = nullptr;
}

void WaveFile::Close()
{
	m_hStream = nullptr;

	wave_format_ex_ = {};

	audio_decoder_.close();
	file_substream_.close();
	file_stream_.close();

	Clear();
}

bool WaveFile::Open(
	const char* pszFilename,
	const std::uint32_t nFilePos)
{
	const auto open_result = file_stream_.open(pszFilename, ul::Stream::OpenMode::read);

	if (!open_result)
	{
		return false;
	}

	auto is_succeed = false;

	auto cleaner = ul::ScopeGuard{
		[&]()
		{
			if (!is_succeed)
			{
				Close();
			}
		}
	};

	const auto set_position_result = file_stream_.set_position(nFilePos);

	if (!set_position_result)
	{
		return false;
	}


	using RiffBuffer = std::array<std::uint32_t, 3>;
	const auto riff_buffer_size = static_cast<int>(sizeof(RiffBuffer));
	auto riff_buffer = RiffBuffer{};

	if (file_stream_.read(riff_buffer.data(), riff_buffer_size) != riff_buffer_size)
	{
		return false;
	}

	const auto wave_size = ltjs::AudioUtils::extract_wave_size(riff_buffer.data());

	if (wave_size == 0)
	{
		return false;
	}

	if (!file_substream_.open(&file_stream_, nFilePos, wave_size))
	{
		return false;
	}

	if (!audio_decoder_.open(&file_substream_))
	{
		return false;
	}

	if (audio_decoder_.get_bit_depth() != default_bit_depth)
	{
		return false;
	}

	if (audio_decoder_.get_channel_count() > max_channel_count)
	{
		return false;
	}

	wave_format_ex_ = audio_decoder_.get_wave_format_ex();

	// Init some member data from format chunk
	m_nAvgDataRate = wave_format_ex_.avg_bytes_per_sec_;

	// Cue for streaming
	if(!Cue())
	{
		return false;
	}

	m_nDuration = (audio_decoder_.get_data_size() * 1000) / m_nAvgDataRate;

	is_succeed = true;

	return true;
}

bool WaveFile::Cue()
{
	m_nBytesCopied = 0;

	if (!audio_decoder_.rewind())
	{
		return false;
	}

	return true;
}

bool WaveFile::IsMP3() const
{
	return audio_decoder_.is_mp3();
}

// Read
//
// Returns number of bytes actually read.
// On error, returns 0.
//
std::uint32_t WaveFile::Read(
	std::uint8_t* pbDest,
	const std::uint32_t cbSize)
{
	if (cbSize == 0)
	{
		return 0;
	}

	auto result = 0;

	if (pbDest)
	{
		const auto read_result = audio_decoder_.decode(pbDest, static_cast<int>(cbSize));

		if (read_result <= 0)
		{
			return 0;
		}

		result = read_result;
	}
	else
	{
		const auto data_size = audio_decoder_.get_data_size();
		const auto decoded_size = audio_decoder_.get_decoded_size();

		auto new_decoded_size = decoded_size + static_cast<int>(cbSize);

		if (new_decoded_size > data_size)
		{
			new_decoded_size = data_size;
		}

		const auto skip_size = new_decoded_size - decoded_size;

		if (skip_size == 0)
		{
			return 0;
		}

		const auto sample_size = audio_decoder_.get_sample_size();
		const auto sample_offset = new_decoded_size / sample_size;

		if (!audio_decoder_.set_position(sample_offset))
		{
			return 0;
		}

		constexpr auto max_sample_size = (default_bit_depth / 8) * max_channel_count;
		const auto sample_reminder = new_decoded_size % sample_size;

		if (sample_reminder > max_sample_size)
		{
			return 0;
		}

		if (sample_reminder > 0)
		{
			std::uint8_t byte_buffer[max_sample_size];

			if (audio_decoder_.decode(byte_buffer, sample_reminder) != sample_reminder)
			{
				return 0;
			}
		}

		result = skip_size;
	}

	m_nBytesCopied += result;
	m_nMaxBytesCopied = Max(m_nMaxBytesCopied, m_nBytesCopied);

	return result;
}

// SeekFromStart
//
// Returns logical offset into buffer.
// On error, returns 0.
//
std::uint32_t WaveFile::SeekFromStart(
	const std::uint32_t nBytes)
{
	if (!Cue())
	{
		return 0;
	}

	return Read(nullptr, nBytes);
}

// ReadCompressed
//
// Returns number of uncompressed bytes actually read.
// On error, returns 0.
//
//
// Basically what we want to do, in general is read STR_BUFFER_SIZE worth
// of UNCOMPRESSED data.  Since we don't know exactly how much compressed
// data to read to get exactly that amount, we estimate and then adjust
// as necessary.
std::uint32_t WaveFile::ReadCompressed(
	std::uint8_t* pbDest,
	const std::uint32_t cbSize,
	const std::uint8_t* pCompressedBuffer,
	std::uint8_t* pDecompressedBuffer)
{
	static_cast<void>(pCompressedBuffer);
	static_cast<void>(pDecompressedBuffer);

	return Read(pbDest, cbSize);
}

std::uint32_t WaveFile::GetDataSize() const
{
	return audio_decoder_.get_data_size();
}

std::uint32_t WaveFile::GetMaxBytesCopied() const
{
	return m_nMaxBytesCopied;
}

std::uint32_t WaveFile::GetDuration() const
{
	return m_nDuration;
}

// GetSilenceData
//
// Returns 8 bits of data representing silence for the Wave file format.
//
// Since we are dealing only with PCM format, we can fudge a bit and take
// advantage of the fact that for all PCM formats, silence can be represented
// by a single byte, repeated to make up the proper word size. The actual size
// of a word of wave data depends on the format:
//
// PCM Format       Word Size       Silence Data
// 8-bit mono       1 byte          0x80
// 8-bit stereo     2 bytes         0x8080
// 16-bit mono      2 bytes         0x0000
// 16-bit stereo    4 bytes         0x00000000
//
std::uint8_t WaveFile::GetSilenceData() const
{
	auto bSilenceData = std::uint8_t{};

	// Silence data depends on format of Wave file
	if (wave_format_ex_.bit_depth_ == 8)
	{
		// For 8-bit formats (unsigned, 0 to 255)
		// Packed DWORD = 0x80808080;
		bSilenceData = 0x80;
	}
	else if (wave_format_ex_.bit_depth_ == 16)
	{
		// For 16-bit formats (signed, -32768 to 32767)
		// Packed DWORD = 0x00000000;
		bSilenceData = 0x00;
	}
	else
	{
		ASSERT(0);
	}

	return bSilenceData;
}

bool WaveFile::IsActive() const
{
	return m_hStream != nullptr;
}

void WaveFile::SetStream(
	LHSTREAM hStream)
{
	m_hStream = hStream;
}

LHSTREAM WaveFile::GetStream() const
{
	return m_hStream;
}

const ul::WaveFormatEx& WaveFile::get_format() const
{
	return wave_format_ex_;
}


//! I3DObject

I3DObject::I3DObject( )
{
	Reset( );
	memset( &m_userData, 0, sizeof( m_userData ));
}

I3DObject::~I3DObject( )
{
	Term( );
}

void I3DObject::Reset( )
{
	m_position = LTVector( 0.0f, 0.0f, 0.0f );
	m_velocity = LTVector( 0.0f, 0.0f, 0.0f );
	m_up = LTVector( 0.0f, 0.0f, 0.0f );
	m_face = LTVector( 0.0f, 0.0f, 0.0f );
//	for( S32 i = 0; i < MAX_USER_DATA_INDEX + 1; m_userData[ i++ ] = 0 );
}

bool I3DObject::Init( )
{
	Term( );
	return true;
}

void I3DObject::Term( )
{
	Reset( );
}

void I3DObject::SetPosition( LTVector& pos )
{
	m_position = pos;
}

void I3DObject::SetVelocity( LTVector& vel )
{
	m_velocity = vel;
}

void I3DObject::SetOrientation( LTVector& up, LTVector& face )
{
	m_up = up;
	m_face = face;
}

//! C3DListener

C3DListener::C3DListener( )
{
	Reset( );
}

C3DListener::~C3DListener( )
{
	Term( );
}

void C3DListener::Reset( )
{
	I3DObject::Reset( );
	m_pDS3DListener = NULL;
	m_fDopplerSetting = 0.0f;
}

bool C3DListener::Init( HRESULT& hResult, LPDIRECTSOUNDBUFFER pDSPrimaryBuffer )
{
	Term( );

	if( !I3DObject::Init( ) )
		return false;

	if( ( hResult = pDSPrimaryBuffer->QueryInterface( IID_IDirectSound3DListener, ( void** )&m_pDS3DListener ) ) != DS_OK )
		return false;

	m_pDS3DListener->SetDopplerFactor( m_fDopplerSetting, DS3D_DEFERRED );

	return true;
}

void C3DListener::Term( )
{
	if( m_pDS3DListener != NULL )
		m_pDS3DListener->Release( );
	I3DObject::Term( );

	Reset( );
}

void C3DListener::SetPosition( LTVector& pos )
{
	I3DObject::SetPosition( pos );
	m_pDS3DListener->SetPosition( pos.x, pos.y, pos.z, DS3D_DEFERRED );
}

void C3DListener::SetVelocity( LTVector& vel )
{
	I3DObject::SetVelocity( vel );
	m_pDS3DListener->SetVelocity( vel.x, vel.y, vel.z, DS3D_DEFERRED );
}

void C3DListener::SetOrientation( LTVector& up, LTVector& face )
{
	I3DObject::SetOrientation( up, face );
	m_pDS3DListener->SetOrientation( face.x, face.y, face.z, up.x, up.y, up.z, DS3D_DEFERRED );
}


//! CSample

LTLink CSample::m_lstSampleLoopHead;


CSample::CSample( )
{
	memset( &m_userData, 0, sizeof( m_userData ));
	m_bLoopBlock = false;
	m_lnkLoopNotify.Init2( this );
	Reset( );
}

CSample::~CSample( )
{
	Term( );
}

void CheckLinkList( )
{
	LTLink* pLink = CSample::m_lstSampleLoopHead.m_pNext;
	while( pLink != &CSample::m_lstSampleLoopHead )
	{
		LTLink* pNext = pLink->m_pNext;

		if( pNext == pLink )
			ASSERT( !"Infinite loop." );

		pLink = pNext;

	}
}


void CSample::Reset( )
{
	m_pDSBuffer = NULL;
	m_pSoundData = NULL;
	m_uiSoundDataLen = 0;
	m_bAllocatedSoundData = false;
	m_dwPlayFlags = 0;
	m_nLastPlayPos = 0;
	m_bLooping = false;
	dl_Remove( &m_lnkLoopNotify );
}

bool CSample::Init( HRESULT& hResult, LPDIRECTSOUND pDS, const uint32 uiNumSamples,
				const bool b3DBuffer, const ul::WaveFormatEx* pWaveFormat, const LTSOUNDFILTERDATA* pFilterData )
{
	bool bUseFilter;

	Term( );

	if( pWaveFormat == NULL )
	{
		m_waveFormat.block_align_ = ( m_waveFormat.channel_count_ * m_waveFormat.bit_depth_ ) >> 3;
		m_waveFormat.avg_bytes_per_sec_ = m_waveFormat.sample_rate_ * m_waveFormat.block_align_;
		m_dsbDesc.dwBufferBytes = uiNumSamples * m_waveFormat.block_align_;
		m_dsbDesc.lpwfxFormat = reinterpret_cast<LPWAVEFORMATEX>(&m_waveFormat);
	}

	else
	{
		m_waveFormat = *pWaveFormat;
		m_dsbDesc.dwBufferBytes = uiNumSamples;
		m_dsbDesc.lpwfxFormat = reinterpret_cast<LPWAVEFORMATEX>(&m_waveFormat);
	}

	// see if we want filtering
	bUseFilter = false;
	if ( pFilterData )
	{
		if ( pFilterData->bUseFilter )
		{
			bUseFilter = true;
		}
	}

	if ( b3DBuffer )
		m_dsbDesc.guid3DAlgorithm = DS3DALG_HRTF_FULL;
	else
		m_dsbDesc.guid3DAlgorithm = DS3DALG_DEFAULT;

	// filters and frequency variation are mutually exclusive
	if ( bUseFilter )
	{
//		m_dsbDesc.dwFlags |= DSBCAPS_CTRLFX;
		m_dsbDesc.dwFlags &= ~DSBCAPS_CTRLFREQUENCY;

		// DirectSound will not create buffer with FX if it's too small
		// so force it to minimum size, if necessary
		DWORD dwMinBufferBytes = DSBSIZE_FX_MIN * m_dsbDesc.lpwfxFormat->nAvgBytesPerSec / 1000;
		if ( m_dsbDesc.dwBufferBytes < dwMinBufferBytes )
			m_dsbDesc.dwBufferBytes = dwMinBufferBytes;
	}
	else
	{
		m_dsbDesc.dwFlags |= DSBCAPS_CTRLFREQUENCY;
		m_dsbDesc.dwFlags &= ~DSBCAPS_CTRLFX;
	}

	LPDIRECTSOUNDBUFFER pDSBuffer;

	hResult = pDS->CreateSoundBuffer( &m_dsbDesc, &pDSBuffer, NULL );


	if( FAILED(hResult))
		return false;

	hResult = pDSBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&m_pDSBuffer);
	pDSBuffer->Release();
	if( FAILED(hResult))
		return false;

	// now set up any effects on the buffer
#ifdef USE_DX8_SOFTWARE_FILTERS
	DWORD dwResult;
	uint32 uiFilterParamFlags;
	if ( bUseFilter )
	{
		DSEFFECTDESC dsEffectDesc;

		memset( (void*) &dsEffectDesc, 0, sizeof( DSEFFECTDESC ) );
		dsEffectDesc.dwSize = sizeof( DSEFFECTDESC );
		dsEffectDesc.dwFlags = 0;

		// set up the correct effect based on the filter info
		switch ( pFilterData->uiFilterType )
		{
		case FilterChorus:
			dsEffectDesc.guidDSFXClass = GUID_DSFX_STANDARD_CHORUS;
			break;
		case FilterCompressor:
			dsEffectDesc.guidDSFXClass = GUID_DSFX_STANDARD_COMPRESSOR;
			break;
		case FilterDistortion:
			dsEffectDesc.guidDSFXClass = GUID_DSFX_STANDARD_DISTORTION;
			break;
		case FilterEcho:
			dsEffectDesc.guidDSFXClass = GUID_DSFX_STANDARD_ECHO;
			break;
		case FilterFlange:
			dsEffectDesc.guidDSFXClass = GUID_DSFX_STANDARD_FLANGER;
			break;
		case FilterParamEQ:
			dsEffectDesc.guidDSFXClass = GUID_DSFX_STANDARD_PARAMEQ;
			break;
		case FilterReverb:
			dsEffectDesc.guidDSFXClass = GUID_DSFX_STANDARD_I3DL2REVERB;
			break;
		}

		hResult = m_pDSBuffer->SetFX( 1, &dsEffectDesc, &dwResult );

		if ( FAILED( hResult ) )
			return false;

		// if a reverb, set the correct one
		switch ( pFilterData->uiFilterType )
		{
		case FilterChorus:
			LPDIRECTSOUNDFXCHORUS lpChorus;
			DSFXChorus chorusParams;
			LTFILTERCHORUS* pLTChorus;

			hResult = m_pDSBuffer->GetObjectInPath( GUID_DSFX_STANDARD_CHORUS,
				0, IID_IDirectSoundFXChorus, (LPVOID*) &lpChorus );

			if ( FAILED( hResult ) )
				return false;

			lpChorus->GetAllParameters( &chorusParams );
			if ( FAILED( hResult ) )
				return false;

			// now set any of the parameters that have been set for this filter type
			uiFilterParamFlags = pFilterData->pSoundFilter->uiFilterParamFlags;
			pLTChorus = (LTFILTERCHORUS*) pFilterData->pSoundFilter;
			if ( uiFilterParamFlags & SET_CHORUS_DELAY )
				chorusParams.fDelay = pLTChorus->fDelay;
			if ( uiFilterParamFlags & SET_CHORUS_DEPTH )
				chorusParams.fDepth = pLTChorus->fDepth;
			if ( uiFilterParamFlags & SET_CHORUS_FEEDBACK )
				chorusParams.fFeedback = pLTChorus->fFeedback;
			if ( uiFilterParamFlags & SET_CHORUS_FREQUENCY )
				chorusParams.fFrequency = pLTChorus->fFrequency;
			if ( uiFilterParamFlags & SET_CHORUS_WETDRYMIX )
				chorusParams.fWetDryMix = pLTChorus->fWetDryMix;
			if ( uiFilterParamFlags & SET_CHORUS_WAVEFORM )
				chorusParams.lWaveform = pLTChorus->lWaveform;
			if ( uiFilterParamFlags & SET_CHORUS_PHASE )
				chorusParams.lPhase = pLTChorus->lPhase;

			hResult = lpChorus->SetAllParameters( &chorusParams );

			if ( FAILED( hResult ) )
				return false;

			break;

		case FilterCompressor:
			LPDIRECTSOUNDFXCOMPRESSOR lpCompressor;
			DSFXCompressor compressorParams;
			LTFILTERCOMPRESSOR* pLTCompressor;

			hResult = m_pDSBuffer->GetObjectInPath( GUID_DSFX_STANDARD_COMPRESSOR,
				0, IID_IDirectSoundFXCompressor, (LPVOID*) &lpCompressor );
			if ( FAILED( hResult ) )
				return false;

			lpCompressor->GetAllParameters( &compressorParams );
			if ( FAILED( hResult ) )
				return false;

			// now set any of the parameters that have been set for this filter type
			uiFilterParamFlags = pFilterData->pSoundFilter->uiFilterParamFlags;
			pLTCompressor = (LTFILTERCOMPRESSOR*) pFilterData->pSoundFilter;
			if ( uiFilterParamFlags & SET_COMPRESSOR_ATTACK )
				compressorParams.fAttack = pLTCompressor->fAttack;
			if ( uiFilterParamFlags & SET_COMPRESSOR_GAIN )
				compressorParams.fGain = pLTCompressor->fGain;
			if ( uiFilterParamFlags & SET_COMPRESSOR_PREDELAY )
				compressorParams.fPredelay = pLTCompressor->fPredelay;
			if ( uiFilterParamFlags & SET_COMPRESSOR_RATIO )
				compressorParams.fRatio = pLTCompressor->fRatio;
			if ( uiFilterParamFlags & SET_COMPRESSOR_RELEASE )
				compressorParams.fRelease = pLTCompressor->fRelease;
			if ( uiFilterParamFlags & SET_COMPRESSOR_THRESHOLD )
				compressorParams.fThreshold = pLTCompressor->fThreshold;

			hResult = lpCompressor->SetAllParameters( &compressorParams );

			if ( FAILED( hResult ) )
				return false;

			break;

		case FilterDistortion:
			LPDIRECTSOUNDFXDISTORTION lpDistortion;
			DSFXDistortion distortionParams;
			LTFILTERDISTORTION* pLTDistortion;

			hResult = m_pDSBuffer->GetObjectInPath( GUID_DSFX_STANDARD_DISTORTION,
				0, IID_IDirectSoundFXDistortion, (LPVOID*) &lpDistortion );
			if ( FAILED( hResult ) )
				return false;

			lpDistortion->GetAllParameters( &distortionParams );
			if ( FAILED( hResult ) )
				return false;

			// now set any of the parameters that have been set for this filter type
			uiFilterParamFlags = pFilterData->pSoundFilter->uiFilterParamFlags;
			pLTDistortion = (LTFILTERDISTORTION*) pFilterData->pSoundFilter;
			if ( uiFilterParamFlags & SET_DISTORTION_EDGE )
				distortionParams.fEdge = pLTDistortion->fEdge;
			if ( uiFilterParamFlags & SET_DISTORTION_GAIN )
				distortionParams.fGain = pLTDistortion->fGain;
			if ( uiFilterParamFlags & SET_DISTORTION_POSTEQBANDWIDTH )
				distortionParams.fPostEQBandwidth = pLTDistortion->fPostEQBandwidth;
			if ( uiFilterParamFlags & SET_DISTORTION_POSTEQCENTERFREQ )
				distortionParams.fPostEQCenterFrequency = pLTDistortion->fPostEQCenterFrequency;
			if ( uiFilterParamFlags & SET_DISTORTION_PRELOWPASSCUTOFF )
				distortionParams.fPreLowpassCutoff = pLTDistortion->fPreLowpassCutoff;

			hResult = lpDistortion->SetAllParameters( &distortionParams );

			if ( FAILED( hResult ) )
				return false;

			break;

		case FilterEcho:
			LPDIRECTSOUNDFXECHO lpEcho;
			DSFXEcho echoParams;
			LTFILTERECHO* pLTEcho;

			hResult = m_pDSBuffer->GetObjectInPath( GUID_DSFX_STANDARD_ECHO,
				0, IID_IDirectSoundFXEcho, (LPVOID*) &lpEcho );
			if ( FAILED( hResult ) )
				return false;

			lpEcho->GetAllParameters( &echoParams );
			if ( FAILED( hResult ) )
				return false;

			// now set any of the parameters that have been set for this filter type
			uiFilterParamFlags = pFilterData->pSoundFilter->uiFilterParamFlags;
			pLTEcho = (LTFILTERECHO*) pFilterData->pSoundFilter;
			if ( uiFilterParamFlags & SET_ECHO_FEEDBACK )
				echoParams.fFeedback = pLTEcho->fFeedback;
			if ( uiFilterParamFlags & SET_ECHO_LEFTDELAY )
				echoParams.fLeftDelay = pLTEcho->fLeftDelay;
			if ( uiFilterParamFlags & SET_ECHO_RIGHTDELAY )
				echoParams.fRightDelay = pLTEcho->fRightDelay;
			if ( uiFilterParamFlags & SET_ECHO_WETDRYMIX )
				echoParams.fWetDryMix = pLTEcho->fWetDryMix;
			if ( uiFilterParamFlags & SET_ECHO_PANDELAY )
				echoParams.lPanDelay = pLTEcho->lPanDelay;

			hResult = lpEcho->SetAllParameters( &echoParams );

			if ( FAILED( hResult ) )
				return false;

			break;

		case FilterFlange:
			LPDIRECTSOUNDFXFLANGER lpFlange;
			DSFXFlanger flangeParams;
			LTFILTERFLANGE* pLTFlange;

			hResult = m_pDSBuffer->GetObjectInPath( GUID_DSFX_STANDARD_FLANGER,
				0, IID_IDirectSoundFXFlanger, (LPVOID*) &lpFlange );
			if ( FAILED( hResult ) )
				return false;

			lpFlange->GetAllParameters( &flangeParams );
			if ( FAILED( hResult ) )
				return false;

			// now set any of the parameters that have been set for this filter type
			uiFilterParamFlags = pFilterData->pSoundFilter->uiFilterParamFlags;
			pLTFlange = (LTFILTERFLANGE*) pFilterData->pSoundFilter;
			if ( uiFilterParamFlags & SET_FLANGE_DELAY )
				flangeParams.fDelay = pLTFlange->fDelay;
			if ( uiFilterParamFlags & SET_FLANGE_DEPTH )
				flangeParams.fDepth = pLTFlange->fDepth;
			if ( uiFilterParamFlags & SET_FLANGE_FEEDBACK )
				flangeParams.fFeedback = pLTFlange->fFeedback;
			if ( uiFilterParamFlags & SET_FLANGE_FREQUENCY )
				flangeParams.fFrequency = pLTFlange->fFrequency;
			if ( uiFilterParamFlags & SET_FLANGE_WETDRYMIX )
				flangeParams.fWetDryMix = pLTFlange->fWetDryMix;
			if ( uiFilterParamFlags & SET_FLANGE_PHASE )
				flangeParams.lPhase = pLTFlange->lPhase;
			if ( uiFilterParamFlags & SET_FLANGE_WAVEFORM )
				flangeParams.lWaveform = pLTFlange->lWaveform;

			hResult = lpFlange->SetAllParameters( &flangeParams );

			if ( FAILED( hResult ) )
				return false;

			break;

		case FilterParamEQ:
			DSFXParamEq paramEQParams;
			LTFILTERPARAMEQ* pLTParamEQ;
			LPDIRECTSOUNDFXPARAMEQ lpParamEQ;

			hResult = m_pDSBuffer->GetObjectInPath( GUID_DSFX_STANDARD_PARAMEQ,
				0, IID_IDirectSoundFXParamEq, (LPVOID*) &lpParamEQ );
			if ( FAILED( hResult ) )
				return false;
			lpParamEQ->GetAllParameters( &paramEQParams );
			if ( FAILED( hResult ) )
				return false;

			// now set any of the parameters that have been set for this filter type
			uiFilterParamFlags = pFilterData->pSoundFilter->uiFilterParamFlags;
			pLTParamEQ = (LTFILTERPARAMEQ*) pFilterData->pSoundFilter;
			if ( uiFilterParamFlags & SET_PARAMEQ_BANDWIDTH )
				paramEQParams.fBandwidth = pLTParamEQ->fBandwidth;
			if ( uiFilterParamFlags & SET_PARAMEQ_CENTER )
				paramEQParams.fCenter = pLTParamEQ->fCenter;
			if ( uiFilterParamFlags & SET_PARAMEQ_GAIN )
				paramEQParams.fGain = pLTParamEQ->fGain;

			hResult = lpParamEQ->SetAllParameters( &paramEQParams );

			if ( FAILED( hResult ) )
				return false;
			break;

		case FilterReverb:
			LPDIRECTSOUNDFXI3DL2REVERB8 lpReverb;
			DSFXI3DL2Reverb reverbParams;
			LTFILTERREVERB* pLTReverb;

			hResult = m_pDSBuffer->GetObjectInPath( GUID_DSFX_STANDARD_I3DL2REVERB,
				0, IID_IDirectSoundFXI3DL2Reverb8, (LPVOID*) &lpReverb );
			if ( FAILED( hResult ) )
				return false;

			lpReverb->GetAllParameters( &reverbParams );
			if ( FAILED( hResult ) )
				return false;

			// now set any of the parameters that have been set for this filter type
			uiFilterParamFlags = pFilterData->pSoundFilter->uiFilterParamFlags;
			pLTReverb = (LTFILTERREVERB*) pFilterData->pSoundFilter;
			if ( uiFilterParamFlags & SET_REVERB_DECAYHFRATIO )
				reverbParams.flDecayHFRatio = pLTReverb->fDecayHFRatio;
			if ( uiFilterParamFlags & SET_REVERB_DECAYTIME )
				reverbParams.flDecayTime = pLTReverb->fDecayTime;
			if ( uiFilterParamFlags & SET_REVERB_DENSITY )
				reverbParams.flDensity = pLTReverb->fDensity;
			if ( uiFilterParamFlags & SET_REVERB_DIFFUSION )
				reverbParams.flDiffusion = pLTReverb->fDiffusion;
			if ( uiFilterParamFlags & SET_REVERB_HFREFERENCE )
				reverbParams.flHFReference = pLTReverb->fHFReference;
			if ( uiFilterParamFlags & SET_REVERB_REFLECTIONSDELAY )
				reverbParams.flReflectionsDelay = pLTReverb->fReflectionsDelay;
			if ( uiFilterParamFlags & SET_REVERB_REVERBDELAY )
				reverbParams.flReverbDelay = pLTReverb->fReverbDelay;
			if ( uiFilterParamFlags & SET_REVERB_ROOMROLLOFFFACTOR )
				reverbParams.flRoomRolloffFactor = pLTReverb->fRoomRolloffFactor;
			if ( uiFilterParamFlags & SET_REVERB_REFLECTIONS )
				reverbParams.lReflections = pLTReverb->lReflections;
			if ( uiFilterParamFlags & SET_REVERB_REVERB )
				reverbParams.lReverb = pLTReverb->lReverb;
			if ( uiFilterParamFlags & SET_REVERB_ROOM )
				reverbParams.lRoom = pLTReverb->lRoom;
			if ( uiFilterParamFlags & SET_REVERB_ROOMHF )
				reverbParams.lRoomHF = pLTReverb->lRoomHF;

			hResult = lpReverb->SetAllParameters( &reverbParams );

			if ( FAILED( hResult ) )
				return false;
			break;
		}

	}
#endif

#ifdef USE_EAX20_HARDWARE_FILTERS

	if ( g_pSoundSys->SupportsEAX20Filter( ))
	{
		// only want the 3D buffers
		if ( b3DBuffer )
		{
			LPKSPROPERTYSET pKSPropertySet = NULL;
			hResult = m_pDSBuffer->QueryInterface(IID_IKsPropertySet, (LPVOID *)&pKSPropertySet);
			if ( hResult != DS_OK )
				return false;

			// set the direct attenuation, this is implemented when we actually allocate a sound.
			hResult = pKSPropertySet->Set(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_DIRECT, NULL, 0, (void*)&g_iCurrentEAXDirectSetting, sizeof(unsigned long));
			if ( hResult != DS_OK )
				return false;
			pKSPropertySet->Release();
		}
	}
#endif

	return true;
}

void CSample::Term( )
{
	dl_Remove( &m_lnkLoopNotify );

 	if( m_pDSBuffer != NULL )
		m_pDSBuffer->Release( );

	if( m_bAllocatedSoundData && m_pSoundData != NULL )
		delete[] m_pSoundData;

	Reset( );
}

void CSample::Restore( )
{
	if( m_pDSBuffer == NULL )
		return;

	unsigned long uiStatus;
	m_pDSBuffer->GetStatus( &uiStatus );

	if( ( uiStatus & DSBSTATUS_BUFFERLOST ) == 0 )
		return;

	m_pDSBuffer->Restore( );

	Fill( );
}

bool CSample::Fill( )
{
	if( m_pSoundData == NULL || m_uiSoundDataLen == 0 || m_pDSBuffer == NULL )
		return false;

	// Make sure we're stopped.  Shouldn't get here, since buffer's are stopped
	// when the sound ends.
	if( IsPlaying( ))
	{
		ASSERT( !"CSample::Fill:  Buffer should have been stopped before reaching fill." );
		m_pDSBuffer->Stop( );
	}

	// lock sound buffer

	unsigned long uiNumBytes[2];
	void* apLockedBytes[2];

	HRESULT hResult = m_pDSBuffer->Lock( 0, m_uiSoundDataLen, &apLockedBytes[0], &uiNumBytes[0],
		&apLockedBytes[1], &uiNumBytes[1], 0 );
	if( hResult != DS_OK )
		return false;

	// copy sound data to buffer

 	memcpy( apLockedBytes[0], m_pSoundData, m_uiSoundDataLen );

	// unlock sound buffer

	m_pDSBuffer->Unlock( apLockedBytes[0], uiNumBytes[0],
		apLockedBytes[1], uiNumBytes[1] );

	return true;
}

//	===========================================================================
//	Incorporation of DSMStrm* required functionality

bool CSample::IsPlaying( )
{
	if( m_pDSBuffer == NULL )
		return false;

	DWORD dwStatus;
	HRESULT hResult = m_pDSBuffer->GetStatus( &dwStatus );

	if( hResult != DS_OK )
		return false;

	if( ( dwStatus & DSBSTATUS_PLAYING ) == DSBSTATUS_PLAYING )
		return true;

	return false;
}

void CSample::SetLooping( CDx8SoundSys* pSoundSys, bool bLoop )
{
	if( bLoop )
		m_dwPlayFlags = m_dwPlayFlags | DSBPLAY_LOOPING;
	else
		m_dwPlayFlags = m_dwPlayFlags & ( ~DSBPLAY_LOOPING );

	// Make sure we're setup on the notify list.
	pSoundSys->SetSampleNotify( this, bLoop );

	// If we changed our looping state, we need
	// to start the sound over.
	if( bLoop != m_bLooping )
	{
		if( IsPlaying( ))
		{
			Stop( false );
			Play( );
		}
	}

	m_bLooping = bLoop;
}

BOOL CSample::GetCurrentPosition( DWORD* pdwPlayPos, DWORD* pdwWritePos )
{
	if( m_pDSBuffer == NULL )
		return FALSE;

	HRESULT hResult = m_pDSBuffer->GetCurrentPosition( pdwPlayPos, pdwWritePos );

	if( hResult != DS_OK )
		return FALSE;

	return TRUE;
}

BOOL CSample::SetCurrentPosition( DWORD dwStartOffset )
{
	if( m_pDSBuffer == NULL )
		return FALSE;

	HRESULT hResult = m_pDSBuffer->SetCurrentPosition( dwStartOffset );

	if( hResult != DS_OK )
		return FALSE;

	// Set the last play pos to zero, because doing this on a playing sound
	// seems to not really set it to exactly dwStartOffset anyway.
	m_nLastPlayPos = 0;

	return TRUE;
}

BOOL CSample::Play( )
{
	if( m_pDSBuffer == NULL )
		return FALSE;

	HRESULT hResult = m_pDSBuffer->Play( 0, 0, m_dwPlayFlags );

	if( hResult != DS_OK )
	{
		if( hResult == DSERR_BUFFERLOST )
		{
			Restore( );

            hResult = m_pDSBuffer->Play( 0, 0, m_dwPlayFlags );

			if( hResult != DS_OK )
				return FALSE;

			return TRUE;
		}

		return FALSE;
	}
	return TRUE;
}

BOOL CSample::Stop( bool bReset )
{
	if( m_pDSBuffer == NULL )
		return FALSE;

	HRESULT hResult = m_pDSBuffer->Stop( );

	if( hResult != DS_OK )
	{
		return FALSE;
	}

	if( bReset )
	{
		m_nLastPlayPos = 0;
        hResult = m_pDSBuffer->SetCurrentPosition( 0 );

		if( hResult != DS_OK )
			return FALSE;
	}

	return TRUE;
}

BOOL CSample::Lock( DWORD dwStartOffset, DWORD dwLockAmount, void** ppChunk1, DWORD* pdwChunkSize1,
	void** ppChunk2, DWORD* pdwChunkSize2, DWORD dwFlags )
{
	if( m_pDSBuffer == NULL )
		return FALSE;

	HRESULT hResult = m_pDSBuffer->Lock( dwStartOffset, dwLockAmount, ppChunk1, pdwChunkSize1, ppChunk2,
		pdwChunkSize2, dwFlags );

	if( hResult != DS_OK )
	{
		if( hResult == DSERR_BUFFERLOST )
		{
			Restore( );

            hResult = m_pDSBuffer->Lock( dwStartOffset, dwLockAmount, ppChunk1, pdwChunkSize1, ppChunk2,
				pdwChunkSize2, dwFlags );

			if( hResult != DS_OK )
			{
				auto pcDSError = ConvertDSErrorToString( hResult );
                static_cast<void>(pcDSError);
				return FALSE;
			}

			return TRUE;
        }

		auto pcDSError = ConvertDSErrorToString( hResult );
        static_cast<void>(pcDSError);
        return FALSE;
    }

	return TRUE;
}

void CSample::Unlock( void* pChunk1, DWORD dwChunkSize1, void* pChunk2, DWORD dwChunkSize2 )
{
	if( m_pDSBuffer == NULL )
		return;
	m_pDSBuffer->Unlock( pChunk1, dwChunkSize1, pChunk2, dwChunkSize2 );
}

void CSample::HandleLoop( CDx8SoundSys* pSoundSys )
{
	HRESULT hr;
	DWORD dwCurrentPlayPos;
	DWORD dwCurrentWritePos;

	// see if it's time for an update
    if( FAILED( hr = m_pDSBuffer->GetCurrentPosition( &dwCurrentPlayPos, &dwCurrentWritePos ) ) )
		return;

	// This will hold the unlooped play pos.
	uint32 nFlatCurrentPlayPos = dwCurrentPlayPos;

	// Check if we looped.
	if( dwCurrentPlayPos < m_nLastPlayPos )
	{
		nFlatCurrentPlayPos = m_uiSoundDataLen + dwCurrentPlayPos;
	}

	m_nLastPlayPos = dwCurrentPlayPos;

	if ( nFlatCurrentPlayPos >= (DWORD)m_nLoopEnd )
	{
		// we've reached the loop point, go back to the beginning
		SetCurrentPosition( m_nLoopStart );
	}
}

//	===========================================================================

C3DSample::C3DSample( )
{
	Reset( );
}

C3DSample::~C3DSample( )
{
	Term( );
}

void C3DSample::Reset( )
{
	I3DObject::Reset( );
	m_sample.Reset( );
	m_pDS3DBuffer = NULL;
	m_status = LS_DONE;
}

bool C3DSample::Init( HRESULT& hResult, LPDIRECTSOUND pDS, const uint32 uiNumSamples, const ul::WaveFormatEx* pWaveFormat, const LTSOUNDFILTERDATA* pFilterData )
{
	Term( );

	if( !I3DObject::Init( ) )
	{
		return false;
	}

	if( !m_sample.Init( hResult, pDS, uiNumSamples, true, pWaveFormat, pFilterData ) )
	{
		return false;
	}

	if( ( hResult = m_sample.m_pDSBuffer->QueryInterface( IID_IDirectSound3DBuffer, ( void** )&m_pDS3DBuffer ) ) != DS_OK )
	{
		return false;
	}

	return true;
}

void C3DSample::Term( )
{
	if( m_pDS3DBuffer != NULL )
	{
		m_pDS3DBuffer->Release( );
	}

	m_sample.Term( );
	I3DObject::Term( );

	Reset( );
}

void C3DSample::SetPosition( LTVector& pos )
{
	LTVector vPosInner;
//	float fRatioInnerToDist, fDistSquared;

	I3DObject::SetPosition(	pos );

/*
	// we want the relative position of the object to the inner
	// radius of the object
	fDistSquared = pos.MagSqr();
	fRatioInnerToDist = sqrtf(m_innerRadiusSquared)/sqrtf(fDistSquared);

	// if it's inside the inner radius
	if ( fRatioInnerToDist > 1.0f )
	{
		float fRatioInv = 1.0f / fRatioInnerToDist;
		pos = pos * fRatioInv * (m_DSMinDist/m_innerRadius);
	}
	else
	{
		// otherwise build a vector that is at the appropriate distance

		// get a vector to the inner radius in the appropriate direction
		vPosInner = pos * fRatioInnerToDist;
		// get the relative position of the object outside the inner radius
		pos = pos - vPosInner;
	}
*/
	HRESULT hr = m_pDS3DBuffer->SetPosition( pos.x, pos.y, pos.z, DS3D_DEFERRED );
    static_cast<void>(hr);
}

void C3DSample::SetVelocity( LTVector& vel )
{
	I3DObject::SetVelocity( vel );
	m_pDS3DBuffer->SetVelocity( vel.x, vel.y, vel.z, DS3D_DEFERRED );
}

void C3DSample::SetOrientation( LTVector& up, LTVector& face )
{
	I3DObject::SetOrientation( up, face );
	m_pDS3DBuffer->SetConeOrientation( face.x, face.y, face.z, DS3D_DEFERRED );
}

void C3DSample::SetRadiusData( float& fInnerRadius, float& fOuterRadius )
{
	m_innerRadius = fInnerRadius;
	m_outerRadius = fOuterRadius;
	m_innerRadiusSquared = fInnerRadius * fInnerRadius;
}

void C3DSample::GetRadiusData( float* fInnerRadius, float* fInnerRadiusSquared )
{
	*fInnerRadius = m_innerRadius;
	*fInnerRadiusSquared = m_innerRadiusSquared;
}


CStream::CStream( CStream* pPrev, CStream* pNext ) :
  	CSample( ), m_pPrev( pPrev ), m_pNext( pNext )
{
	if( m_pPrev != NULL )
  	{
  		m_pNext = m_pPrev->m_pNext;
  		m_pPrev->m_pNext = this;
  	}

  	if( m_pNext != NULL )
  	{
  		m_pPrev = m_pNext->m_pPrev;
  		m_pNext->m_pPrev = this;
  	}

	m_uiBufferSize = 0;
	m_uiNextWriteOffset = 0;
	m_uiLastPlayPos = 0;
	m_uiTotalPlayed = 0;
	m_pWaveFile = NULL;
	m_nEventNum = -1;
}

CStream::~CStream( )
{
	if( m_pPrev != NULL )
  		m_pPrev->m_pNext = m_pNext;

  	if( m_pNext != NULL )
  		m_pNext->m_pPrev = m_pPrev;
}

void CStream::ReadStreamIntoBuffer( CDx8SoundSys* pSoundSys, BYTE* pBuffer, int32 nBufferSize )
{
	if( IsLooping( ))
{
		uint32 nTotalBytesRead = 0;

		while( 1 )
	{
			// Fill in the buffer from the stream.
			BYTE* pBufferFill = pBuffer + nTotalBytesRead;
			uint32 nBytesToRead = nBufferSize - nTotalBytesRead;
			uint32 nBytesRead = pSoundSys->ReadStream( m_pWaveFile, pBufferFill, nBytesToRead );

			nTotalBytesRead += nBytesRead;

			// Check to see if we're done.
			if( nTotalBytesRead >= ( uint32 )nBufferSize )
		{
				break;
			}

			// Start the file from the beginning for the next read.
			m_pWaveFile->Cue( );
		}
		}
	// Not looping.
		else
		{
		int32 uiBytesRead = pSoundSys->ReadStream( m_pWaveFile, pBuffer, nBufferSize );
		if( uiBytesRead < nBufferSize )
			{
				// Fill in the rest of the buffer with silence.
			BYTE nZeroValue = (BYTE)(m_pWaveFile->get_format().bit_depth_ == 8 ? 128 : 0 );
			BYTE* pBufferEnd = pBuffer + uiBytesRead;
			uint32 nBytesToZero = nBufferSize - uiBytesRead;
			FillMemory( pBufferEnd, nBytesToZero, nZeroValue );
		}
	}
}

// stream in more data when we get an
// update event
void CStream::HandleUpdate( CDx8SoundSys* pSoundSys )
{
	HRESULT hr;
    DWORD   dwCurrentPlayPos;
	DWORD	dwCurrentWritePos;
    VOID*   pDSLockedBuffer = NULL;
    VOID*   pDSLockedBuffer2 = NULL;
    DWORD   dwDSLockedBufferSize;
    DWORD   dwDSLockedBufferSize2;

	// see if it's time for an update
    if( FAILED( hr = m_pDSBuffer->GetCurrentPosition( &dwCurrentPlayPos, &dwCurrentWritePos ) ) )
		return;

	// Update the total number of bytes played.
	uint32 nDeltaBytesPlayed = 0;
	if( dwCurrentPlayPos < m_uiLastPlayPos )
	{
		nDeltaBytesPlayed += ( m_uiBufferSize - m_uiLastPlayPos );
		nDeltaBytesPlayed += dwCurrentPlayPos;
	}
	else
	{
		nDeltaBytesPlayed += ( dwCurrentPlayPos - m_uiLastPlayPos );
	}

	// Update the bytes played.
	m_uiTotalPlayed += nDeltaBytesPlayed;
    m_uiLastPlayPos = dwCurrentPlayPos;

	// We have to use the bytes played read of the datasize of
	// the wave file because compressed sounds cannot report their exact
	// size without being decompressed.  As we stream, the bytes read out
	// out of the buffer will always lead the play position, until
	// we reach the end of the file.
	uint32 nWaveFileSize = m_pWaveFile->GetMaxBytesCopied( );
	if( IsLooping( ))
	{
		// Loop based on the filesize.
		m_uiTotalPlayed %= nWaveFileSize;
	}
	else
	{
		// Stop reading past the end of the file.
		m_uiTotalPlayed = Min( m_uiTotalPlayed, nWaveFileSize );
	}

	// Calculate the number of bytes we can write into the buffer.
	uint32 nBytesToWrite = 0;
	if( m_uiNextWriteOffset <= dwCurrentPlayPos )
	{
		nBytesToWrite = dwCurrentPlayPos - m_uiNextWriteOffset;
	}
	else
	{
		nBytesToWrite = ( m_uiBufferSize - m_uiNextWriteOffset ) + dwCurrentPlayPos;
	}

	// Check if there are no bytes to write out yet.
	if( nBytesToWrite == 0 )
	{
		return;
	}

	// Lock the DirectSound buffer
    if( FAILED( hr = m_pDSBuffer->Lock( m_uiNextWriteOffset, nBytesToWrite,
                                            &pDSLockedBuffer, &dwDSLockedBufferSize,
                                            &pDSLockedBuffer2, &dwDSLockedBufferSize2, 0L ) ) )
	{
		return;
	}

	if( dwDSLockedBufferSize > 0 )
	{
		ReadStreamIntoBuffer( pSoundSys, ( BYTE* )pDSLockedBuffer, dwDSLockedBufferSize );
	}
	if( dwDSLockedBufferSize2 > 0 )
	{
		ReadStreamIntoBuffer( pSoundSys, ( BYTE* )pDSLockedBuffer2, dwDSLockedBufferSize2 );
	}

    // Unlock the buffer, we don't need it anymore.
    m_pDSBuffer->Unlock( pDSLockedBuffer, dwDSLockedBufferSize, pDSLockedBuffer2, dwDSLockedBufferSize2 );

	m_uiNextWriteOffset += dwDSLockedBufferSize + dwDSLockedBufferSize2;
	m_uiNextWriteOffset %= m_uiBufferSize;

}

bool CStream::IsDone( )
{
	// Looping sounds are never done.
	if( IsLooping( ))
		return false;

	if( m_uiTotalPlayed < m_pWaveFile->GetMaxBytesCopied( ))
		return false;

	return true;
}

// fills in stream at current update location
uint32 CStream::FillBuffer( CDx8SoundSys* pSoundSys )
{
    HRESULT hr;
    VOID*   pDSLockedBuffer      = NULL; // Pointer to locked buffer memory
    DWORD   dwDSLockedBufferSize = 0;    // Size of the locked DirectSound buffer
	uint32 uiBytesRead = 0;

    // Lock the buffer down
    if( FAILED( hr = m_pDSBuffer->Lock( 0, m_uiBufferSize,
                                 &pDSLockedBuffer, &dwDSLockedBufferSize,
                                 NULL, NULL, 0L ) ) )
	{
		return 0;
	}

	uiBytesRead = pSoundSys->ReadStream( m_pWaveFile, (BYTE*) pDSLockedBuffer, dwDSLockedBufferSize );

    if( uiBytesRead < dwDSLockedBufferSize )
    {
        // Don't repeat the wav file, just fill in silence
        FillMemory( (BYTE*) pDSLockedBuffer + uiBytesRead,
                    dwDSLockedBufferSize - uiBytesRead,
                    (BYTE)(m_pWaveFile->get_format().bit_depth_ == 8 ? 128 : 0 ) );
		uiBytesRead = dwDSLockedBufferSize;
    }

    // Unlock the buffer, we don't need it anymore.
    m_pDSBuffer->Unlock( pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0 );

	// Setup the next write offset.
	m_uiNextWriteOffset += dwDSLockedBufferSize;
	m_uiNextWriteOffset %= m_uiBufferSize;

    return uiBytesRead;
}

void CStream::SetLooping( CDx8SoundSys* pSoundSys, bool bLoop )
{
	m_bLooping = bLoop;
}


BOOL CStream::SetCurrentPosition( DWORD dwStartOffset )
{
	if( m_pDSBuffer == NULL )
		return FALSE;

	m_pWaveFile->SeekFromStart( dwStartOffset );

	// Set the last play pos to zero, because doing this on a playing sound
	// seems to not really set it to exactly dwStartOffset anyway.
	m_nLastPlayPos = 0;

	m_uiNextWriteOffset = 0;
	m_uiLastPlayPos = 0;
	m_uiTotalPlayed = dwStartOffset;

	if( !FillBuffer( g_pSoundSys ))
		return FALSE;

	return TRUE;
}


#define NUM_PLAY_NOTIFICATIONS  16
#define STREAM_BUF_SECONDS	3


//-----------------------------------------------------------------------------
//	Streaming thread handling
//	Looping thread handling
//-----------------------------------------------------------------------------
void CDx8SoundSys::Thread_Func()
{
	while (true)
	{
		auto is_sleep = true;

		// Calculate how much time we are using up
		uint32 nTheadedTickCounts = ::timeGetTime();

		{
			MtRLockGuard lock_guard{m_cCS_SoundThread};

			if (is_mt_sample_loop_active_ || is_mt_streaming_active_)
			{
				is_sleep = false;

				// Stream section.
				if (is_mt_streaming_active_)
				{
					CountAdder cntAdd(&nTheadedTickCounts);

					// go through the streams and see if any are playing
					auto bSleep = true;

					for (auto pStream = m_pStreams; pStream; )
					{
						auto pNext = pStream->m_pNext;

						if (pStream->IsPlaying())
						{
							pStream->HandleUpdate(this);
							bSleep = false;
						}

						pStream = pNext;
					}

					// Go to sleep if we didn't do anything
					if (bSleep)
					{
						is_mt_streaming_active_ = false;
					}
				}

				// Loop point section.
				if (is_mt_sample_loop_active_)
				{
					CountAdder cntAdd(&nTheadedTickCounts);

					auto bSleep = true;
					auto pLink = CSample::m_lstSampleLoopHead.m_pNext;

					while (pLink != &CSample::m_lstSampleLoopHead)
					{
						auto pNext = pLink->m_pNext;
						auto pSample = static_cast<CSample*>(pLink->m_pData);

						if (pSample)
						{
							pSample->HandleLoop(this);
							bSleep = false;
						}

						pLink = pNext;
					}

					// Go to sleep if we didn't do anything
					if (bSleep)
					{
						is_mt_sample_loop_active_ = false;
					}
				}
			}
			else if (is_mt_shutdown_)
			{
				break;
			}
		}

		// Add up the tick counts.
		{
			MtLockGuard cThreadTickCountsProtection{m_cCS_ThreadedTickCounts};
			m_nThreadedTickCounts += ::timeGetTime() - nTheadedTickCounts;
		}

		if (is_sleep)
		{
			constexpr auto sleep_delay_ms = std::chrono::milliseconds{10};
			std::this_thread::sleep_for(sleep_delay_ms);
		}
	}
}

#define HANDLE_DS_ERROR( hr, val )
#define	DS_CHECK		HANDLE_DS_ERROR( m_hResult, DS_OK )

#define DECOMPRESSION_BUFFER_PAD	16384

//! CDx8SoundSys

//	===========================================================================
//	Incorporation of DSMStrm* required functionality

CSample* CDx8SoundSys::CreateBuffer( ul::WaveFormatEx* pWaveFormat, DWORD dwBufferSize, DWORD dwFlags )
{
	CSample* pSample;
	LT_MEM_TRACK_ALLOC(pSample = new CSample( ),LT_MEM_TYPE_SOUND);
	if( pSample->Init( m_hResult, m_pDirectSound, dwBufferSize, false, pWaveFormat ) )
	{
		return pSample;
	}
	delete pSample;
	return NULL;
}

void CDx8SoundSys::DestroyBuffer( CSample* pSoundBuffer )
{
	if( pSoundBuffer == NULL )
		return;
	pSoundBuffer->Term( );
	delete pSoundBuffer;
}

CDx8SoundSys::CDx8SoundSys( )
{
	Reset( );
}

CDx8SoundSys::~CDx8SoundSys( )
{
	Term( );
}

void CDx8SoundSys::Reset( )
{
	m_bCOMInitialized = false;
	m_bLocked = false;
	m_pDirectSound = NULL;

	// probably shouldn't need to lock this but ...
	m_pDSPrimaryBuffer = NULL;

	m_hResult = NULL;
	m_waveFormat = {};
	memset( &m_userPrefs, 0, ( MAX_USER_PREF_INDEX + 1 ) * sizeof( S32 ) );
	m_pStreams = NULL;
	m_iCur3DProvider = SOUND3DPROVIDERID_NONE;
	m_dwMinHardwareBuffers = 0;
#ifdef USE_EAX20_HARDWARE_FILTERS
	m_bSupportsEAX20Filtering = false;
#endif
	m_pKSPropertySet = NULL;
	g_iCurrentEAXDirectSetting = 0;

	is_mt_sample_loop_active_ = false;
	is_mt_streaming_active_ = false;
	is_mt_shutdown_ = false;

	m_cThread_Handle = {};

	m_nThreadedTickCounts = 0;
}

bool CDx8SoundSys::SupportsDS3DHardware()
{
	// make sure card has minimum # of 3D buffers available
	if ( m_dscaps.dwMaxHw3DAllBuffers >= m_dwMinHardwareBuffers )
		return true;
	else
		return false;
}



#ifdef USE_EAX20_HARDWARE_FILTERS

static bool QueryEAXSupport( IKsPropertySet& ksPropertySet, U32 uiQuery )
{
	ULONG ulSupport = 0;
	HRESULT hr = ksPropertySet.QuerySupport(DSPROPSETID_EAX_ListenerProperties, uiQuery, &ulSupport);
	if( FAILED( hr ))
		return false;

	if ( (ulSupport&(KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET)) == (KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET) )
	{
		//  m_dwSupport |= (DWORD)(1 << uiQuery);
		return true;
	}

	return false;
}

bool CDx8SoundSys::GetPropertySetForEAX( )
{
	// create a dummy buffer to get EAX property set
	ul::WaveFormatEx wave;
	wave = {};
    wave.tag_ = ul::WaveFormatTag::pcm;
    wave.channel_count_ = 1;
    wave.sample_rate_ = 22050;
    wave.bit_depth_ = 16;
    wave.block_align_ = wave.bit_depth_ / 8 * wave.channel_count_;
    wave.avg_bytes_per_sec_ = wave.sample_rate_ * wave.block_align_;

	DSBUFFERDESC dsbdesc;
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
    dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_STATIC|DSBCAPS_CTRL3D|DSBCAPS_LOCHARDWARE;
    dsbdesc.dwBufferBytes = 64;
    dsbdesc.lpwfxFormat = reinterpret_cast<LPWAVEFORMATEX>(&wave);

	// Create a soundbuffer so we can get the propertyset interface.
	LPDIRECTSOUNDBUFFER lpDSBuffer = NULL;
	HRESULT hRes = m_pDirectSound->CreateSoundBuffer(&dsbdesc, &lpDSBuffer, NULL );
	if( FAILED( hRes ))
	{
		return false;
	}

	// Clear out any old property set.
	if( m_pKSPropertySet )
	{
		m_pKSPropertySet->Release( );
		m_pKSPropertySet = NULL;
	}

	hRes = lpDSBuffer->QueryInterface( IID_IKsPropertySet, ( LPVOID* )&m_pKSPropertySet );

	// Don't need buffer anymore.
	if( lpDSBuffer )
	{
		// free up the buffer we don't need it anymore
		lpDSBuffer->Release();
		lpDSBuffer = NULL;
	}

	if( FAILED( hRes ))
		return false;

	return true;
}

void CDx8SoundSys::InitEAX20Filtering()
{

	// assume we can't do it
	m_bSupportsEAX20Filtering = false;

	// Get a propertyset interface.
	if( !GetPropertySetForEAX( ))
		return;

	bool bOk = true;

	bOk = bOk && QueryEAXSupport( *m_pKSPropertySet, DSPROPERTY_EAXLISTENER_NONE );
	bOk = bOk && QueryEAXSupport( *m_pKSPropertySet, DSPROPERTY_EAXLISTENER_ALLPARAMETERS );
	bOk = bOk && QueryEAXSupport( *m_pKSPropertySet, DSPROPERTY_EAXLISTENER_ROOM );
	bOk = bOk && QueryEAXSupport( *m_pKSPropertySet, DSPROPERTY_EAXLISTENER_ROOMHF );
	bOk = bOk && QueryEAXSupport( *m_pKSPropertySet, DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR );
	bOk = bOk && QueryEAXSupport( *m_pKSPropertySet, DSPROPERTY_EAXLISTENER_DECAYTIME );
	bOk = bOk && QueryEAXSupport( *m_pKSPropertySet, DSPROPERTY_EAXLISTENER_DECAYHFRATIO );
	bOk = bOk && QueryEAXSupport( *m_pKSPropertySet, DSPROPERTY_EAXLISTENER_REFLECTIONS );
	bOk = bOk && QueryEAXSupport( *m_pKSPropertySet, DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY );
	bOk = bOk && QueryEAXSupport( *m_pKSPropertySet, DSPROPERTY_EAXLISTENER_REVERB );
	bOk = bOk && QueryEAXSupport( *m_pKSPropertySet, DSPROPERTY_EAXLISTENER_REVERBDELAY );
	bOk = bOk && QueryEAXSupport( *m_pKSPropertySet, DSPROPERTY_EAXLISTENER_ENVIRONMENT );
	bOk = bOk && QueryEAXSupport( *m_pKSPropertySet, DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE );
	bOk = bOk && QueryEAXSupport( *m_pKSPropertySet, DSPROPERTY_EAXLISTENER_ENVIRONMENTDIFFUSION );
	bOk = bOk && QueryEAXSupport( *m_pKSPropertySet, DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF );
	bOk = bOk && QueryEAXSupport( *m_pKSPropertySet, DSPROPERTY_EAXBUFFER_DIRECT );

	if( bOk )
	{
		// set reverb off initially
		EAXLISTENERPROPERTIES props;
		props.lRoom = -10000;
		m_hResult = m_pKSPropertySet->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ROOM, NULL, 0, (void*)&props.lRoom, sizeof(FLOAT));
		if ( m_hResult != DS_OK )
			bOk = false;
	}

	// Passed all the tests, the card supports it.
	if( bOk )
	{
		m_bSupportsEAX20Filtering = true;
	}
}
#endif

bool CDx8SoundSys::SetSampleNotify( CSample* pSample, bool bEnable )
{
	// if disabling, free up previously used slot, if available
	if ( !bEnable )
	{
		// Remove if part of the list.
		if( !pSample->m_lnkLoopNotify.IsTiedOff( ))
		{
			// don't write to list if other thread is using it.
			MtRLockGuard cProtection(m_cCS_SoundThread);

			dl_Remove( &pSample->m_lnkLoopNotify );

		}
	}
	else
	{
		// Don't need to re-add if already part of the list.
		if( pSample->m_lnkLoopNotify.IsTiedOff( ))
		{
	  		if ( pSample->UsesLoopingBlock() )
  			{

				// don't write to array if other thread is using it.
				MtRLockGuard cProtection(m_cCS_SoundThread);

				// store sample for later use
				dl_Insert( CSample::m_lstSampleLoopHead.m_pPrev, &pSample->m_lnkLoopNotify );

				// Tell the looping thread to wake up
				is_mt_sample_loop_active_ = true;
			}
		}
	}
	return true;
}


#ifdef	HANDLE_DS_ERROR
#undef	HANDLE_DS_ERROR
#define HANDLE_DS_ERROR( hr, val )	\
	if( hr != val )					\
	{								\
		Term( );					\
		return false;				\
	}
#endif	// HANDLE_DS_ERROR

#define TIME_DELAY			10
#define TIME_RESOLUTION		0

bool CDx8SoundSys::Init( )
{
	LOG_OPEN;

	ltjs::AudioDecoder::initialize_current_thread();

	m_hResult = ::CoInitialize( NULL );
	if( m_hResult != S_OK && m_hResult != S_FALSE )
		return false;

	if( m_hResult == S_OK )
		m_bCOMInitialized = true;

	// Initialize our loop list.
	dl_TieOff( &CSample::m_lstSampleLoopHead );

	// create the direct sound object
	m_hResult = ::CoCreateInstance( CLSID_DirectSound8, NULL, CLSCTX_INPROC_SERVER, IID_IDirectSound8,
		( void** )&m_pDirectSound );
	m_pcLastError = LastError( );
	DS_CHECK

	// init it

	m_hResult = m_pDirectSound->Initialize( NULL );
	m_pcLastError = LastError( );
	DS_CHECK

  	// set the cooperative level
	HWND hWnd = FindWindow("LithTech", NULL);
	if(hWnd)
		m_hResult = m_pDirectSound->SetCooperativeLevel( hWnd, DSSCL_PRIORITY );
	else
		m_hResult = m_pDirectSound->SetCooperativeLevel( GetForegroundWindow( ), DSSCL_PRIORITY );
	m_pcLastError = LastError( );
	DS_CHECK

  	ZeroMemory( &m_dscaps, sizeof(DSCAPS) );
  	m_dscaps.dwSize = sizeof(DSCAPS);
  	m_hResult = m_pDirectSound->GetCaps( &m_dscaps );
	m_pcLastError = LastError( );
	DS_CHECK


	LT_MEM_TRACK_ALLOC(m_pStreams = new CStream( NULL, NULL ),LT_MEM_TYPE_SOUND);

	m_pcLastError = LastError( );
	DS_CHECK

	// Initialize the soundtick counter.
	m_nThreadedTickCounts = 0;

	// Create a thread to handle streaming
	m_cThread_Handle = Thread{std::bind(&CDx8SoundSys::Thread_Func, this)};
	m_pcLastError = LastError( );
	DS_CHECK

	g_pSoundSys = this;

	return true;

}

//
// CDx8SoundSys::HasOnBoardMemory
//
// Determines if hardware has on-board memory.  If it does, it is
// most likely an older ISA card.
//
bool CDx8SoundSys::HasOnBoardMemory( )
{
	return ( m_dscaps.dwMaxHwMixingStaticBuffers > 0 && m_dscaps.dwMaxHwMixingStreamingBuffers == 0 );
}

void CDx8SoundSys::Term( )
{
	// Shut down the thread
	is_mt_shutdown_ = true;

	if (m_cThread_Handle.joinable())
	{
		m_cThread_Handle.join();
	}

	m_cThread_Handle = {};

	while( m_pStreams != NULL )
	{
		CStream* pStream = m_pStreams;
		m_pStreams = m_pStreams->m_pNext;
		delete pStream;
	}

	LOG_CLOSE;


//	if( m_pDirectSound != NULL )
//	{
//		m_pDirectSound->Release( );
//		m_pDirectSound = NULL;
//	}

	if( m_bCOMInitialized )
		::CoUninitialize( );

	Reset( );

	g_pSoundSys = NULL;
}

void* CDx8SoundSys::GetDDInterface( uint uiDDInterfaceId )
{
	return NULL;
}

// system wide functions
void CDx8SoundSys::Lock( void )
{
	if( m_bLocked )
		return;
	m_bLocked = true;
}

void CDx8SoundSys::Unlock( void )
{
	if( !m_bLocked )
		return;
	m_bLocked = false;
}

//	===========================================================================
//	DONE...

S32	CDx8SoundSys::Startup( void )
{
	Shutdown( );

	return LS_OK;
}

void CDx8SoundSys::Shutdown( void )
{
	if ( m_pKSPropertySet )
	{
		m_pKSPropertySet->Release();
		m_pKSPropertySet = NULL;
	}
}

U32	CDx8SoundSys::MsCount( void )
{
//	Sleep( 20 );
	return ::timeGetTime( );
}

S32	CDx8SoundSys::SetPreference( U32 uiNumber, S32 siValue )
{
	if( uiNumber > MAX_USER_PREF_INDEX )
		return LS_ERROR;
	m_userPrefs[ uiNumber ] = siValue;
	return LS_OK;
}

S32	CDx8SoundSys::GetPreference( U32 uiNumber )
{
	if( uiNumber > MAX_USER_PREF_INDEX )
		return 0;
	return m_userPrefs[ uiNumber ];
}

void CDx8SoundSys::MemFreeLock( void* ptr )
{
	delete [] ptr;
}

void* CDx8SoundSys::MemAllocLock( U32 uiSize )
{
	void* p;
	LT_MEM_TRACK_ALLOC(p = (void*)(new uint8[uiSize]),	LT_MEM_TYPE_SOUND);
	return p;
}

const char* CDx8SoundSys::LastError( void )
{
	return ConvertDSErrorToString( m_hResult );
}


#ifdef	HANDLE_DS_ERROR
#undef	HANDLE_DS_ERROR
#define HANDLE_DS_ERROR( hr, val )	\
	if( hr != val )					\
	{								\
		return LS_ERROR;			\
	}
#endif	// HANDLE_DS_ERROR

// digital sound driver functions
S32	CDx8SoundSys::WaveOutOpen( LHDIGDRIVER& phDriver, PHWAVEOUT& pphWaveOut, const S32 siDeviceId, const ul::WaveFormatEx& pWaveFormat )
{

	WaveOutClose( m_pDSPrimaryBuffer );

	// Set up DSBUFFERDESC structure.

	DSBUFFERDESC dsbdesc;
	memset( &dsbdesc, 0, sizeof( DSBUFFERDESC ) );

	dsbdesc.dwSize = sizeof( DSBUFFERDESC );
	dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME;
	dsbdesc.dwBufferBytes = 0;
	dsbdesc.lpwfxFormat = NULL; // Must be NULL for primary buffers.

	// lock access to primary buffer address
	m_hResult = m_pDirectSound->CreateSoundBuffer( &dsbdesc, &m_pDSPrimaryBuffer, NULL );

	m_pcLastError = LastError( );
	DS_CHECK

    // Set primary buffer to desired format.

    m_hResult = m_pDSPrimaryBuffer->SetFormat( reinterpret_cast<LPCWAVEFORMATEX>(&pWaveFormat) );
	m_pcLastError = LastError( );
	DS_CHECK

	phDriver = ( LHDIGDRIVER )m_pDSPrimaryBuffer;

	m_waveFormat = pWaveFormat;

	// see if we can set up EAX filtering
#ifdef USE_EAX20_HARDWARE_FILTERS
	InitEAX20Filtering();
#endif

    return LS_OK;

}

void CDx8SoundSys::WaveOutClose( LHDIGDRIVER hDriver )
{
	LPDIRECTSOUNDBUFFER pDSPrimaryBuffer = ( LPDIRECTSOUNDBUFFER )hDriver;

	if( m_pDSPrimaryBuffer == pDSPrimaryBuffer  )
	{
		m_pDSPrimaryBuffer = NULL;
	}

	if( pDSPrimaryBuffer != NULL )
		pDSPrimaryBuffer->Release( );
}

#define MAX_MASTER_VOLUME	127.0f
#define MIN_MASTER_VOLUME	1.0f
#define DEL_MASTER_VOLUME	( float )( MAX_MASTER_VOLUME - MIN_MASTER_VOLUME )

#define DSBVOLUME_DEL		( float )( DSBVOLUME_MAX - DSBVOLUME_MIN )

#define LOG_ATTENUATION		( float )sqrt( MAX_MASTER_VOLUME )
#define LOG_INVATTENUATION	( 1.0f / LOG_ATTENUATION )

#define CONVERT_LIN_VOL_TO_LOG_VOL( linVol, logVol )	\
	{	\
		float t = ( 1.0f - ( MIN_MASTER_VOLUME / ( float )linVol ) ) * ( MAX_MASTER_VOLUME / DEL_MASTER_VOLUME );	\
		t = ( float )pow( t, LOG_ATTENUATION );	\
		logVol = DSBVOLUME_MIN + ( long )( t * DSBVOLUME_DEL );	\
	}	\

#define CONVERT_LOG_VOL_TO_LIN_VOL( logVol, linVol )	\
	{	\
		float t = ( float )( logVol - DSBVOLUME_MIN ) / DSBVOLUME_DEL;	\
		t = ( float )pow( t, LOG_INVATTENUATION );	\
		linVol = ( S32 )( ( MAX_MASTER_VOLUME * MIN_MASTER_VOLUME ) / ( MAX_MASTER_VOLUME - ( t * DEL_MASTER_VOLUME ) ) );	\
	}	\

void CDx8SoundSys::SetDigitalMasterVolume( LHDIGDRIVER hDig, S32 siMasterVolume )
{
	LPDIRECTSOUNDBUFFER pDSPrimaryBuffer = ( LPDIRECTSOUNDBUFFER )hDig;

	if( pDSPrimaryBuffer == NULL )
		return;

	if( siMasterVolume < ( S32 )MIN_MASTER_VOLUME )
		siMasterVolume = ( S32 )MIN_MASTER_VOLUME;

	if( siMasterVolume > ( S32 )MAX_MASTER_VOLUME )
		siMasterVolume = ( S32 )MAX_MASTER_VOLUME;

	long lDSMasterVolume;
	CONVERT_LIN_VOL_TO_LOG_VOL( siMasterVolume, lDSMasterVolume );

	m_hResult = pDSPrimaryBuffer->SetVolume( lDSMasterVolume );
	m_pcLastError = LastError( );
}

S32	CDx8SoundSys::GetDigitalMasterVolume( LHDIGDRIVER hDig )
{
	LPDIRECTSOUNDBUFFER pDSPrimaryBuffer = ( LPDIRECTSOUNDBUFFER )hDig;

	if( pDSPrimaryBuffer == NULL )
		return 0;

	long lDSMasterVolume = 0;
	m_hResult = pDSPrimaryBuffer->GetVolume( &lDSMasterVolume );
	m_pcLastError = LastError( );

	S32 siMasterVolume;
	CONVERT_LOG_VOL_TO_LIN_VOL( lDSMasterVolume, siMasterVolume );

	return siMasterVolume;
}

//	===========================================================================
//	TO DO...

S32	CDx8SoundSys::DigitalHandleRelease( LHDIGDRIVER hDriver )
{
	return 0;
}

S32	CDx8SoundSys::DigitalHandleReacquire( LHDIGDRIVER hDriver )
{
	return 0;
}

#ifdef USE_EAX20_HARDWARE_FILTERS
bool CDx8SoundSys::SetEAX20Filter( const bool bEnable, const LTSOUNDFILTERDATA& pFilterData )
{
	// Check if we support filtering.
	if( !m_bSupportsEAX20Filtering )
		return false;

	LTFILTERREVERB* pLTReverb;

	// for EAX 2.0, this should be a reverb
	if ( pFilterData.uiFilterType != FilterReverb )
		return false;

	pLTReverb = (LTFILTERREVERB*) pFilterData.pSoundFilter;

	// Should have this setup the first time we allocated a 3d sound buffer.
	if( !m_pKSPropertySet )
		return false;

	// if we want to turn off filter, just set room property to minimum
	if ( !bEnable )
	{
		EAXLISTENERPROPERTIES props;
		props.lRoom = -10000;
		m_hResult = m_pKSPropertySet->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ROOM, NULL, 0, (void*)&props.lRoom, sizeof(FLOAT));
		m_pcLastError = LastError( );

		if ( m_hResult != DS_OK )
		{
			return false;
		}
		g_iCurrentEAXDirectSetting = pLTReverb->lDirect = 0;
	}
	else
	{
		EAXLISTENERPROPERTIES props;
		uint32 uiFilterParamFlags;

		// First check if they are setting the environment property.  We have to set
		// this first so it sets up all the defaults.  Then we get the defaults and modify
		// them the way we want.
		uiFilterParamFlags = pFilterData.pSoundFilter->uiFilterParamFlags;
		if( uiFilterParamFlags & SET_REVERB_ENVIRONMENT )
		{
			unsigned long nEnvironment = pLTReverb->lEnvironment;

			m_hResult = m_pKSPropertySet->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ENVIRONMENT, NULL, 0, (void*)&nEnvironment, sizeof(nEnvironment));
			m_pcLastError = LastError( );
			if ( m_hResult != DS_OK )
			{
				return false;
			}
		}


		// [KLS 9/1/02] There are some ordering issues so get the current value of the
		// listener properties and set the ones that have changed.

		// Get the current settings...
		unsigned long uPropSize;
		m_hResult = m_pKSPropertySet->Get(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ALLPARAMETERS, NULL, 0, (void*)&props, sizeof(EAXLISTENERPROPERTIES), &uPropSize);
		m_pcLastError = LastError( );
		if ( m_hResult != DS_OK )
		{
			return false;
		}

		if ( uiFilterParamFlags & SET_REVERB_ROOM )
		{
			props.lRoom = pLTReverb->lRoom;
		}
		if ( uiFilterParamFlags & SET_REVERB_ROOMHF )
		{
			props.lRoomHF = pLTReverb->lRoomHF;
		}
		if ( uiFilterParamFlags & SET_REVERB_ROOMROLLOFFFACTOR )
		{
			props.flRoomRolloffFactor = pLTReverb->fRoomRolloffFactor;
		}
		if ( uiFilterParamFlags & SET_REVERB_DECAYTIME )
		{
			props.flDecayTime = pLTReverb->fDecayTime;
		}
		if ( uiFilterParamFlags & SET_REVERB_DECAYHFRATIO )
		{
			props.flDecayHFRatio = pLTReverb->fDecayHFRatio;
		}
		if ( uiFilterParamFlags & SET_REVERB_REFLECTIONS )
		{
			props.lReflections = pLTReverb->lReflections;
		}
		if ( uiFilterParamFlags & SET_REVERB_REFLECTIONSDELAY )
		{
			props.flReflectionsDelay = pLTReverb->fReflectionsDelay;
		}
		if ( uiFilterParamFlags & SET_REVERB_REVERB )
		{
			props.lReverb = pLTReverb->lReverb;
		}
		if ( uiFilterParamFlags & SET_REVERB_REVERBDELAY )
		{
			props.flReverbDelay = pLTReverb->fReverbDelay;
		}
		if ( uiFilterParamFlags & SET_REVERB_SIZE )
		{
			props.flEnvironmentSize = pLTReverb->fSize;
		}
		if ( uiFilterParamFlags & SET_REVERB_DIFFUSION )
		{
			props.flEnvironmentDiffusion = pLTReverb->fDiffusion;
		}
		if ( uiFilterParamFlags & SET_REVERB_AIRABSORPTIONHF )
		{
			props.flAirAbsorptionHF = pLTReverb->fAirAbsorptionHF;
		}
		if ( uiFilterParamFlags & SET_REVERB_DIRECT )
		{
		  	// store directsetting for any sounds we create later
			g_iCurrentEAXDirectSetting = pLTReverb->lDirect;
		}

		// Okay, set whatever changed...

		m_hResult = m_pKSPropertySet->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ALLPARAMETERS, NULL, 0, (void*)&props, sizeof(EAXLISTENERPROPERTIES));
		m_pcLastError = LastError( );
		if ( m_hResult != DS_OK )
		{
			return false;
		}
	}

	return true;
}

bool CDx8SoundSys::SetEAX20BufferSettings( LH3DSAMPLE h3DSample, const LTSOUNDFILTERDATA& pFilterData )
{
	uint32 uiFilterParamFlags;
	EAXBUFFERPROPERTIES soundProps;
	LTFILTERREVERB* pLTReverb;
	C3DSample* p3DSample = (C3DSample*) h3DSample;
	CSample* pSample = &p3DSample->m_sample;

	if( !m_bSupportsEAX20Filtering )
		return false;

	if ( pSample == NULL )
		return false;

	if ( pSample->m_pDSBuffer == NULL )
		return false;

	// for EAX 2.0, this should be a reverb
	if ( pFilterData.uiFilterType != FilterReverb )
		return false;

	// now set any of the parameters
	uiFilterParamFlags = pFilterData.pSoundFilter->uiFilterParamFlags;
	pLTReverb = (LTFILTERREVERB*) pFilterData.pSoundFilter;

	LPKSPROPERTYSET pKSPropertySet = NULL;
  	m_hResult = pSample->m_pDSBuffer->QueryInterface(IID_IKsPropertySet, (LPVOID *)&pKSPropertySet);
	m_pcLastError = LastError( );
	if ( m_hResult != DS_OK )
		return false;

	if ( uiFilterParamFlags & SET_REVERB_DIRECT )
	{
		// set the direct attenuation, this is implemented when we actually allocate a sound.
		soundProps.lDirect = pLTReverb->lDirect;
		m_hResult = pKSPropertySet->Set(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_DIRECT, NULL, 0, (void*)&soundProps.lDirect, sizeof(unsigned long));
		m_pcLastError = LastError( );
		if ( m_hResult != DS_OK )
			return false;

		pKSPropertySet->Release();
	}

	return true;
}

bool CDx8SoundSys::SupportsEAX20Filter()
{
	return m_bSupportsEAX20Filtering;
}
#endif

void CDx8SoundSys::Set3DProviderMinBuffers( U32 uiMinBuffers )
{
	m_dwMinHardwareBuffers = uiMinBuffers;
}

// 3d sound provider functions
S32 CDx8SoundSys::Open3DProvider( LHPROVIDER hLib )
{
	// assume failure
	bool bRetVal = LTFALSE;
	m_iCur3DProvider = SOUND3DPROVIDERID_NONE;

	// make sure this is a legit provider
	switch ( hLib )
	{
	case SOUND3DPROVIDERID_DS3D_SOFTWARE:
		m_iCur3DProvider = hLib;
		bRetVal = LTTRUE;
		break;
	case SOUND3DPROVIDERID_DS3D_HARDWARE:
		if ( SupportsDS3DHardware() )
		{
			m_iCur3DProvider = hLib;
			bRetVal = LTTRUE;
		}
		break;
	default:
		break;
	}
	return bRetVal;
}


void CDx8SoundSys::Close3DProvider( LHPROVIDER hLib )
{
	// set back to no provider
	m_iCur3DProvider = SOUND3DPROVIDERID_NONE;
}

void CDx8SoundSys::Set3DProviderPreference( LHPROVIDER hLib, const char* sName, const void* pVal )
{
}

void CDx8SoundSys::Get3DProviderAttribute( LHPROVIDER hLib, const char* sName, void* pVal )
{
	int *pnVal = (int*) pVal;
    if (strcmp(sName, "Max samples") == 0)
	{
		switch ( hLib )
		{
		case SOUND3DPROVIDERID_DS3D_SOFTWARE:
			*pnVal =  MAX_3D_SOFTWARE_SAMPLES;
			break;
		case SOUND3DPROVIDERID_DS3D_HARDWARE:
			*pnVal = m_dscaps.dwMaxHw3DAllBuffers;
			break;
		}
	}

}


S32	CDx8SoundSys::Enumerate3DProviders( LHPROENUM& phNext, LHPROVIDER& phDest, const char*& psName)
{
	int nCur = phNext;
	phNext += 1;

	if( nCur == NUM_PROVIDERS )
	{
		psName = NULL;
		phDest = 0;
		return 0;
	}

	switch ( nCur )
	{
	case PROVIDER_DS3D_SOFTWARE:
		// always allow software support
		psName = SOUND3DPROVIDERNAME_DS3D_SOFTWARE;
		phDest = SOUND3DPROVIDERID_DS3D_SOFTWARE;
		break;
	case PROVIDER_DS3D_HARDWARE:
		// check for hardware support
		if ( SupportsDS3DHardware() )
		{
			psName = SOUND3DPROVIDERNAME_DS3D_HARDWARE;
			phDest = SOUND3DPROVIDERID_DS3D_HARDWARE;
		}
		else
		{
			phDest = 0;
		}
		break;
	case PROVIDER_DS3D_DEFAULT:
		psName = SOUND3DPROVIDERNAME_DS3D_DEFAULT;
		phDest = SOUND3DPROVIDERID_DS3D_DEFAULT;
		break;
	}

	return 1;
}

//	===========================================================================
//	DONE...

// 3d listener functions
LH3DPOBJECT	CDx8SoundSys::Open3DListener( LHPROVIDER hLib )
{
	C3DListener* p3DListener;
	LT_MEM_TRACK_ALLOC(p3DListener = new C3DListener,LT_MEM_TYPE_SOUND);
	p3DListener->Init( m_hResult, m_pDSPrimaryBuffer );
	return p3DListener;
}

void CDx8SoundSys::Close3DListener( LH3DPOBJECT hListener )
{
	if( hListener == NULL )
		return;

	C3DListener* p3DListener = ( C3DListener* )hListener;
	delete p3DListener;
}

void CDx8SoundSys::SetListenerDoppler( LH3DPOBJECT hListener, float fDoppler )
{
	C3DListener* p3DListener = ( C3DListener* ) hListener;

	// set the Doppler factor
	if ( fDoppler < DS3D_MINDOPPLERFACTOR )
		fDoppler = DS3D_MINDOPPLERFACTOR;
	else if ( fDoppler > DS3D_MAXDOPPLERFACTOR )
		fDoppler = DS3D_MAXDOPPLERFACTOR;

	p3DListener->m_fDopplerSetting = fDoppler;

	p3DListener->m_pDS3DListener->SetDopplerFactor( fDoppler, DS3D_DEFERRED );
}

//
//---- Submit deferred setting all at once ( performance gain )
//
void CDx8SoundSys::CommitDeferred()
{
	LPDIRECTSOUND3DLISTENER8 pDS3DListener; 

	if ( m_pDSPrimaryBuffer )
	{
		if (m_pDSPrimaryBuffer->QueryInterface( IID_IDirectSound3DListener, ( void** )&pDS3DListener ) == DS_OK )
		{

			pDS3DListener->CommitDeferredSettings();
			pDS3DListener->Release();
		}
	}
}


// 3d sound object functions
void CDx8SoundSys::Set3DPosition( LH3DPOBJECT hObj, float fX, float fY, float fZ)
{
	if( hObj == NULL )
		return;

	I3DObject* p3DObject = ( I3DObject* )hObj;

    LTVector position(fX, fY, fZ);
	p3DObject->SetPosition( position );
}

void CDx8SoundSys::Set3DVelocityVector( LH3DPOBJECT hObj, float fDX_per_s, float fDY_per_s, float fDZ_per_s )
{
	if( hObj == NULL )
		return;

	I3DObject* p3DObject = ( I3DObject* )hObj;

    LTVector velocity(fDX_per_s, fDY_per_s, fDZ_per_s);
	p3DObject->SetVelocity( velocity );
}

void CDx8SoundSys::Set3DOrientation( LH3DPOBJECT hObj, float fX_face, float fY_face, float fZ_face, float fX_up, float fY_up, float fZ_up )
{
	if( hObj == NULL )
		return;

	I3DObject* p3DObject = ( I3DObject* )hObj;

    LTVector up_vector(fX_up, fY_up, fZ_up);
    LTVector face_vector(fX_face, fY_face, fZ_face);

	p3DObject->SetOrientation( up_vector, face_vector );
}

void CDx8SoundSys::Set3DUserData( LH3DPOBJECT hObj, U32 uiIndex, S32 siValue )
{
	if( hObj == NULL || uiIndex > MAX_USER_DATA_INDEX )
		return;

	I3DObject* p3DObject = ( I3DObject* )hObj;
	p3DObject->m_userData[ uiIndex ] = siValue;
}

void CDx8SoundSys::Get3DPosition( LH3DPOBJECT hObj, float& pfX, float& pfY, float& pfZ)
{
	if( hObj == NULL )
		return;

	I3DObject* p3DObject = ( I3DObject* )hObj;

	pfX = p3DObject->m_position.x;
	pfY = p3DObject->m_position.y;
	pfZ = p3DObject->m_position.z;
}

void CDx8SoundSys::Get3DVelocity( LH3DPOBJECT hObj, float& pfDX_per_ms, float& pfDY_per_ms, float& pfDZ_per_ms )
{
	if( hObj == NULL )
		return;

	I3DObject* p3DObject = ( I3DObject* )hObj;

	pfDX_per_ms = p3DObject->m_velocity.x;
	pfDY_per_ms = p3DObject->m_velocity.y;
	pfDZ_per_ms = p3DObject->m_velocity.z;
}

void CDx8SoundSys::Get3DOrientation( LH3DPOBJECT hObj, float& pfX_face, float& pfY_face, float& pfZ_face, float& pfX_up, float& pfY_up, float& pfZ_up )
{
	if( hObj == NULL )
		return;

	I3DObject* p3DObject = ( I3DObject* )hObj;

	pfX_up = p3DObject->m_up.x;
	pfY_up = p3DObject->m_up.y;
	pfZ_up = p3DObject->m_up.z;

	pfX_face = p3DObject->m_face.x;
	pfY_face = p3DObject->m_face.y;
	pfZ_face = p3DObject->m_face.z;
}

S32	CDx8SoundSys::Get3DUserData( LH3DPOBJECT hObj, U32 uiIndex )
{
	if( hObj == NULL || uiIndex > MAX_USER_DATA_INDEX )
		return 0;

	I3DObject* p3DObject = ( I3DObject* )hObj;
	return p3DObject->m_userData[ uiIndex ];
}

// 3d sound sample functions
LH3DSAMPLE CDx8SoundSys::Allocate3DSampleHandle( LHPROVIDER hLib )
{
	C3DSample* p3DSample;
	LT_MEM_TRACK_ALLOC(p3DSample = new C3DSample,LT_MEM_TYPE_SOUND);

    // Set up wave format structure.
	p3DSample->m_sample.m_waveFormat = m_waveFormat;
	// 3d sounds must be mono
	CSample* pSample = &p3DSample->m_sample;
	pSample->m_waveFormat.channel_count_ = 1;
	pSample->m_waveFormat.block_align_ = ( pSample->m_waveFormat.channel_count_ * pSample->m_waveFormat.bit_depth_ ) >> 3;
	pSample->m_waveFormat.avg_bytes_per_sec_ = pSample->m_waveFormat.sample_rate_ * pSample->m_waveFormat.block_align_;


    // Set up DSBUFFERDESC structure.

    memset( &p3DSample->m_sample.m_dsbDesc, 0, sizeof( DSBUFFERDESC ) ); // Zero it out.

    p3DSample->m_sample.m_dsbDesc.dwSize = sizeof( DSBUFFERDESC );
    p3DSample->m_sample.m_dsbDesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_MUTE3DATMAXDISTANCE;

	// if hardware only set appropriate flag
	if ( m_iCur3DProvider == SOUND3DPROVIDERID_DS3D_HARDWARE )
	{
	    p3DSample->m_sample.m_dsbDesc.dwFlags |= DSBCAPS_LOCHARDWARE;

		if( HasOnBoardMemory( ))
			p3DSample->m_sample.m_dsbDesc.dwFlags |= DSBCAPS_STATIC;
	}
	else if ( m_iCur3DProvider == SOUND3DPROVIDERID_DS3D_SOFTWARE )
		p3DSample->m_sample.m_dsbDesc.dwFlags |= DSBCAPS_LOCSOFTWARE;

    // Create buffer.
	p3DSample->Init( m_hResult, m_pDirectSound, m_waveFormat.sample_rate_, &pSample->m_waveFormat, NULL );


	// Make sure we have our propertyset interface setup.
	if( m_bSupportsEAX20Filtering && !m_pKSPropertySet )
	{
		HRESULT hRes = p3DSample->m_sample.m_pDSBuffer->QueryInterface( IID_IKsPropertySet, ( LPVOID* )&m_pKSPropertySet );
		if( FAILED( hRes ))
		{
			delete p3DSample;
			return NULL;
		}
	}

	return p3DSample;
}

void CDx8SoundSys::Release3DSampleHandle( LH3DSAMPLE hS )
{
	if( hS == NULL )
		return;

	MtRLockGuard cProtection(m_cCS_SoundThread);

	C3DSample* p3DSample = ( C3DSample* )hS;
	delete p3DSample;
}

// pause playback at current position
void CDx8SoundSys::Stop3DSample( LH3DSAMPLE hS )
{
	if( hS == NULL )
		return;

	C3DSample* p3DSample = ( C3DSample* )hS;
	p3DSample->m_sample.Restore( );

	p3DSample->m_sample.Stop( false );
	p3DSample->m_status = LS_STOPPED;

	// stop any sample notification
	SetSampleNotify( &p3DSample->m_sample, false );
	m_pcLastError = LastError( );
}

// start playback at beginning position
void CDx8SoundSys::Start3DSample( LH3DSAMPLE hS )
{
	if( hS == NULL )
		return;

	C3DSample* p3DSample = ( C3DSample* )hS;
	p3DSample->m_sample.Restore( );

	m_hResult = p3DSample->m_sample.Stop( true );
	m_hResult = p3DSample->m_sample.Play( );
	p3DSample->m_status = LS_PLAYING;
	m_pcLastError = LastError( );
}

// continue playback from current position
void CDx8SoundSys::Resume3DSample( LH3DSAMPLE hS )
{
	if( hS == NULL )
		return;

	C3DSample* p3DSample = ( C3DSample* )hS;
	p3DSample->m_sample.Restore( );

	p3DSample->m_sample.Play( );
	p3DSample->m_status = LS_PLAYING;
	m_pcLastError = LastError( );
}

// terminate playback and reset position
void CDx8SoundSys::End3DSample( LH3DSAMPLE hS )
{
	if( hS == NULL )
		return;

	C3DSample* p3DSample = ( C3DSample* )hS;
	p3DSample->m_sample.Restore( );

	if( p3DSample->m_sample.m_pDSBuffer != NULL )
	{
		m_hResult = p3DSample->m_sample.Stop( true );
	}
	p3DSample->m_status = LS_DONE;

	SetSampleNotify( &p3DSample->m_sample, false );
	m_pcLastError = LastError( );
}


S32 CDx8SoundSys::Init3DSampleFromAddress( LH3DSAMPLE hS, const void* pStart, const U32 uiLen, const ul::WaveFormatEx& pWaveFormat, const S32 siPlaybackRate, const LTSOUNDFILTERDATA* pFilterData )
{
//	LOG_WRITE( g_pLogFile, "SetSampleAddress( %x, %x, %d )\n", hS, pStart, uiLen );

	if( hS == NULL || pStart == NULL || uiLen == 0 )
		return LTFALSE;

	C3DSample* p3DSample = ( C3DSample* )hS;
	CSample* pSample = &p3DSample->m_sample;

	// Modify the pitch.
	auto waveFormat = pWaveFormat;
	waveFormat.sample_rate_ = siPlaybackRate;
	waveFormat.avg_bytes_per_sec_ = waveFormat.block_align_ * waveFormat.sample_rate_;

	if ( !p3DSample->Init( m_hResult, m_pDirectSound, uiLen, &waveFormat, pFilterData ) )
		return LTFALSE;

	pSample->m_pSoundData = const_cast<void*>(pStart);
	pSample->m_uiSoundDataLen = p3DSample->m_sample.m_dsbDesc.dwBufferBytes;

	if( !pSample->Fill( ))
	{
		p3DSample->Term( );
		return LTFALSE;
	}

	// set up new looping, if appropriate
	if  ( !SetSampleNotify( pSample, true ) )
		return LTFALSE;

	return LTTRUE;
}

S32	CDx8SoundSys::Init3DSampleFromFile(
	LH3DSAMPLE hS,
	const void* pFile_image,
	const S32 siBlock,
	const S32 siPlaybackRate,
	const LTSOUNDFILTERDATA* pFilterData)
{
	if (!hS)
	{
		return false;
	}

	const auto wave_size = ltjs::AudioUtils::extract_wave_size(pFile_image);

	if (wave_size == 0)
	{
		return false;
	}

	auto memory_stream = ul::MemoryStream{pFile_image, wave_size, ul::Stream::OpenMode::read};

	if (!audio_decoder_.open(&memory_stream))
	{
		return false;
	}

	auto wave_format_ex = audio_decoder_.get_wave_format_ex();

	// if we have more than one channel, we fail
	// 3D sounds can't be stereo
	if (wave_format_ex.channel_count_ != 1)
	{
		return false;
	}

	auto p3DSample = static_cast<C3DSample*>(hS);
	auto pSample = &p3DSample->m_sample;

	const auto max_decoded_size = audio_decoder_.get_data_size();

	if (!audio_decoder_.is_pcm())
	{
		auto pucUncompressedSampleData = std::make_unique<std::uint8_t[]>(max_decoded_size);

		const auto decoded_size = audio_decoder_.decode(pucUncompressedSampleData.get(), max_decoded_size);

		if (decoded_size <= 0)
		{
			return false;
		}

		if (!Init3DSampleFromAddress(
			hS,
			pucUncompressedSampleData.get(),
			decoded_size,
			wave_format_ex,
			siPlaybackRate,
			pFilterData))
		{
			return false;
		}

		pSample->m_bAllocatedSoundData = true;

		static_cast<void>(pucUncompressedSampleData.release());
	}
	else
	{
		// Modify the pitch.
		wave_format_ex.sample_rate_ = siPlaybackRate;
		wave_format_ex.avg_bytes_per_sec_ = wave_format_ex.block_align_ * wave_format_ex.sample_rate_;

		if (!pSample->Init(m_hResult, m_pDirectSound, max_decoded_size, false, &wave_format_ex, pFilterData))
		{
			return false;
		}

		if (!SetSampleNotify(pSample, true))
		{
			return false;
		}
	}

	return true;
}

S32	CDx8SoundSys::Get3DSampleVolume( LH3DSAMPLE hS )
{
//	LOG_WRITE( g_pLogFile, "GetSampleVolume( %x )\n", hS );

	if( hS == NULL )
		return 0;

	C3DSample* p3DSample = ( C3DSample* ) hS;
	CSample* pSample = &p3DSample->m_sample;

	pSample->Restore( );

	long lDSVolume = 0;
	m_hResult = pSample->m_pDSBuffer->GetVolume( &lDSVolume );
	m_pcLastError = LastError( );

	S32 siVolume;
	CONVERT_LOG_VOL_TO_LIN_VOL( lDSVolume, siVolume );

	return siVolume;
}

void CDx8SoundSys::Set3DSampleVolume( LH3DSAMPLE hS, const S32 siVolume )
{
	if( hS == NULL )
		return;

	C3DSample* p3DSample = ( C3DSample* )hS;
	p3DSample->m_sample.Restore( );

	auto volume = siVolume;

	if( volume < ( S32 )MIN_MASTER_VOLUME )
		volume = ( S32 )MIN_MASTER_VOLUME;

	if( volume > ( S32 )MAX_MASTER_VOLUME )
		volume = ( S32 )MAX_MASTER_VOLUME;

	long lDSVolume;
	CONVERT_LIN_VOL_TO_LOG_VOL( volume, lDSVolume );
	m_hResult = p3DSample->m_sample.m_pDSBuffer->SetVolume( lDSVolume );
	m_pcLastError = LastError( );
}

//	===========================================================================
//	TO DO...

uint32 CDx8SoundSys::Get3DSampleStatus( LH3DSAMPLE hS )
{
	C3DSample* p3DSample = (C3DSample*) hS;
	return ( p3DSample->m_sample.IsPlaying( ) ? LS_PLAYING : LS_STOPPED );
}

void CDx8SoundSys::Set3DSampleMsPosition( LHSAMPLE hS, const sint32 siMilliseconds )
{
	if( hS == NULL )
		return;

	C3DSample* p3DSample = ( C3DSample* )hS;
	SetSampleMsPosition(( LHSAMPLE )&p3DSample->m_sample, siMilliseconds );
}



//	===========================================================================
//	DONE...

S32	CDx8SoundSys::Set3DSampleInfo( LH3DSAMPLE hS, const LTSOUNDINFO& pInfo )
{

	if( hS == NULL )
		return LS_ERROR;

	C3DSample* p3DSample = ( C3DSample* )hS;

    // Set up wave format structure.

    p3DSample->m_sample.m_waveFormat.channel_count_ = ( U16 )pInfo.channels;
    p3DSample->m_sample.m_waveFormat.sample_rate_ = pInfo.rate;
    p3DSample->m_sample.m_waveFormat.bit_depth_ = ( U16 )pInfo.bits;

    // Create buffer.

	if( !p3DSample->Init( m_hResult, m_pDirectSound, pInfo.samples, &p3DSample->m_sample.m_waveFormat, NULL ) )
		return LS_ERROR;

	return LS_OK;

}


// what attenuation factor is approximately silent
// this represents 1/32 of the maximum volume
#define MAX_ATTENUATION	32
void CDx8SoundSys::Set3DSampleDistances( LH3DSAMPLE hS, float fMax_dist, float fMin_dist )
{
	if( hS == NULL )
		return;

	C3DSample* p3DSample = ( C3DSample* )hS;


/*
	// In our system fMin_dist and fMax_dist represent an inner and outer radius of
	// audibility with the sound at full strength inside the inner radius, and inaudible
	// outside of the outer radius, with linear attenuation between the inner and outer
	// DirectSound  uses a different scheme, so this is to allow the ability to use an
	// inner and outer radius with DirectSound;
	// We essentially will calculate everything with respective to the inner radius.
	// If inside, we'll set it to max volume.
	// If outside the max, it'll be silent
	// Between the inner and outer, it'll attenuate with a logarithmic curve.
	// store the inner radius for later use
	p3DSample->SetRadiusData( fMin_dist, fMax_dist );

	// create a relative minimum distance that will attenuate the way we want
	float fRelDist = fMax_dist - fMin_dist;
	fMin_dist = fRelDist / MAX_ATTENUATION;
	p3DSample->SetDSMinDist( fMin_dist );

	m_hResult = p3DSample->m_pDS3DBuffer->SetMinDistance( fMin_dist, DS3D_DEFERRED );
	m_hResult = p3DSample->m_pDS3DBuffer->SetMaxDistance( fRelDist, DS3D_DEFERRED );
*/
	// We want a linear curve on attenuation as the player moves
	// away from the object.  Unfortunately, DirectSound doesn't allow this, so
	// we're going to fake it out.  Basically, we set the minimum and the maximum
	// distance to the outer radius, so the sound is always at the full volume
	// but with the right pan and filter calculations for 3D (we let DS do that
	// part).  We'll control the volume separately ourselves, so we can get
	// our linear curve.
  	m_hResult = p3DSample->m_pDS3DBuffer->SetMinDistance( fMax_dist, DS3D_DEFERRED );
	m_hResult = p3DSample->m_pDS3DBuffer->SetMaxDistance( fMax_dist, DS3D_DEFERRED );
}

//	===========================================================================
//	TO DO...

void CDx8SoundSys::Set3DSamplePreference( LH3DSAMPLE hSample, const char* sName, const void* pVal )
{
}

void CDx8SoundSys::Set3DSampleLoopBlock( LH3DSAMPLE hS, const S32 siLoop_start_offset, const S32 siLoop_end_offset, const bool bEnable )
{
	// this function sets up a loop within a sample, so until notified otherwise,
	// the sample will play between the start and end points
	if( hS == NULL )
		return;

	C3DSample* p3DSample = ( C3DSample* )hS;
	CSample* pSample = &p3DSample->m_sample;
	pSample->m_nLoopStart = siLoop_start_offset;
	pSample->m_nLoopEnd = siLoop_end_offset;
	pSample->m_bLoopBlock = bEnable;
}

void CDx8SoundSys::Set3DSampleLoop( LH3DSAMPLE hS, bool bLoop )
{
	if( hS == NULL )
		return;

	C3DSample* p3DSample = ( C3DSample* )hS;
	p3DSample->m_sample.Restore( );

	MtRLockGuard cProtection(m_cCS_SoundThread);

	p3DSample->m_sample.SetLooping( this, bLoop );
}

void CDx8SoundSys::Set3DSampleObstruction( LH3DSAMPLE hS, float fObstruction )
{
}

float CDx8SoundSys::Get3DSampleObstruction( LH3DSAMPLE hS )
{
	return 0;
}

void CDx8SoundSys::Set3DSampleOcclusion( LH3DSAMPLE hS, float fOcclusion )
{
}

float CDx8SoundSys::Get3DSampleOcclusion( LH3DSAMPLE hS )
{
	return 0;
}

//	===========================================================================
//	DONE...

// 2d sound sample functions
LHSAMPLE CDx8SoundSys::AllocateSampleHandle( LHDIGDRIVER hDig )
{
//	LOG_WRITE( g_pLogFile, "AllocateSampleHandle( %x )\n", hDig );

	CSample* pSample;
	LT_MEM_TRACK_ALLOC(pSample = new CSample,LT_MEM_TYPE_SOUND);

    // Set up wave format structure.

	pSample->m_waveFormat = m_waveFormat;

    // Set up DSBUFFERDESC structure.

    memset( &pSample->m_dsbDesc, 0, sizeof( DSBUFFERDESC ) ); // Zero it out.

    pSample->m_dsbDesc.dwSize = sizeof( DSBUFFERDESC );
    pSample->m_dsbDesc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME;

	// if software only set flag
	if ( m_iCur3DProvider == SOUND3DPROVIDERID_DS3D_SOFTWARE )
		pSample->m_dsbDesc.dwFlags |= DSBCAPS_LOCSOFTWARE;

    // Create buffer.
	pSample->Init( m_hResult, m_pDirectSound, m_waveFormat.sample_rate_, false );

	SetSampleNotify( pSample, true );

	return pSample;
}

void CDx8SoundSys::ReleaseSampleHandle( LHSAMPLE hS )
{
//	LOG_WRITE( g_pLogFile, "ReleaseSampleHandle( %x )\n", hS );

	if( hS == NULL )
		return;

	MtRLockGuard cProtection(m_cCS_SoundThread);

	CSample* pSample = ( CSample* )hS;

	SetSampleNotify( pSample, false );

	delete pSample;
}

#define DEFAULT_SAMPLE_CHANNELS	1
#define DEFAULT_SAMPLE_RATE		11025
#define DEFAULT_SAMPLE_BITS		8

void CDx8SoundSys::InitSample( LHSAMPLE hS )
{
//	LOG_WRITE( g_pLogFile, "InitSample( %x )\n", hS );

	if( hS == NULL )
		return;

	CSample* pSample = ( CSample* )hS;

	pSample->m_waveFormat.tag_ = ul::WaveFormatTag::pcm;
	pSample->m_waveFormat.channel_count_ = DEFAULT_SAMPLE_CHANNELS;
	pSample->m_waveFormat.sample_rate_ = DEFAULT_SAMPLE_RATE;
	pSample->m_waveFormat.bit_depth_ = DEFAULT_SAMPLE_BITS;

	pSample->Init( m_hResult, m_pDirectSound, DEFAULT_SAMPLE_RATE, false );
	SetSampleNotify( pSample, true );

}

// pause playback at current position
void CDx8SoundSys::StopSample( LHSAMPLE hS )
{
//	LOG_WRITE( g_pLogFile, "StopSample( %x )\n", hS );

	if( hS == NULL )
		return;

	CSample* pSample = ( CSample* )hS;
	pSample->Restore( );
	m_hResult = pSample->Stop( true );

	SetSampleNotify( pSample, false );
	m_pcLastError = LastError( );
}

// start playback at beginning position
void CDx8SoundSys::StartSample( LHSAMPLE hS )
{
//	LOG_WRITE( g_pLogFile, "StartSample( %x )\n", hS );

	if( hS == NULL )
		return;

	CSample* pSample = ( CSample* )hS;
	pSample->Restore( );

	m_hResult = pSample->Stop( true );
	m_hResult = pSample->Play( );
	m_pcLastError = LastError( );
}

// continue playback from current position
void CDx8SoundSys::ResumeSample( LHSAMPLE hS )
{
//	LOG_WRITE( g_pLogFile, "ResumeSample( %x )\n", hS );

	if( hS == NULL )
		return;

	CSample* pSample = ( CSample* )hS;
	pSample->Restore( );

	m_hResult = pSample->Play( );
	m_pcLastError = LastError( );
}

// terminate playback and reset position
void CDx8SoundSys::EndSample( LHSAMPLE hS )
{
//	LOG_WRITE( g_pLogFile, "EndSample( %x )\n", hS );

	if( hS == NULL )
		return;

	CSample* pSample = ( CSample* )hS;
	pSample->Restore( );
	m_hResult = pSample->Stop( true );

	SetSampleNotify( pSample, false );
	m_pcLastError = LastError( );
}

void CDx8SoundSys::SetSampleVolume( LHSAMPLE hS, S32 siVolume )
{
//	LOG_WRITE( g_pLogFile, "SetSampleVolume( %x, %d )\n", hS, siVolume );

	if( hS == NULL )
		return;

	CSample* pSample = ( CSample* )hS;
	pSample->Restore( );

	if( siVolume < ( S32 )MIN_MASTER_VOLUME )
		siVolume = ( S32 )MIN_MASTER_VOLUME;

	if( siVolume > ( S32 )MAX_MASTER_VOLUME )
		siVolume = ( S32 )MAX_MASTER_VOLUME;

	long lDSVolume;
	CONVERT_LIN_VOL_TO_LOG_VOL( siVolume, lDSVolume );

	m_hResult = pSample->m_pDSBuffer->SetVolume( lDSVolume );
	m_pcLastError = LastError( );
}

S32	CDx8SoundSys::GetSampleVolume( LHSAMPLE hS )
{
//	LOG_WRITE( g_pLogFile, "GetSampleVolume( %x )\n", hS );

	if( hS == NULL )
		return 0;

	CSample* pSample = ( CSample* )hS;
	pSample->Restore( );

	long lDSVolume = 0;
	m_hResult = pSample->m_pDSBuffer->GetVolume( &lDSVolume );
	m_pcLastError = LastError( );

	S32 siVolume;
	CONVERT_LOG_VOL_TO_LIN_VOL( lDSVolume, siVolume );

	return siVolume;
}

#define MIN_MASTER_PAN	1		// LEFT
#define MAX_MASTER_PAN	127		// RIGHT
#define DEL_MASTER_PAN	(float) ( MAX_MASTER_PAN - MIN_MASTER_PAN )
#define CENTER_PAN		(float) ((MAX_MASTER_PAN - MIN_MASTER_PAN)/2 + MIN_MASTER_PAN)
#define DEL_SIDE_PAN	(float) (MAX_MASTER_PAN - CENTER_PAN)

#define DSBPAN_MIN		(float) DSBPAN_LEFT
#define DSBPAN_MAX		(float) DSBPAN_RIGHT
#define DSBPAN_DEL		(float) ( DSBPAN_RIGHT - DSBPAN_LEFT )
#define DEL_DSBPAN_SIDE	(float) ( DSBPAN_RIGHT - DSBPAN_CENTER )

// raise this value decrease drop off of pan level
#define ATTENUATION_FACTOR	6.0f
#define LOG_PAN_ATTENUATION	(float)	( ATTENUATION_FACTOR * sqrt( CENTER_PAN ))
#define LOG_PAN_INVATTENUATION	(float) ( 1.0f / LOG_PAN_ATTENUATION )

// raise this to drop max pan rolloff for a channel
#define MAX_ROLLOFF_DIVIDER	3.0f

// helper functions, since DX8 wants logarithmic
// pan values, while the game keeps linear ones
long ConvertLinPanToLogPan( long linPan )
{
	long logPan;

	float fDistFromCenter = fabsf( (float)linPan - CENTER_PAN );

	// avoid divide by 0
	if ( fDistFromCenter == 0 )
		return DSBPAN_CENTER;

	float t = ( 1.0f - ( MIN_MASTER_PAN / fDistFromCenter ) ) * ( CENTER_PAN / DEL_SIDE_PAN );
	t = ( float )pow( t, LOG_PAN_ATTENUATION );

	// this essentially clamps the maximum rolloff
	// DX allows 100dB max.  This allows only 33 dB
	logPan = ( long )( t * DEL_DSBPAN_SIDE / MAX_ROLLOFF_DIVIDER );

	if ( linPan < CENTER_PAN )
		logPan = -logPan;

	return logPan;
}

S32 ConvertLogPanToLinPan( long logPan )
{
	long linPan;

	long lTempPan = abs( logPan );
	float t = (float) lTempPan * MAX_ROLLOFF_DIVIDER / DEL_DSBPAN_SIDE;
	t = ( float )pow( t, LOG_PAN_INVATTENUATION );

	float fDistFromCenter = (CENTER_PAN * MIN_MASTER_PAN) / (CENTER_PAN - (t * DEL_SIDE_PAN));

	if ( logPan < 0 )
		linPan = (long) (CENTER_PAN - fDistFromCenter);
	else
		linPan = (long) (CENTER_PAN + fDistFromCenter);

	return linPan;
}

void CDx8SoundSys::SetSamplePan( LHSAMPLE hS, S32 siPan )
{
//	LOG_WRITE( g_pLogFile, "SetSamplePan( %x, %d )\n", hS, siPan );

	if( hS == NULL )
		return;

	CSample* pSample = ( CSample* )hS;
	pSample->Restore( );

	long lDSPan = ConvertLinPanToLogPan( siPan );

	m_hResult = pSample->m_pDSBuffer->SetPan( lDSPan );
	m_pcLastError = LastError( );
}

S32	CDx8SoundSys::GetSamplePan( LHSAMPLE hS )
{
//	LOG_WRITE( g_pLogFile, "GetSamplePan( %x )\n", hS );

	if( hS == NULL )
		return 0;

	CSample* pSample = ( CSample* )hS;
	pSample->Restore( );

	long lDSPan = 0;
	m_hResult = pSample->m_pDSBuffer->GetPan( &lDSPan );
	m_pcLastError = LastError( );

	S32 siPan = ConvertLogPanToLinPan( lDSPan );

	return siPan;
}

void CDx8SoundSys::SetSampleUserData( LHSAMPLE hS, U32 uiIndex, S32 siValue )
{
//	LOG_WRITE( g_pLogFile, "SetSampleUserData( %x, %d, %d )\n", hS, uiIndex, siValue );

	if( hS == NULL || uiIndex > MAX_USER_DATA_INDEX )
		return;

	CSample* pSample = ( CSample* )hS;
	pSample->m_userData[ uiIndex ] = siValue;
}

void CDx8SoundSys::GetDirectSoundInfo( LHSAMPLE hS, PTDIRECTSOUND& ppDS, PTDIRECTSOUNDBUFFER& ppDSB )
{
	ppDS = NULL;
	ppDSB = NULL;

	ppDS = m_pDirectSound;

	if( hS == NULL )
		return;

	CSample* pSample = ( CSample* )hS;

	ppDSB = pSample->m_pDSBuffer;
}


//	===========================================================================
//	TO DO...

void CDx8SoundSys::SetSampleReverb( LHSAMPLE hS, float fReverb_level, float fReverb_reflect_time, float fReverb_decay_time )
{
//	LOG_WRITE( g_pLogFile, "SetSampleReverb( %x, %f, %f, %f )\n", hS, fReverb_level, fReverb_reflect_time, fReverb_decay_time );
}

//	===========================================================================
//	DONE...

S32 CDx8SoundSys::InitSampleFromAddress( LHSAMPLE hS, const void* pStart, const U32 uiLen, const ul::WaveFormatEx& pWaveFormat, const S32 siPlaybackRate, const LTSOUNDFILTERDATA* pFilterData )
{
//	LOG_WRITE( g_pLogFile, "InitSampleFromAddress( %x, %x, %d )\n", hS, pStart, uiLen );

	if( hS == NULL || pStart == NULL || uiLen == 0 )
		return LTFALSE;

	// Modify the pitch.
	auto waveFormat = pWaveFormat;
	waveFormat.sample_rate_ = siPlaybackRate;
	waveFormat.avg_bytes_per_sec_ = waveFormat.block_align_ * waveFormat.sample_rate_;
	CSample* pSample = ( CSample* )hS;
	if( !pSample->Init( m_hResult, m_pDirectSound, uiLen, false, &waveFormat, pFilterData ))
		return LTFALSE;

	pSample->m_pSoundData = const_cast<void*>(pStart);
	pSample->m_uiSoundDataLen = pSample->m_dsbDesc.dwBufferBytes;

	if( !pSample->Fill( ))
	{
		pSample->Term( );
		return LTFALSE;
	}

	SetSampleNotify( pSample, true );

	return LTTRUE;
}

S32	CDx8SoundSys::InitSampleFromFile(
	LHSAMPLE hS,
	const void* pFile_image,
	const S32 siBlock,
	const S32 siPlaybackRate,
	const LTSOUNDFILTERDATA* pFilterData)
{
	if (!hS)
	{
		return false;
	}

	const auto wave_size = ltjs::AudioUtils::extract_wave_size(pFile_image);

	if (wave_size == 0)
	{
		return false;
	}

	auto memory_stream = ul::MemoryStream{pFile_image, wave_size, ul::Stream::OpenMode::read};

	if (!audio_decoder_.open(&memory_stream))
	{
		return false;
	}

	auto pSample = static_cast<CSample*>(hS);

	const auto max_decoded_size = audio_decoder_.get_data_size();
	auto wave_format_ex = audio_decoder_.get_wave_format_ex();

	if (!audio_decoder_.is_pcm())
	{
		auto pucUncompressedSampleData = std::make_unique<std::uint8_t[]>(max_decoded_size);

		const auto decoded_size = audio_decoder_.decode(pucUncompressedSampleData.get(), max_decoded_size);

		if (decoded_size <= 0)
		{
			return false;
		}

		if (!pSample->Init(m_hResult, m_pDirectSound, decoded_size, false, &wave_format_ex))
		{
			return false;
		}

		if (!InitSampleFromAddress(
			hS,
			pucUncompressedSampleData.get(),
			decoded_size,
			wave_format_ex,
			siPlaybackRate,
			pFilterData))
		{
			return false;
		}

		pSample->m_bAllocatedSoundData = true;

		static_cast<void>(pucUncompressedSampleData.release());
	}
	else
	{
		// Modify the pitch.
		wave_format_ex.sample_rate_ = siPlaybackRate;
		wave_format_ex.avg_bytes_per_sec_ = wave_format_ex.block_align_ * wave_format_ex.sample_rate_;

		if (!pSample->Init(m_hResult, m_pDirectSound, max_decoded_size, false, &wave_format_ex, pFilterData))
		{
			return false;
		}

		if (!SetSampleNotify(pSample, true))
		{
			return false;
		}
	}

	return true;
}

//	===========================================================================
//	TO DO...

void CDx8SoundSys::SetSampleLoopBlock( LHSAMPLE hS, const S32 siLoop_start_offset, const S32 siLoop_end_offset, const bool bEnable )
{
	if( hS == NULL )
		return;

	CSample* pSample = ( CSample* )hS;
	pSample->m_nLoopStart = siLoop_start_offset;
	pSample->m_nLoopEnd = siLoop_end_offset;
	pSample->m_bLoopBlock = bEnable;
}

void CDx8SoundSys::SetSampleLoop( LHSAMPLE hS, const bool bLoop )
{
//	LOG_WRITE( g_pLogFile, "SetSampleLoopCount( %x, %d )\n", hS, siLoop_count );

	if( hS == NULL )
		return;

	CSample* pSample = ( CSample* )hS;
	pSample->Restore( );

	MtRLockGuard cProtection(m_cCS_SoundThread);

	pSample->SetLooping( this, bLoop );
}

//	===========================================================================
//	DONE...

void CDx8SoundSys::SetSampleMsPosition( LHSAMPLE hS, const S32 siMilliseconds )
{
//	LOG_WRITE( g_pLogFile, "SetSampleMsPosition( %x, %d )\n", hS, siMilliseconds );

	if( hS == NULL )
		return;

	CSample* pSample = ( CSample* )hS;
	pSample->Restore( );

	auto milliseconds = siMilliseconds;

	if( milliseconds < 0 )
		milliseconds = 0;

	uint32 uiByteOffset = MulDiv( pSample->m_waveFormat.avg_bytes_per_sec_, milliseconds, 1000 );
	uiByteOffset -= uiByteOffset % pSample->m_waveFormat.block_align_;
	m_hResult = pSample->SetCurrentPosition( uiByteOffset );
	m_pcLastError = LastError( );
}

S32	CDx8SoundSys::GetSampleUserData( LHSAMPLE hS, const U32 uiIndex )
{
//	LOG_WRITE( g_pLogFile, "GetSampleUserData( %x, %d )\n", hS, uiIndex );

	if( hS == NULL || uiIndex > MAX_USER_DATA_INDEX )
		return 0;

	CSample* pSample = ( CSample* )hS;
	return pSample->m_userData[ uiIndex ];
}

uint32 CDx8SoundSys::GetSampleStatus( LHSAMPLE hS )
{
	CSample* pSample = ( CSample* )hS;
	sint32 siStatus = ( pSample->IsPlaying( ) ? LS_PLAYING : LS_STOPPED );
	return siStatus;
}



//	===========================================================================
//	TO DO...

// old 2d sound stream functions
LHSTREAM CDx8SoundSys::OpenStream(
	const char* sFilename,
	const uint32 nFilePos,
	LHDIGDRIVER hDig,
	const char* sStream,
	sint32 siStream_mem)
{
	int i;

	for (i = 0; i < MAX_WAVE_STREAMS; ++i)
	{
		if (!m_WaveStream[i].IsActive())
		{
			if (!m_WaveStream[i].Open(sFilename, nFilePos))
			{
				return nullptr;
			}
			else
			{
				break;
			}
		}
	}

	if (i == MAX_WAVE_STREAMS)
	{
		return nullptr;
	}

	auto pWaveFile = &m_WaveStream[i];
	auto streamBufferParams = streamBufferParams_t{};

	const auto& wave_format_ex = pWaveFile->get_format();

	// create a buffer that holds STREAM_BUF_SECONDS seconds of data, rounded out so each read ends at block end
	auto nBlockAlign = static_cast<int>(wave_format_ex.channel_count_ * (m_waveFormat.bit_depth_ / 8));

	auto nBufferSize = static_cast<int>(
		wave_format_ex.sample_rate_ * STREAM_BUF_SECONDS * nBlockAlign / NUM_PLAY_NOTIFICATIONS);

	nBufferSize -= nBufferSize % nBlockAlign;
	nBufferSize *= NUM_PLAY_NOTIFICATIONS;

	streamBufferParams.m_siParams[SBP_BUFFER_SIZE] = nBufferSize;
	streamBufferParams.m_siParams[SBP_BITS_PER_CHANNEL] = wave_format_ex.bit_depth_;
	streamBufferParams.m_siParams[SBP_CHANNELS_PER_SAMPLE] = wave_format_ex.channel_count_;
	streamBufferParams.m_siParams[SBP_SAMPLES_PER_SEC] = wave_format_ex.sample_rate_;

	// Use the new streaming functions.
	auto hStream = OpenStream(&streamBufferParams, pWaveFile, static_cast<uint8>(i));

	if (!hStream)
	{
		// Error opening stream
		pWaveFile->Close();
	}

	return hStream;
}

void CDx8SoundSys::SetStreamLoop( LHSTREAM hStream, bool bLoop )
{
	if( hStream == NULL )
		return;

	MtRLockGuard cProtection(m_cCS_SoundThread);

	CStream* pStream = ( CStream* )hStream;

	pStream->SetLooping( this, bLoop );
}

void CDx8SoundSys::SetStreamPlaybackRate( LHSTREAM hStream, S32 siRate )
{
	//OutputDebugString("OLD [CDx8SoundSys::SetStreamPlaybackRate]\n");
}

void CDx8SoundSys::SetStreamMsPosition( LHSTREAM hStream, S32 siMilliseconds )
{
	MtRLockGuard cProtection(m_cCS_SoundThread);

	if( hStream == NULL )
		return;

	if( siMilliseconds < 0 )
		siMilliseconds = 0;

	CStream* pStream = ( CStream* )hStream;
	uint32 uiByteOffset = MulDiv( pStream->m_pWaveFile->GetDataSize( ), siMilliseconds,
		pStream->m_pWaveFile->GetDuration( ));
	uiByteOffset -= uiByteOffset % pStream->m_waveFormat.block_align_;
	m_hResult = pStream->SetCurrentPosition( uiByteOffset );
	m_pcLastError = LastError( );
}

void CDx8SoundSys::SetStreamUserData( LHSTREAM hS, U32 uiIndex, S32 siValue)
{
	//OutputDebugString("OLD [CDx8SoundSys::SetStreamUserData]\n");
}

S32	CDx8SoundSys::GetStreamUserData( LHSTREAM hS, U32 uiIndex)
{
	//OutputDebugString("OLD [CDx8SoundSys::GetStreamUserData]\n");
	return 0;
}

//	===========================================================================

//	DONE...

// new 2d sound stream functions
LHSTREAM CDx8SoundSys::OpenStream( streamBufferParams_t* pStreamBufferParams, WaveFile* pWaveFile, uint8 nEventNum )
{
	LHSTREAM hStream = NULL;

	//OutputDebugString("NEW [CDx8SoundSys::OpenStream]\n");
	if( pStreamBufferParams == NULL )
		return NULL;

	MtRLockGuard cProtection(m_cCS_SoundThread);

    CStream* pStream;
	LT_MEM_TRACK_ALLOC(pStream = new CStream( m_pStreams, NULL ),LT_MEM_TYPE_SOUND);

	pStream->Reset();

	ul::WaveFormatEx waveFormat;
	waveFormat = {};

	waveFormat.tag_ = ul::WaveFormatTag::pcm;
	waveFormat.bit_depth_ = ( WORD )pStreamBufferParams->m_siParams[ SBP_BITS_PER_CHANNEL ];
	waveFormat.sample_rate_ = pStreamBufferParams->m_siParams[ SBP_SAMPLES_PER_SEC ];
	waveFormat.channel_count_ = ( WORD )pStreamBufferParams->m_siParams[ SBP_CHANNELS_PER_SAMPLE ];
	waveFormat.block_align_ = ( waveFormat.channel_count_ * waveFormat.bit_depth_ ) >> 3;
	waveFormat.avg_bytes_per_sec_ = ( waveFormat.block_align_ * waveFormat.sample_rate_ );

	uint uiNumBytes = pStreamBufferParams->m_siParams[ SBP_BUFFER_SIZE ];
	pStream->m_uiBufferSize = uiNumBytes;

    // Set up DSBUFFERDESC structure.

    memset( &pStream->m_dsbDesc, 0, sizeof( DSBUFFERDESC ) ); // Zero it out.

    pStream->m_dsbDesc.dwSize = sizeof( DSBUFFERDESC );
    pStream->m_dsbDesc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;

	// Init the stream's sample

	pStream->Init( m_hResult, m_pDirectSound, uiNumBytes, false, &waveFormat );

	// Always need to loop streamed sounds buffers.
	pStream->m_dwPlayFlags |= DSBPLAY_LOOPING;

	if( m_hResult != DS_OK )
	{
		delete pStream;
		pStream = NULL;

		goto end;
	}
	else
	{
		memcpy( &pStream->m_streamBufferParams, pStreamBufferParams, sizeof( pStream->m_streamBufferParams ) );

	   	pStream->m_nEventNum = nEventNum;
		pStream->m_pWaveFile = pWaveFile;
		pStream->m_uiNextWriteOffset = 0;
		pStream->m_uiLastPlayPos = 0;
		pStream->m_uiTotalPlayed = 0;

		if ( !pStream->FillBuffer( this ) )
			goto end;

	   	hStream = ( LHSTREAM )pStream;
	}

	// Associate a stream with a file object.  We'll use this later in the
	// TimeCallback() function to load data and store it in the stream
	pWaveFile->SetStream(hStream);

//	ClearStreamBuffer( hStream, false );
end:
	return hStream;
}


void CDx8SoundSys::CloseStream( LHSTREAM hStream )
{
	MtRLockGuard cProtection(m_cCS_SoundThread);

	//OutputDebugString("NEW [CDx8SoundSys::CloseStream]\n");
	PauseStream( hStream, 1 );
	for(int i = 0; i < MAX_WAVE_STREAMS; i++)
	{
		if(m_WaveStream[i].GetStream() == hStream)
		{
			m_WaveStream[i].Close();
			break;
		}
	}
	CStream* pStream = ( CStream* )hStream;
	delete pStream;
}

void CDx8SoundSys::StartStream( LHSTREAM hStream )
{
	MtRLockGuard cProtection(m_cCS_SoundThread);

	CStream* pStream = ( CStream* )hStream;
	pStream->Stop( true );
	pStream->Play( );

	// Tell the streaming thread something's going on...
	is_mt_streaming_active_ = true;
}

void CDx8SoundSys::PauseStream( LHSTREAM hStream, sint32 siOnOff )
{
	MtRLockGuard cProtection(m_cCS_SoundThread);

	//OutputDebugString("NEW [CDx8SoundSys::PauseStream]\n");
	CStream* pStream = ( CStream* )hStream;
	if( siOnOff == 1 )
		pStream->Stop( false );
	else
	{
		is_mt_streaming_active_ = true;
		pStream->Play( );
	}
}

void CDx8SoundSys::ResetStream( LHSTREAM hStream )
{
	MtRLockGuard cProtection(m_cCS_SoundThread);

	//OutputDebugString("NEW [CDx8SoundSys::ResetStream]\n");
	PauseStream( hStream, 1 );
	CStream* pStream = ( CStream* )hStream;
	pStream->Stop( true );
}


//	DONE...

void CDx8SoundSys::SetStreamVolume( LHSTREAM hStream, sint32 siVolume )
{
	MtRLockGuard cProtection(m_cCS_SoundThread);

	//OutputDebugString("NEW [CDx8SoundSys::SetStreamVolume]\n");

	CStream* pStream = ( CStream* )hStream;
	pStream->Restore( );

	if( siVolume < ( S32 )MIN_MASTER_VOLUME )
		siVolume = ( S32 )MIN_MASTER_VOLUME;

	if( siVolume > ( S32 )MAX_MASTER_VOLUME )
		siVolume = ( S32 )MAX_MASTER_VOLUME;

	long lDSVolume;
	CONVERT_LIN_VOL_TO_LOG_VOL( siVolume, lDSVolume );

	m_hResult = pStream->m_pDSBuffer->SetVolume( lDSVolume );
	m_pcLastError = LastError( );
}

void CDx8SoundSys::SetStreamPan( LHSTREAM hStream, sint32 siPan )
{
	MtRLockGuard cProtection(m_cCS_SoundThread);

	//OutputDebugString("NEW [CDx8SoundSys::SetStreamPan]\n");

	CStream* pStream = ( CStream* )hStream;
	pStream->Restore( );

	long lDSPan = ConvertLinPanToLogPan( siPan );

	m_hResult = pStream->m_pDSBuffer->SetPan( lDSPan );
	m_pcLastError = LastError( );
}

sint32 CDx8SoundSys::GetStreamVolume( LHSTREAM hStream )
{
	MtRLockGuard cProtection(m_cCS_SoundThread);

	//OutputDebugString("NEW [CDx8SoundSys::GetStreamVolume]\n");

	CStream* pStream = ( CStream* )hStream;
	pStream->Restore( );

	long lDSVolume = 0;
	m_hResult = pStream->m_pDSBuffer->GetVolume( &lDSVolume );
	m_pcLastError = LastError( );

	S32 siVolume;
	CONVERT_LOG_VOL_TO_LIN_VOL( lDSVolume, siVolume );

	return 0;
}

sint32 CDx8SoundSys::GetStreamPan( LHSTREAM hStream )
{
	MtRLockGuard cProtection(m_cCS_SoundThread);

	//OutputDebugString("NEW [CDx8SoundSys::GetStreamPan]\n");

	CStream* pStream = ( CStream* )hStream;
	pStream->Restore( );

	long lDSPan = 0;
	m_hResult = pStream->m_pDSBuffer->GetPan( &lDSPan );
	m_pcLastError = LastError( );

	S32 siPan = ConvertLogPanToLinPan( lDSPan );

	return siPan;
}

uint32 CDx8SoundSys::GetStreamStatus( LHSTREAM hStream )
{
	MtRLockGuard cProtection(m_cCS_SoundThread);

	//OutputDebugString("NEW [CDx8SoundSys::GetStreamStatus]\n");
	CStream* pStream = ( CStream* )hStream;
	uint32 nStatus = ( pStream->IsDone( ) ? LS_STOPPED : LS_PLAYING );
	return nStatus;
}

sint32 CDx8SoundSys::GetStreamBufferParam( LHSTREAM hStream, uint32 uiParam )
{
	MtRLockGuard cProtection(m_cCS_SoundThread);

	//OutputDebugString("NEW [CDx8SoundSys::GetStreamBufferParam]\n");
	CStream* pStream = ( CStream* )hStream;
	sint32 siParamVal = ( uiParam < SBP_NUM_PARAMS ? pStream->m_streamBufferParams.m_siParams[ uiParam ] : 0 );
	return siParamVal;
}

void CDx8SoundSys::ClearStreamBuffer( LHSTREAM hStream, bool bClearStreamDataQueue )
{
	MtRLockGuard cProtection(m_cCS_SoundThread);

	//OutputDebugString("NEW [CDx8SoundSys::ClearStreamBuffer]\n");
	PauseStream( hStream, 1 );

	CStream* pStream = ( CStream* )hStream;

	uint32 uiBufferSize = pStream->m_streamBufferParams.m_siParams[ SBP_BUFFER_SIZE ];

	unsigned long uiChunkSize1, uiChunkSize2;
	uint8* pucChunk1 = NULL;
	uint8* pucChunk2 = NULL;

	if( pStream->Lock( 0, uiBufferSize, ( void** )&pucChunk1, &uiChunkSize1,
		( void** )&pucChunk2, &uiChunkSize2, 0 ) == TRUE )
	{
		if( pucChunk1 != NULL )
			memset( pucChunk1, 0, uiChunkSize1 );

		if( pucChunk2 != NULL )
			memset( pucChunk2, 0, uiChunkSize2 );

		pStream->Unlock( pucChunk1, uiChunkSize1, pucChunk2, uiChunkSize2 );
	}
}


//	===========================================================================
//	TO DO...

// these next two functions are here apparently for compatibility with MSS interface
// they aren't used.  We should probably remove them when we update the SoundSys interface
S32	CDx8SoundSys::DecompressADPCM(
	const LTSOUNDINFO& pInfo,
	void*& ppOutData,
	U32& puiOutSize)
{
	static_cast<void>(pInfo);
	static_cast<void>(ppOutData);
	static_cast<void>(puiOutSize);

	return 0;
}

S32	CDx8SoundSys::DecompressASI(
	const void* pInData,
	const U32 uiInSize,
	const char* sFilename_ext,
	void*& ppWav,
	U32& puiWavSize,
	LTLENGTHYCB fnCallback)
{
	auto memory_stream = ul::MemoryStream{pInData, static_cast<int>(uiInSize), ul::Stream::OpenMode::read};

	if (!audio_decoder_.open(&memory_stream))
	{
		return false;
	}

	if (!audio_decoder_.is_mp3())
	{
		return false;
	}

	const auto max_decoded_size = audio_decoder_.get_data_size();

	const auto header_size =
		4 + 4 + // "RIFF" + size
		4 + // WAVE
		4 + 4 + ul::WaveFormatEx::packed_size + // "fmt " + size + format_size
		4 + 4 + // "data" + size
		0;

	auto pucUncompressedSampleData = std::make_unique<std::uint8_t[]>(header_size + max_decoded_size);

	const auto decoded_size = audio_decoder_.decode(
		pucUncompressedSampleData.get() + header_size,
		max_decoded_size);

	if (decoded_size <= 0)
	{
		return false;
	}

	const auto wave_size = header_size + decoded_size;

	auto header = pucUncompressedSampleData.get();

	// fill in RIFF chunk
	header[0] = 'R';
	header[1] = 'I';
	header[2] = 'F';
	header[3] = 'F';
	header += 4;

	*reinterpret_cast<std::uint32_t*>(header) = wave_size - 8;
	header += 4;

	// fill in WAVE chunk
	header[0] = 'W';
	header[1] = 'A';
	header[2] = 'V';
	header[3] = 'E';
	header[4] = 'f';
	header[5] = 'm';
	header[6] = 't';
	header[7] = ' ';
	header += 8;

	*reinterpret_cast<std::uint32_t*>(header) = ul::WaveFormatEx::packed_size;
	header += 4;

	const auto wave_format_ex = audio_decoder_.get_wave_format_ex();
	*reinterpret_cast<ul::WaveFormatEx*>(header) = wave_format_ex;
	header += ul::WaveFormatEx::packed_size;

	// fill in DATA chunk
	header[0] = 'd';
	header[1] = 'a';
	header[2] = 't';
	header[3] = 'a';
	header += 4;

	*reinterpret_cast<std::uint32_t*>(header) = decoded_size;
	header += 4;

	ppWav = pucUncompressedSampleData.release();
	puiWavSize = wave_size;

	return true;
}

UINT CDx8SoundSys::ReadStream(
	WaveFile* pStream,
	BYTE* pOutBuffer,
	int nSize)
{
	return pStream->Read(pOutBuffer, nSize);
}

uint32 CDx8SoundSys::GetThreadedSoundTicks( )
{
	uint32 nThreadedTickCounts = 0;

	// Copy the tick counts.
	{
		MtLockGuard cThreadTickCountsProtection(m_cCS_ThreadedTickCounts);
		nThreadedTickCounts = m_nThreadedTickCounts;

		// Reset it for the next frame.
		m_nThreadedTickCounts = 0;
	}

	return nThreadedTickCounts;
}

//	===========================================================================
//	DONE...

CDx8SoundSys CDx8SoundSys::m_Dx8SoundSys;
const char* CDx8SoundSys::m_pcDx8SoundSysDesc = "DirectSound ( DirectX 8 )";

extern "C"
{
	__declspec( dllexport ) const char*		SoundSysDesc( );
	__declspec( dllexport ) ILTSoundSys*	SoundSysMake( );
}

const char* SoundSysDesc( )
{
	return CDx8SoundSys::m_pcDx8SoundSysDesc;
}

ILTSoundSys* SoundSysMake( )
{
	return &CDx8SoundSys::m_Dx8SoundSys;
}

bool APIENTRY DllMain( HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{
    return true;
}

