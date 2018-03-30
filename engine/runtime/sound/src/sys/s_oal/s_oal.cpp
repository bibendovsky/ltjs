#include "s_oal.h"
#include <cstdint>
#include <array>
#include <chrono>
#include <limits>
#include <list>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "efx.h"
#include "bibendovsky_spul_algorithm.h"
#include "bibendovsky_spul_scope_guard.h"
#include "ltjs_audio_decoder.h"


struct OalSoundSys::Impl
{
	static constexpr auto min_aux_sends = 1;
	static constexpr auto default_aux_sends = 2;

	static constexpr auto min_volume = 1;
	static constexpr auto max_volume = 127;
	static constexpr auto default_volume = max_volume;


	using String = std::string;
	using ExtensionsStrings = std::vector<String>;

	using Clock = std::chrono::system_clock;
	using ClockTs = Clock::time_point;


	struct SampleBase
	{
		using Data = std::vector<std::uint8_t>;


		ul::WaveFormatEx format_;
		bool is_looping_;
		bool has_loop_block_;
		sint32 loop_start_;
		sint32 loop_end_;
		sint32 volume_;
		Data data_;

		ALenum oal_buffer_;
		ALenum oal_loop_buffer_;
	}; // SampleBase

	struct Sample2d :
		public SampleBase
	{
		static constexpr auto max_user_data_count = 8;
		static constexpr auto max_user_data_index = max_user_data_count - 1;

		using UserDataArray = std::array<sint32, max_user_data_count>;

		sint32 pan_;
		UserDataArray user_data_array_;
	}; // Sample2d

	using Samples2d = std::list<Sample2d>;


	// =========================================================================
	// OpenAL
	// =========================================================================
	//

	struct OalVersion
	{
		int major_;
		int minor_;


		constexpr bool is_empty() const
		{
			return major_ == 0 && minor_ == 0;
		}

		constexpr bool operator<(
			const OalVersion& that)
		{
			return major_ < that.major_ || (major_ == that.major_ && minor_ < that.minor_);
		}
	}; // OalVersion


	static constexpr auto oal_ref_version = OalVersion{1, 1};
	static constexpr auto oal_efx_ref_version = OalVersion{1, 0};


	const char* get_error_message() const
	{
		return error_message_.c_str();
	}

	static void oal_clear_error()
	{
		static_cast<void>(::alGetError());
	}

	static bool oal_is_succeed()
	{
		return ::alGetError() == AL_NO_ERROR;
	}

	OalVersion oal_get_version(
		const ALenum oal_major_id,
		const ALenum oal_minor_id)
	{
		oal_clear_error();

		auto major = ALCint{};
		auto minor = ALCint{};

		::alcGetIntegerv(oal_device_, oal_major_id, 1, &major);
		::alcGetIntegerv(oal_device_, oal_minor_id, 1, &minor);

		if (!oal_is_succeed())
		{
			return {};
		}

		return {major, minor};
	}

	ExtensionsStrings oal_parse_extensions_string(
		const char* const extensions_string)
	{
		auto iss = std::istringstream{extensions_string};

		return ExtensionsStrings{
			std::istream_iterator<String>{iss},
			std::istream_iterator<String>{}};
	}

	bool oal_get_global_strings()
	{
		oal_clear_error();

		auto version_string = ::alGetString(AL_VERSION);
		auto renderer_string = ::alGetString(AL_RENDERER);
		auto vendor_string = ::alGetString(AL_VENDOR);
		auto extensions_string = ::alGetString(AL_EXTENSIONS);

		if (!oal_is_succeed() || !version_string || !renderer_string || !vendor_string || !extensions_string)
		{
			error_message_ = "OAL: Failed to get global strings.";
			return false;
		}

		oal_version_string_ = version_string;
		oal_renderer_string_ = renderer_string;
		oal_vendor_string_ = vendor_string;
		oal_extentions_strings_ = oal_parse_extensions_string(extensions_string);

		return true;
	}

	bool oal_get_version()
	{
		oal_version_ = oal_get_version(ALC_MAJOR_VERSION, ALC_MINOR_VERSION);

		if (oal_version_.is_empty())
		{
			error_message_ = "OAL: Failed to get a version.";
			return false;
		}

		if (oal_version_ < oal_ref_version)
		{
			error_message_ = "OAL: Expected at least version 1.1.";
			return false;
		}

		return true;
	}

	bool oal_get_efx_version()
	{
		oal_efx_version_ = oal_get_version(ALC_EFX_MAJOR_VERSION, ALC_EFX_MINOR_VERSION);

		if (oal_efx_version_.is_empty() || oal_efx_version_ < oal_efx_ref_version)
		{
			return false;
		}

		return true;
	}

	bool oal_get_max_aux_sends()
	{
		oal_clear_error();

		auto max_aux_sends = ALCint{};
		::alcGetIntegerv(oal_device_, ALC_MAX_AUXILIARY_SENDS, 1, &max_aux_sends);

		if (!oal_is_succeed() || max_aux_sends == 0)
		{
			return false;
		}

		oal_max_aux_sends_ = max_aux_sends;

		return true;
	}

	template<typename T>
	static bool oal_get_efx_function(
		const char* name,
		T& function)
	{
		function = reinterpret_cast<T>(::alGetProcAddress(name));

		return function;
	}

	void oal_clear_efx_functions()
	{
		alGenEffects_ = nullptr;
		alDeleteEffects_ = nullptr;
		alIsEffect_ = nullptr;
		alEffecti_ = nullptr;
		alEffectiv_ = nullptr;
		alEffectf_ = nullptr;
		alEffectfv_ = nullptr;
		alGetEffecti_ = nullptr;
		alGetEffectiv_ = nullptr;
		alGetEffectf_ = nullptr;
		alGetEffectfv_ = nullptr;
		alGenFilters_ = nullptr;
		alDeleteFilters_ = nullptr;
		alIsFilter_ = nullptr;
		alFilteri_ = nullptr;
		alFilteriv_ = nullptr;
		alFilterf_ = nullptr;
		alFilterfv_ = nullptr;
		alGetFilteri_ = nullptr;
		alGetFilteriv_ = nullptr;
		alGetFilterf_ = nullptr;
		alGetFilterfv_ = nullptr;
		alGenAuxiliaryEffectSlots_ = nullptr;
		alDeleteAuxiliaryEffectSlots_ = nullptr;
		alIsAuxiliaryEffectSlot_ = nullptr;
		alAuxiliaryEffectSloti_ = nullptr;
		alAuxiliaryEffectSlotiv_ = nullptr;
		alAuxiliaryEffectSlotf_ = nullptr;
		alAuxiliaryEffectSlotfv_ = nullptr;
		alGetAuxiliaryEffectSloti_ = nullptr;
		alGetAuxiliaryEffectSlotiv_ = nullptr;
		alGetAuxiliaryEffectSlotf_ = nullptr;
		alGetAuxiliaryEffectSlotfv_ = nullptr;
	}

	bool oal_get_efx_symbols()
	{
		auto result = true;

		result &= oal_get_efx_function("alGenEffects", alGenEffects_);
		result &= oal_get_efx_function("alDeleteEffects", alDeleteEffects_);
		result &= oal_get_efx_function("alIsEffect", alIsEffect_);
		result &= oal_get_efx_function("alEffecti", alEffecti_);
		result &= oal_get_efx_function("alEffectiv", alEffectiv_);
		result &= oal_get_efx_function("alEffectf", alEffectf_);
		result &= oal_get_efx_function("alEffectfv", alEffectfv_);
		result &= oal_get_efx_function("alGetEffecti", alGetEffecti_);
		result &= oal_get_efx_function("alGetEffectiv", alGetEffectiv_);
		result &= oal_get_efx_function("alGetEffectf", alGetEffectf_);
		result &= oal_get_efx_function("alGetEffectfv", alGetEffectfv_);
		result &= oal_get_efx_function("alGenFilters", alGenFilters_);
		result &= oal_get_efx_function("alDeleteFilters", alDeleteFilters_);
		result &= oal_get_efx_function("alIsFilter", alIsFilter_);
		result &= oal_get_efx_function("alFilteri", alFilteri_);
		result &= oal_get_efx_function("alFilteriv", alFilteriv_);
		result &= oal_get_efx_function("alFilterf", alFilterf_);
		result &= oal_get_efx_function("alFilterfv", alFilterfv_);
		result &= oal_get_efx_function("alGetFilteri", alGetFilteri_);
		result &= oal_get_efx_function("alGetFilteriv", alGetFilteriv_);
		result &= oal_get_efx_function("alGetFilterf", alGetFilterf_);
		result &= oal_get_efx_function("alGetFilterfv", alGetFilterfv_);
		result &= oal_get_efx_function("alGenAuxiliaryEffectSlots", alGenAuxiliaryEffectSlots_);
		result &= oal_get_efx_function("alDeleteAuxiliaryEffectSlots", alDeleteAuxiliaryEffectSlots_);
		result &= oal_get_efx_function("alIsAuxiliaryEffectSlot", alIsAuxiliaryEffectSlot_);
		result &= oal_get_efx_function("alAuxiliaryEffectSloti", alAuxiliaryEffectSloti_);
		result &= oal_get_efx_function("alAuxiliaryEffectSlotiv", alAuxiliaryEffectSlotiv_);
		result &= oal_get_efx_function("alAuxiliaryEffectSlotf", alAuxiliaryEffectSlotf_);
		result &= oal_get_efx_function("alAuxiliaryEffectSlotfv", alAuxiliaryEffectSlotfv_);
		result &= oal_get_efx_function("alGetAuxiliaryEffectSloti", alGetAuxiliaryEffectSloti_);
		result &= oal_get_efx_function("alGetAuxiliaryEffectSlotiv", alGetAuxiliaryEffectSlotiv_);
		result &= oal_get_efx_function("alGetAuxiliaryEffectSlotf", alGetAuxiliaryEffectSlotf_);
		result &= oal_get_efx_function("alGetAuxiliaryEffectSlotfv", alGetAuxiliaryEffectSlotfv_);

		return result;
	}

	bool oal_has_efx()
	{
		if (!::alcIsExtensionPresent(oal_device_, ALC_EXT_EFX_NAME))
		{
			return false;
		}

		return true;
	}

	bool oal_detect_effect(
		const ALenum effect_type)
	{
		oal_clear_error();

		auto has_effect = false;
		auto effect_name = ALuint{};

		alGenEffects_(1, &effect_name);

		if (oal_is_succeed())
		{
			alEffecti_(effect_name, AL_EFFECT_TYPE, effect_type);

			if (oal_is_succeed())
			{
				has_effect = true;
			}
		}

		if (alIsEffect_(effect_name))
		{
			alDeleteEffects_(1, &effect_name);
		}

		return has_effect;
	}

	void oal_detect_effects()
	{
		oal_has_chorus_effect_ = oal_detect_effect(AL_EFFECT_CHORUS);
		oal_has_compressor_effect_ = oal_detect_effect(AL_EFFECT_COMPRESSOR);
		oal_has_distortion_effect_ = oal_detect_effect(AL_EFFECT_DISTORTION);
		oal_has_echo_effect_ = oal_detect_effect(AL_EFFECT_ECHO);
		oal_has_flange_effect_ = oal_detect_effect(AL_EFFECT_FLANGER);
		oal_has_equalizer_effect_ = oal_detect_effect(AL_EFFECT_EQUALIZER);
		oal_has_reverb_effect_ = oal_detect_effect(AL_EFFECT_REVERB);
		oal_has_eax_reverb_effect_ = oal_detect_effect(AL_EFFECT_EAXREVERB);
	}

	void oal_examine_efx()
	{
		auto is_succeed = true;

		auto guard_this = ul::ScopeGuard{
			[&]()
			{
				if (!is_succeed)
				{
					oal_clear_efx();
				}
			}
		};

		if (!oal_get_efx_version())
		{
			return;
		}

		if (!oal_get_max_aux_sends())
		{
			return;
		}

		if (!oal_get_efx_symbols())
		{
			return;
		}

		oal_detect_effects();

		is_succeed = true;
		oal_has_efx_ = true;
	}

	void oal_clear_efx()
	{
		oal_efx_version_ = {};

		oal_has_efx_ = false;

		oal_has_chorus_effect_ = false;
		oal_has_compressor_effect_ = false;
		oal_has_distortion_effect_ = false;
		oal_has_echo_effect_ = false;
		oal_has_flange_effect_ = false;
		oal_has_equalizer_effect_ = false;
		oal_has_reverb_effect_ = false;
		oal_has_eax_reverb_effect_ = false;
		oal_max_aux_sends_ = 0;

		oal_clear_efx_functions();
	}

	bool oal_open_device()
	{
		oal_device_ = ::alcOpenDevice(nullptr);

		if (!oal_device_)
		{
			error_message_ = "OAL: Failed to open a device.";
			return false;
		}

		return true;
	}

	void oal_close_device()
	{
		if (oal_device_)
		{
			static_cast<void>(::alcCloseDevice(oal_device_));
			oal_device_ = nullptr;
		}
	}

	bool oal_create_context()
	{
		static const ALint attribs[] =
		{
			ALC_MAX_AUXILIARY_SENDS, default_aux_sends,
			0, 0,
		}; // attribs

		oal_context_ = ::alcCreateContext(oal_device_, attribs);

		if (!oal_context_)
		{
			error_message_ = "OAL: Failed to create a context.";
			return false;
		}

		if (!::alcMakeContextCurrent(oal_context_))
		{
			error_message_ = "OAL: Failed to make a context current.";
			return false;
		}

		return true;
	}

	void oal_destroy_context()
	{
		if (oal_context_)
		{
			static_cast<void>(::alcMakeContextCurrent(nullptr));
			::alcDestroyContext(oal_context_);
			oal_context_ = nullptr;
		}
	}

	//
	// =========================================================================
	// OpenAL
	// =========================================================================

	// =========================================================================
	// API
	// =========================================================================
	//

	bool startup()
	{
		clock_base_ = Clock::now();

		return true;
	}

	std::uint32_t get_milliseconds()
	{
		constexpr auto max_uint32_t = std::numeric_limits<std::uint32_t>::max();

		const auto time_diff = Clock::now() - clock_base_;
		const auto time_diff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_diff).count();

		return static_cast<std::uint32_t>(time_diff_ms % max_uint32_t);
	}

	void* allocate(
		const std::size_t size)
	{
		return new (std::nothrow) char[size];
	}

	void deallocate(
		void* ptr)
	{
		delete[] static_cast<char*>(ptr);
	}

	bool wave_out_open_internal()
	{
		auto is_succeed = false;

		auto guard_this = ul::ScopeGuard{
			[&]()
			{
				if (!is_succeed)
				{
					wave_out_close_internal();
				}
			}
		};

		if (!oal_open_device())
		{
			return false;
		}

		if (!oal_create_context())
		{
			return false;
		}

		if (!oal_get_version())
		{
			return false;
		}

		if (!oal_get_global_strings())
		{
			return false;
		}

		oal_examine_efx();

		is_succeed = true;
		master_volume_ = max_volume;

		return true;
	}

	sint32 wave_out_open(
		LHDIGDRIVER& driver,
		PHWAVEOUT& wave_out,
		const sint32 device_id,
		const ul::WaveFormatEx& wave_format)
	{
		static_cast<void>(device_id);
		static_cast<void>(wave_format);

		driver = nullptr;
		wave_out = nullptr;

		wave_out_close_internal();

		if (!wave_out_open_internal())
		{
			return LS_ERROR;
		}

		driver = oal_device_;

		return LS_OK;
	}

	void wave_out_close_internal()
	{
		oal_destroy_context();
		oal_close_device();
		oal_clear_efx();

		master_volume_ = {};
	}

	void wave_out_close(
		LHDIGDRIVER driver_ptr)
	{
		static_cast<void>(driver_ptr);

		wave_out_close_internal();
	}

	sint32 get_master_volume(
		LHDIGDRIVER driver_ptr)
	{
		if (!driver_ptr || driver_ptr != oal_device_)
		{
			return {};
		}

		return master_volume_;
	}

	void set_master_volume(
		LHDIGDRIVER driver_ptr,
		const int master_volume)
	{
		if (!driver_ptr || driver_ptr != oal_device_)
		{
			return;
		}

		const auto new_master_volume = ul::Algorithm::clamp(master_volume, min_volume, max_volume);

		if (master_volume_ == new_master_volume)
		{
			return;
		}

		master_volume_ = new_master_volume;


		const auto oal_gain =
			static_cast<ALfloat>(new_master_volume - min_volume) /
			static_cast<ALfloat>(max_volume - min_volume);

		::alListenerf(AL_GAIN, oal_gain);
	}

	//
	// =========================================================================
	// API
	// =========================================================================


	String error_message_;
	ALCdevice* oal_device_;
	ALCcontext* oal_context_;

	OalVersion oal_version_;
	String oal_version_string_;
	String oal_vendor_string_;
	String oal_renderer_string_;
	ExtensionsStrings oal_extentions_strings_;

	OalVersion oal_efx_version_;
	bool oal_has_efx_;
	bool oal_has_chorus_effect_;
	bool oal_has_compressor_effect_;
	bool oal_has_distortion_effect_;
	bool oal_has_echo_effect_;
	bool oal_has_flange_effect_;
	bool oal_has_equalizer_effect_;
	bool oal_has_reverb_effect_;
	bool oal_has_eax_reverb_effect_;
	int oal_max_aux_sends_;

	LPALGENEFFECTS alGenEffects_;
	LPALDELETEEFFECTS alDeleteEffects_;
	LPALISEFFECT alIsEffect_;
	LPALEFFECTI alEffecti_;
	LPALEFFECTIV alEffectiv_;
	LPALEFFECTF alEffectf_;
	LPALEFFECTFV alEffectfv_;
	LPALGETEFFECTI alGetEffecti_;
	LPALGETEFFECTIV alGetEffectiv_;
	LPALGETEFFECTF alGetEffectf_;
	LPALGETEFFECTFV alGetEffectfv_;
	LPALGENFILTERS alGenFilters_;
	LPALDELETEFILTERS alDeleteFilters_;
	LPALISFILTER alIsFilter_;
	LPALFILTERI alFilteri_;
	LPALFILTERIV alFilteriv_;
	LPALFILTERF alFilterf_;
	LPALFILTERFV alFilterfv_;
	LPALGETFILTERI alGetFilteri_;
	LPALGETFILTERIV alGetFilteriv_;
	LPALGETFILTERF alGetFilterf_;
	LPALGETFILTERFV alGetFilterfv_;
	LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots_;
	LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots_;
	LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot_;
	LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti_;
	LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv_;
	LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf_;
	LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv_;
	LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti_;
	LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv_;
	LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf_;
	LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv_;


	ClockTs clock_base_;
	sint32 master_volume_;
}; // OalSoundSys::Impl


OalSoundSys::OalSoundSys()
	:
	pimpl_{new Impl{}}
{
}

OalSoundSys::OalSoundSys(
	OalSoundSys&& that)
	:
	pimpl_{std::move(that.pimpl_)}
{
}

OalSoundSys::~OalSoundSys()
{
}

bool OalSoundSys::Init()
{
	ltjs::AudioDecoder::initialize_current_thread();

	return true;
}

void OalSoundSys::Term()
{
}

void* OalSoundSys::GetDDInterface(
	const uint dd_interface_id)
{
	static_cast<void>(dd_interface_id);

	return {};
}

void OalSoundSys::Lock()
{
}

void OalSoundSys::Unlock()
{
}

sint32 OalSoundSys::Startup()
{
	if (!pimpl_->startup())
	{
		return LS_ERROR;
	}

	return LS_OK;
}

void OalSoundSys::Shutdown()
{
}

uint32 OalSoundSys::MsCount()
{
	return pimpl_->get_milliseconds();
}

sint32 OalSoundSys::SetPreference(
	const uint32 number,
	const sint32 value)
{
	static_cast<void>(number);
	static_cast<void>(value);

	return LS_ERROR;
}

sint32 OalSoundSys::GetPreference(
	const uint32 number)
{
	static_cast<void>(number);

	return {};
}

void OalSoundSys::MemFreeLock(
	void* ptr)
{
	pimpl_->deallocate(ptr);
}

void* OalSoundSys::MemAllocLock(
	const uint32 size)
{
	return pimpl_->allocate(size);
}

const char* OalSoundSys::LastError()
{
	return pimpl_->get_error_message();
}

sint32 OalSoundSys::WaveOutOpen(
	LHDIGDRIVER& driver,
	PHWAVEOUT& wave_out,
	const sint32 device_id,
	const ul::WaveFormatEx& wave_format)
{
	return pimpl_->wave_out_open(driver, wave_out, device_id, wave_format);
}

void OalSoundSys::WaveOutClose(
	LHDIGDRIVER driver_ptr)
{
	pimpl_->wave_out_close(driver_ptr);
}

void OalSoundSys::SetDigitalMasterVolume(
	LHDIGDRIVER driver_ptr,
	const sint32 master_volume)
{
	pimpl_->set_master_volume(driver_ptr, master_volume);
}

sint32 OalSoundSys::GetDigitalMasterVolume(
	LHDIGDRIVER driver_ptr)
{
	return pimpl_->get_master_volume(driver_ptr);
}

sint32 OalSoundSys::DigitalHandleRelease(
	LHDIGDRIVER driver_ptr)
{
	static_cast<void>(driver_ptr);

	return {};
}

sint32 OalSoundSys::DigitalHandleReacquire(
	LHDIGDRIVER driver_ptr)
{
	static_cast<void>(driver_ptr);

	return {};
}

bool OalSoundSys::SetEAX20Filter(
	const bool is_enable,
	const LTSOUNDFILTERDATA& filter_data)
{
	static_cast<void>(is_enable);
	static_cast<void>(filter_data);

	return {};
}

bool OalSoundSys::SupportsEAX20Filter()
{
	return {};
}

bool OalSoundSys::SetEAX20BufferSettings(
	LHSAMPLE sample_handle,
	const LTSOUNDFILTERDATA& filter_data)
{
	static_cast<void>(sample_handle);
	static_cast<void>(filter_data);

	return {};
}

void OalSoundSys::Set3DProviderMinBuffers(
	const uint32 min_buffers)
{
	static_cast<void>(min_buffers);
}

sint32 OalSoundSys::Open3DProvider(
	LHPROVIDER provider_id)
{
	switch (provider_id)
	{
	case SOUND3DPROVIDERID_DS3D_SOFTWARE:
	case SOUND3DPROVIDERID_DS3D_HARDWARE:
		return true;

	default:
		return false;
	}
}

void OalSoundSys::Close3DProvider(
	LHPROVIDER provider_id)
{
	static_cast<void>(provider_id);
}

void OalSoundSys::Set3DProviderPreference(
	LHPROVIDER provider_id,
	const char* name,
	const void* value)
{
	static_cast<void>(provider_id);
	static_cast<void>(name);
	static_cast<void>(value);
}

void OalSoundSys::Get3DProviderAttribute(
	LHPROVIDER provider_id,
	const char* name,
	void* value)
{
	static_cast<void>(provider_id);
	static_cast<void>(name);
	static_cast<void>(value);
}

sint32 OalSoundSys::Enumerate3DProviders(
	LHPROENUM& next,
	LHPROVIDER& destination,
	const char*& name)
{
	constexpr auto max_providers = 3;

	static const char* const provider_names[max_providers] =
	{
		"OpenAL (2D)",
		"OpenAL (3D)",
		"OpenAL (default)",
	}; // provider_names

	const auto current = next++;

	destination = 0;
	name = nullptr;

	if (current < 0 || current >= max_providers)
	{
		return false;
	}

	switch (current)
	{
	case 0:
		destination = SOUND3DPROVIDERID_DS3D_SOFTWARE;
		break;

	case 1:
		destination = SOUND3DPROVIDERID_DS3D_HARDWARE;
		break;

	case 2:
		destination = SOUND3DPROVIDERID_DS3D_DEFAULT;
		break;

	default:
		return false;
	}

	name = provider_names[current];

	return true;
}

LH3DPOBJECT OalSoundSys::Open3DListener(
	LHPROVIDER provider_id)
{
	static_cast<void>(provider_id);

	return {};
}

void OalSoundSys::Close3DListener(
	LH3DPOBJECT listener_ptr)
{
	static_cast<void>(listener_ptr);
}

void OalSoundSys::SetListenerDoppler(
	LH3DPOBJECT listener_ptr,
	const float doppler)
{
	static_cast<void>(listener_ptr);
	static_cast<void>(doppler);
}

void OalSoundSys::CommitDeferred()
{
}

void OalSoundSys::Set3DPosition(
	LH3DPOBJECT object_ptr,
	const float x,
	const float y,
	const float z)
{
	static_cast<void>(object_ptr);
	static_cast<void>(x);
	static_cast<void>(y);
	static_cast<void>(z);
}

void OalSoundSys::Set3DVelocityVector(
	LH3DPOBJECT object_ptr,
	const float dx_per_s,
	const float dy_per_s,
	const float dz_per_s)
{
	static_cast<void>(object_ptr);
	static_cast<void>(dx_per_s);
	static_cast<void>(dy_per_s);
	static_cast<void>(dz_per_s);
}

void OalSoundSys::Set3DOrientation(
	LH3DPOBJECT object_ptr,
	const float x_face,
	const float y_face,
	const float z_face,
	const float x_up,
	const float y_up,
	const float z_up)
{
	static_cast<void>(object_ptr);
	static_cast<void>(x_face);
	static_cast<void>(y_face);
	static_cast<void>(z_face);
	static_cast<void>(x_up);
	static_cast<void>(y_up);
	static_cast<void>(z_up);
}

void OalSoundSys::Set3DUserData(
	LH3DPOBJECT object_ptr,
	const uint32 index,
	const sint32 value)
{
	static_cast<void>(object_ptr);
	static_cast<void>(index);
	static_cast<void>(value);
}

void OalSoundSys::Get3DPosition(
	LH3DPOBJECT object_ptr,
	float& x,
	float& y,
	float& z)
{
	static_cast<void>(object_ptr);
	static_cast<void>(x);
	static_cast<void>(y);
	static_cast<void>(z);
}

void OalSoundSys::Get3DVelocity(
	LH3DPOBJECT object_ptr,
	float& dx_per_ms,
	float& dy_per_ms,
	float& dz_per_ms)
{
	static_cast<void>(object_ptr);
	static_cast<void>(dx_per_ms);
	static_cast<void>(dy_per_ms);
	static_cast<void>(dz_per_ms);
}

void OalSoundSys::Get3DOrientation(
	LH3DPOBJECT object_ptr,
	float& x_face,
	float& y_face,
	float& z_face,
	float& x_up,
	float& y_up,
	float& z_up)
{
	static_cast<void>(object_ptr);
	static_cast<void>(x_face);
	static_cast<void>(y_face);
	static_cast<void>(z_face);
	static_cast<void>(x_up);
	static_cast<void>(y_up);
	static_cast<void>(z_up);
}

sint32 OalSoundSys::Get3DUserData(
	LH3DPOBJECT object_ptr,
	const uint32 index)
{
	static_cast<void>(object_ptr);
	static_cast<void>(index);

	return {};
}

LH3DSAMPLE OalSoundSys::Allocate3DSampleHandle(
	LHPROVIDER driver_id)
{
	static_cast<void>(driver_id);

	return {};
}

void OalSoundSys::Release3DSampleHandle(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::Stop3DSample(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::Start3DSample(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::Resume3DSample(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::End3DSample(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

sint32 OalSoundSys::Init3DSampleFromAddress(
	LH3DSAMPLE sample_handle,
	const void* ptr,
	const uint32 length,
	const ul::WaveFormatEx& wave_format,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	static_cast<void>(sample_handle);
	static_cast<void>(ptr);
	static_cast<void>(length);
	static_cast<void>(wave_format);
	static_cast<void>(playback_rate);
	static_cast<void>(filter_data_ptr);

	return {};
}

sint32 OalSoundSys::Init3DSampleFromFile(
	LH3DSAMPLE sample_handle,
	const void* file_image_ptr,
	const sint32 block,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	static_cast<void>(sample_handle);
	static_cast<void>(file_image_ptr);
	static_cast<void>(block);
	static_cast<void>(playback_rate);
	static_cast<void>(filter_data_ptr);

	return {};
}

sint32 OalSoundSys::Get3DSampleVolume(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

void OalSoundSys::Set3DSampleVolume(
	LH3DSAMPLE sample_handle,
	const sint32 volume)
{
	static_cast<void>(sample_handle);
	static_cast<void>(volume);
}

uint32 OalSoundSys::Get3DSampleStatus(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

void OalSoundSys::Set3DSampleMsPosition(
	LHSAMPLE sample_handle,
	const sint32 milliseconds)
{
	static_cast<void>(sample_handle);
	static_cast<void>(milliseconds);
}

sint32 OalSoundSys::Set3DSampleInfo(
	LH3DSAMPLE sample_handle,
	const LTSOUNDINFO& sound_info)
{
	static_cast<void>(sample_handle);
	static_cast<void>(sound_info);

	return {};
}

void OalSoundSys::Set3DSampleDistances(
	LH3DSAMPLE sample_handle,
	const float max_distance,
	const float min_distance)
{
	static_cast<void>(sample_handle);
	static_cast<void>(max_distance);
	static_cast<void>(min_distance);
}

void OalSoundSys::Set3DSamplePreference(
	LH3DSAMPLE sample_handle,
	const char* name,
	const void* value)
{
	static_cast<void>(sample_handle);
	static_cast<void>(name);
	static_cast<void>(value);
}

void OalSoundSys::Set3DSampleLoopBlock(
	LH3DSAMPLE sample_handle,
	const sint32 loop_start_offset,
	const sint32 loop_end_offset,
	const bool is_enable)
{
	static_cast<void>(sample_handle);
	static_cast<void>(loop_start_offset);
	static_cast<void>(loop_end_offset);
	static_cast<void>(is_enable);
}

void OalSoundSys::Set3DSampleLoop(
	LH3DSAMPLE sample_handle,
	const bool is_loop)
{
	static_cast<void>(sample_handle);
	static_cast<void>(is_loop);
}

void OalSoundSys::Set3DSampleObstruction(
	LH3DSAMPLE sample_handle,
	const float obstruction)
{
	static_cast<void>(sample_handle);
	static_cast<void>(obstruction);
}

float OalSoundSys::Get3DSampleObstruction(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

void OalSoundSys::Set3DSampleOcclusion(
	LH3DSAMPLE sample_handle,
	const float occlusion)
{
	static_cast<void>(sample_handle);
	static_cast<void>(occlusion);
}

float OalSoundSys::Get3DSampleOcclusion(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

LHSAMPLE OalSoundSys::AllocateSampleHandle(
	LHDIGDRIVER driver_ptr)
{
	static_cast<void>(driver_ptr);

	return {};
}

void OalSoundSys::ReleaseSampleHandle(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::InitSample(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::StopSample(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::StartSample(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::ResumeSample(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::EndSample(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::SetSampleVolume(
	LHSAMPLE sample_handle,
	const sint32 volume)
{
	static_cast<void>(sample_handle);
	static_cast<void>(volume);
}

void OalSoundSys::SetSamplePan(
	LHSAMPLE sample_handle,
	const sint32 pan)
{
	static_cast<void>(sample_handle);
	static_cast<void>(pan);
}

sint32 OalSoundSys::GetSampleVolume(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

sint32 OalSoundSys::GetSamplePan(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

void OalSoundSys::SetSampleUserData(
	LHSAMPLE sample_handle,
	const uint32 index,
	const sint32 value)
{
	static_cast<void>(sample_handle);
	static_cast<void>(index);
	static_cast<void>(value);
}

void OalSoundSys::GetDirectSoundInfo(
	LHSAMPLE sample_handle,
	PTDIRECTSOUND& direct_sound,
	PTDIRECTSOUNDBUFFER& direct_sound_buffer)
{
	static_cast<void>(sample_handle);
	direct_sound = nullptr;
	direct_sound_buffer = nullptr;
}

void OalSoundSys::SetSampleReverb(
	LHSAMPLE sample_handle,
	const float reverb_level,
	const float reverb_reflect_time,
	const float reverb_decay_time)
{
	static_cast<void>(sample_handle);
	static_cast<void>(reverb_level);
	static_cast<void>(reverb_reflect_time);
	static_cast<void>(reverb_decay_time);
}

sint32 OalSoundSys::InitSampleFromAddress(
	LHSAMPLE sample_handle,
	const void* ptr,
	const uint32 length,
	const ul::WaveFormatEx& wave_format,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	static_cast<void>(sample_handle);
	static_cast<void>(ptr);
	static_cast<void>(length);
	static_cast<void>(wave_format);
	static_cast<void>(playback_rate);
	static_cast<void>(filter_data_ptr);

	return {};
}

sint32 OalSoundSys::InitSampleFromFile(
	LHSAMPLE sample_handle,
	const void* file_image_ptr,
	const sint32 block,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	static_cast<void>(sample_handle);
	static_cast<void>(file_image_ptr);
	static_cast<void>(block);
	static_cast<void>(playback_rate);
	static_cast<void>(filter_data_ptr);

	return {};
}

void OalSoundSys::SetSampleLoopBlock(
	LHSAMPLE sample_handle,
	const sint32 loop_start_offset,
	const sint32 loop_end_offset,
	const bool is_enable)
{
	static_cast<void>(sample_handle);
	static_cast<void>(loop_start_offset);
	static_cast<void>(loop_end_offset);
	static_cast<void>(is_enable);
}

void OalSoundSys::SetSampleLoop(
	LHSAMPLE sample_handle,
	const bool is_loop)
{
	static_cast<void>(sample_handle);
	static_cast<void>(is_loop);
}

void OalSoundSys::SetSampleMsPosition(
	LHSAMPLE sample_handle,
	const sint32 milliseconds)
{
	static_cast<void>(sample_handle);
	static_cast<void>(milliseconds);
}

sint32 OalSoundSys::GetSampleUserData(
	LHSAMPLE sample_handle,
	const uint32 index)
{
	static_cast<void>(sample_handle);
	static_cast<void>(index);

	return {};
}

uint32 OalSoundSys::GetSampleStatus(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

LHSTREAM OalSoundSys::OpenStream(
	const char* file_name,
	const uint32 file_offset,
	LHDIGDRIVER driver_ptr,
	const char* stream_ptr,
	const sint32 stream_memory_size)
{
	static_cast<void>(file_name);
	static_cast<void>(file_offset);
	static_cast<void>(driver_ptr);
	static_cast<void>(stream_ptr);
	static_cast<void>(stream_memory_size);

	return {};
}

void OalSoundSys::SetStreamLoop(
	LHSTREAM stream_ptr,
	const bool is_loop)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(is_loop);
}

void OalSoundSys::SetStreamPlaybackRate(
	LHSTREAM stream_ptr,
	const sint32 rate)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(rate);
}

void OalSoundSys::SetStreamMsPosition(
	LHSTREAM stream_ptr,
	const sint32 milliseconds)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(milliseconds);
}

void OalSoundSys::SetStreamUserData(
	LHSTREAM stream_ptr,
	const uint32 index,
	const sint32 value)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(index);
	static_cast<void>(value);
}

sint32 OalSoundSys::GetStreamUserData(
	LHSTREAM stream_ptr,
	const uint32 index)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(index);

	return {};
}

void OalSoundSys::CloseStream(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);
}

void OalSoundSys::StartStream(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);
}

void OalSoundSys::PauseStream(
	LHSTREAM stream_ptr,
	const sint32 is_pause)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(is_pause);
}

void OalSoundSys::ResetStream(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);
}

void OalSoundSys::SetStreamVolume(
	LHSTREAM stream_ptr,
	const sint32 volume)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(volume);
}

void OalSoundSys::SetStreamPan(
	LHSTREAM stream_ptr,
	const sint32 pan)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(pan);
}

sint32 OalSoundSys::GetStreamVolume(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);

	return {};
}

sint32 OalSoundSys::GetStreamPan(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);

	return {};
}

uint32 OalSoundSys::GetStreamStatus(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);

	return {};
}

sint32 OalSoundSys::GetStreamBufferParam(
	LHSTREAM stream_ptr,
	const uint32 param)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(param);

	return {};
}

void OalSoundSys::ClearStreamBuffer(
	LHSTREAM stream_ptr,
	const bool is_clear_data_queue)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(is_clear_data_queue);
}

sint32 OalSoundSys::DecompressADPCM(
	const LTSOUNDINFO& sound_info,
	void*& dst_data,
	uint32& dst_size)
{
	static_cast<void>(sound_info);
	static_cast<void>(dst_data);
	static_cast<void>(dst_size);

	return {};
}

sint32 OalSoundSys::DecompressASI(
	const void* srd_data_ptr,
	const uint32 src_size,
	const char* file_name_ext,
	void*& dst_wav,
	uint32& dst_wav_size,
	LTLENGTHYCB callback)
{
	static_cast<void>(srd_data_ptr);
	static_cast<void>(src_size);
	static_cast<void>(file_name_ext);
	static_cast<void>(dst_wav);
	static_cast<void>(dst_wav_size);
	static_cast<void>(callback);

	return {};
}

uint32 OalSoundSys::GetThreadedSoundTicks()
{
	return {};
}

bool OalSoundSys::HasOnBoardMemory()
{
	return {};
}

OalSoundSys& OalSoundSys::get_singleton()
{
	static auto singleton = OalSoundSys{};
	return singleton;
}


extern "C"
{
	__declspec(dllexport) char* SoundSysDesc();
	__declspec(dllexport) ILTSoundSys* SoundSysMake();
}

char* SoundSysDesc()
{
	static char* description = const_cast<char*>("OpenAL");
	return description;
}

ILTSoundSys* SoundSysMake()
{
	return &OalSoundSys::get_singleton();
}
