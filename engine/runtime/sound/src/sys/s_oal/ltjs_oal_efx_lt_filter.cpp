#include "ltjs_oal_efx_lt_filter.h"

#include <cassert>
#include <cmath>

#include "bibendovsky_spul_algorithm.h"

#include "ltjs_exception.h"

#include "ltjs_audio_utils.h"
#include "ltjs_eax_api.h"
#include "ltjs_oal_efx_symbols.h"
#include "ltjs_oal_object.h"
#include "ltjs_oal_utils.h"


namespace ul = bibendovsky::spul;


namespace ltjs
{
namespace oal
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class EfxLtFilterException :
	public SafeException
{
public:
	explicit EfxLtFilterException(
		const char* message)
		:
		SafeException{message}
	{
	}
}; // EfxLtFilterException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

EfxLtFilter::EfxLtFilter()
	:
	efx_symbols_{make_efx_symbols()},
	effect_slot_{make_effect_slot_object(efx_symbols_)},
	reverb_effect_{make_reverb_effect()},
	direct_filter_{make_direct_filter()},
	al_xreverb_descriptors_{make_xreverb_descriptors()}
{
	detect_max_low_pass_gain();
	set_eax_reverb_defaults();
	mute_efx_slot_effect();
	set_efx_reverb_all();
	set_efx_effect();

	info_.name = "EFX";
}

// -------------------------------------------------------------------------
// LtFilter

const LtFilterInfo& EfxLtFilter::get_info() const noexcept
{
	return info_;
}

void EfxLtFilter::initialize_source(
	::ALuint al_source,
	int& lt_filter_direct_mb)
{
	ensure_al_source(al_source);

	lt_filter_direct_mb = ::EAXBUFFER_DEFAULTDIRECT;

	set_source_direct(al_source, lt_filter_direct_mb);

	clear_error_debug();

	::alSource3i(
		al_source,
		AL_AUXILIARY_SEND_FILTER,
		reverb_effect_.get(),
		0,
		direct_filter_.get()
	);

	ensure_no_error_debug();
}

void EfxLtFilter::set_listener(
	LtFilterState filter_state,
	const LTSOUNDFILTERDATA& lt_filter_data)
{
	ensure_lt_reverb_filter(lt_filter_data);

	is_reverb_dirty_ = false;

	const auto& lt_reverb = *reinterpret_cast<const LTFILTERREVERB*>(lt_filter_data.pSoundFilter);

	if ((lt_reverb.uiFilterParamFlags & SET_REVERB_ENVIRONMENT) != 0)
	{
		set_reverb_environment(lt_reverb.lEnvironment);
	}

	if ((lt_reverb.uiFilterParamFlags & SET_REVERB_SIZE) != 0)
	{
		set_reverb_environment_size(lt_reverb.fSize);
	}

	if ((lt_reverb.uiFilterParamFlags & SET_REVERB_ROOM) != 0)
	{
		set_reverb_room(lt_reverb.lRoom);
	}

	if ((lt_reverb.uiFilterParamFlags & SET_REVERB_ROOMHF) != 0)
	{
		set_reverb_room_hf(lt_reverb.lRoomHF);
	}

	if ((lt_reverb.uiFilterParamFlags & SET_REVERB_ROOMROLLOFFFACTOR) != 0)
	{
		set_reverb_room_rolloff_factor(lt_reverb.fRoomRolloffFactor);
	}

	if ((lt_reverb.uiFilterParamFlags & SET_REVERB_DECAYTIME) != 0)
	{
		set_reverb_decay_time(lt_reverb.fDecayTime);
	}

	if ((lt_reverb.uiFilterParamFlags & SET_REVERB_DECAYHFRATIO) != 0)
	{
		set_reverb_decay_hf_ratio(lt_reverb.fDecayHFRatio);
	}

	if ((lt_reverb.uiFilterParamFlags & SET_REVERB_REFLECTIONS) != 0)
	{
		set_reverb_reflections(lt_reverb.lReflections);
	}

	if ((lt_reverb.uiFilterParamFlags & SET_REVERB_REFLECTIONSDELAY) != 0)
	{
		set_reverb_reflections_delay(lt_reverb.fReflectionsDelay);
	}

	if ((lt_reverb.uiFilterParamFlags & SET_REVERB_REVERB) != 0)
	{
		set_reverb_reverb(lt_reverb.lReverb);
	}

	if ((lt_reverb.uiFilterParamFlags & SET_REVERB_REVERBDELAY) != 0)
	{
		set_reverb_reverb_delay(lt_reverb.fReverbDelay);
	}

	if ((lt_reverb.uiFilterParamFlags & SET_REVERB_DIFFUSION) != 0)
	{
		set_reverb_diffusion(lt_reverb.fDiffusion);
	}

	if ((lt_reverb.uiFilterParamFlags & SET_REVERB_AIRABSORPTIONHF) != 0)
	{
		set_reverb_air_absorption_hf(lt_reverb.fAirAbsorptionHF);
	}

	if (filter_state == LtFilterState::disable)
	{
		mute_efx_slot_effect();
	}
	else
	{
		unmute_efx_slot_effect();
	}

	if (is_reverb_dirty_)
	{
		is_reverb_dirty_ = false;

		set_efx_effect();
	}
}

void EfxLtFilter::set_source(
	::ALuint al_source,
	const LTSOUNDFILTERDATA& lt_filter_data,
	int& lt_filter_direct_mb)
{
	ensure_al_source(al_source);
	ensure_lt_reverb_filter(lt_filter_data);

	const auto& lt_reverb = *reinterpret_cast<const LTFILTERREVERB*>(lt_filter_data.pSoundFilter);

	if ((lt_reverb.uiFilterParamFlags & SET_REVERB_DIRECT) != 0)
	{
		if (lt_filter_direct_mb != lt_reverb.lDirect)
		{
			lt_filter_direct_mb = lt_reverb.lDirect;

			set_source_direct(al_source, lt_filter_direct_mb);
		}
	}
}

// LtFilter
// -------------------------------------------------------------------------

const EfxLtFilter::AlXReverbDescriptors& EfxLtFilter::get_std_reverb_descriptors()
{
	static const auto std_reverb_descriptors = AlXReverbDescriptors
	{{
		{
			AL_REVERB_DENSITY,
			AL_REVERB_MIN_DENSITY,
			AL_REVERB_MAX_DENSITY
		},
		{
			AL_REVERB_DIFFUSION,
			AL_REVERB_MIN_DIFFUSION,
			AL_REVERB_MAX_DIFFUSION
		},
		{
			AL_REVERB_GAIN,
			AL_REVERB_MIN_GAIN,
			AL_REVERB_MAX_GAIN
		},
		{
			AL_REVERB_GAINHF,
			AL_REVERB_MIN_GAINHF,
			AL_REVERB_MAX_GAINHF
		},
		{
			AL_NONE,
			AL_NONE,
			AL_NONE
		},
		{
			AL_REVERB_DECAY_TIME,
			AL_REVERB_MIN_DECAY_TIME,
			AL_REVERB_MAX_DECAY_TIME
		},
		{
			AL_REVERB_DECAY_HFRATIO,
			AL_REVERB_MIN_DECAY_HFRATIO,
			AL_REVERB_MAX_DECAY_HFRATIO
		},
		{
			AL_NONE,
			AL_NONE,
			AL_NONE
		},
		{
			AL_REVERB_REFLECTIONS_GAIN,
			AL_REVERB_MIN_REFLECTIONS_GAIN,
			AL_REVERB_MAX_REFLECTIONS_GAIN
		},
		{
			AL_REVERB_REFLECTIONS_DELAY,
			AL_REVERB_MIN_REFLECTIONS_DELAY,
			AL_REVERB_MAX_REFLECTIONS_DELAY
		},
		{
			AL_NONE,
			AL_NONE,
			AL_NONE
		},
		{
			AL_REVERB_LATE_REVERB_GAIN,
			AL_REVERB_MIN_LATE_REVERB_GAIN,
			AL_REVERB_MAX_LATE_REVERB_GAIN
		},
		{
			AL_REVERB_LATE_REVERB_DELAY,
			AL_REVERB_MIN_LATE_REVERB_DELAY,
			AL_REVERB_MAX_LATE_REVERB_DELAY
		},
		{
			AL_NONE,
			AL_NONE,
			AL_NONE
		},
		{
			AL_NONE,
			AL_NONE,
			AL_NONE
		},
		{
			AL_NONE,
			AL_NONE,
			AL_NONE
		},
		{
			AL_NONE,
			AL_NONE,
			AL_NONE
		},
		{
			AL_NONE,
			AL_NONE,
			AL_NONE
		},
		{
			AL_REVERB_AIR_ABSORPTION_GAINHF,
			AL_REVERB_MIN_AIR_ABSORPTION_GAINHF,
			AL_REVERB_MAX_AIR_ABSORPTION_GAINHF
		},
		{
			AL_NONE,
			AL_NONE,
			AL_NONE
		},
		{
			AL_NONE,
			AL_NONE,
			AL_NONE
		},
		{
			AL_REVERB_ROOM_ROLLOFF_FACTOR,
			AL_REVERB_MIN_ROOM_ROLLOFF_FACTOR,
			AL_REVERB_MAX_ROOM_ROLLOFF_FACTOR
		},
		{
			AL_REVERB_DECAY_HFLIMIT,
			AL_REVERB_MIN_DECAY_HFLIMIT,
			AL_REVERB_MAX_DECAY_HFLIMIT
		},
	}};

	return std_reverb_descriptors;
}

const EfxLtFilter::AlXReverbDescriptors& EfxLtFilter::get_eax_reverb_descriptors()
{
	static const auto eax_reverb_descriptors = AlXReverbDescriptors
	{{
		{
			AL_EAXREVERB_DENSITY,
			AL_EAXREVERB_MIN_DENSITY,
			AL_EAXREVERB_MAX_DENSITY
		},
		{
			AL_EAXREVERB_DIFFUSION,
			AL_EAXREVERB_MIN_DIFFUSION,
			AL_EAXREVERB_MAX_DIFFUSION
		},
		{
			AL_EAXREVERB_GAIN,
			AL_EAXREVERB_MIN_GAIN,
			AL_EAXREVERB_MAX_GAIN
		},
		{
			AL_EAXREVERB_GAINHF,
			AL_EAXREVERB_MIN_GAINHF,
			AL_EAXREVERB_MAX_GAINHF
		},
		{
			AL_EAXREVERB_GAINLF,
			AL_EAXREVERB_MIN_GAINLF,
			AL_EAXREVERB_MAX_GAINLF
		},
		{
			AL_EAXREVERB_DECAY_TIME,
			AL_EAXREVERB_MIN_DECAY_TIME,
			AL_EAXREVERB_MAX_DECAY_TIME
		},
		{
			AL_EAXREVERB_DECAY_HFRATIO,
			AL_EAXREVERB_MIN_DECAY_HFRATIO,
			AL_EAXREVERB_MAX_DECAY_HFRATIO
		},
		{
			AL_EAXREVERB_DECAY_LFRATIO,
			AL_EAXREVERB_MIN_DECAY_LFRATIO,
			AL_EAXREVERB_MAX_DECAY_LFRATIO
		},
		{
			AL_EAXREVERB_REFLECTIONS_GAIN,
			AL_EAXREVERB_MIN_REFLECTIONS_GAIN,
			AL_EAXREVERB_MAX_REFLECTIONS_GAIN
		},
		{
			AL_EAXREVERB_REFLECTIONS_DELAY,
			AL_EAXREVERB_MIN_REFLECTIONS_DELAY,
			AL_EAXREVERB_MAX_REFLECTIONS_DELAY
		},
		{
			AL_EAXREVERB_REFLECTIONS_PAN,
			AL_NONE,
			AL_NONE
		},
		{
			AL_EAXREVERB_LATE_REVERB_GAIN,
			AL_EAXREVERB_MIN_LATE_REVERB_GAIN,
			AL_EAXREVERB_MAX_LATE_REVERB_GAIN
		},
		{
			AL_EAXREVERB_LATE_REVERB_DELAY,
			AL_EAXREVERB_MIN_LATE_REVERB_DELAY,
			AL_EAXREVERB_MAX_LATE_REVERB_DELAY
		},
		{
			AL_EAXREVERB_LATE_REVERB_PAN,
			AL_NONE,
			AL_NONE
		},
		{
			AL_EAXREVERB_ECHO_TIME,
			AL_EAXREVERB_MIN_ECHO_TIME,
			AL_EAXREVERB_MAX_ECHO_TIME
		},
		{
			AL_EAXREVERB_ECHO_DEPTH,
			AL_EAXREVERB_MIN_ECHO_DEPTH,
			AL_EAXREVERB_MAX_ECHO_DEPTH
		},
		{
			AL_EAXREVERB_MODULATION_TIME,
			AL_EAXREVERB_MIN_MODULATION_TIME,
			AL_EAXREVERB_MAX_MODULATION_TIME
		},
		{
			AL_EAXREVERB_MODULATION_DEPTH,
			AL_EAXREVERB_MIN_MODULATION_DEPTH,
			AL_EAXREVERB_MAX_MODULATION_DEPTH
		},
		{
			AL_EAXREVERB_AIR_ABSORPTION_GAINHF,
			AL_EAXREVERB_MIN_AIR_ABSORPTION_GAINHF,
			AL_EAXREVERB_MAX_AIR_ABSORPTION_GAINHF
		},
		{
			AL_EAXREVERB_HFREFERENCE,
			AL_EAXREVERB_MIN_HFREFERENCE,
			AL_EAXREVERB_MAX_HFREFERENCE
		},
		{
			AL_EAXREVERB_LFREFERENCE,
			AL_EAXREVERB_MIN_LFREFERENCE,
			AL_EAXREVERB_MAX_LFREFERENCE
		},
		{
			AL_EAXREVERB_ROOM_ROLLOFF_FACTOR,
			AL_EAXREVERB_MIN_ROOM_ROLLOFF_FACTOR,
			AL_EAXREVERB_MAX_ROOM_ROLLOFF_FACTOR
		},
		{
			AL_EAXREVERB_DECAY_HFLIMIT,
			AL_EAXREVERB_MIN_DECAY_HFLIMIT,
			AL_EAXREVERB_MAX_DECAY_HFLIMIT
		},
	}};

	return eax_reverb_descriptors;
}

EffectObject EfxLtFilter::make_reverb_effect()
{
	auto reverb_effect = make_effect_object(efx_symbols_);

#if 1
	clear_error();

	efx_symbols_.alEffecti(reverb_effect.get(), AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);

	if (::alGetError() == AL_NO_ERROR)
	{
		is_std_reverb_ = false;
		return reverb_effect;
	}
#endif

#if 1
	clear_error();

	efx_symbols_.alEffecti(reverb_effect.get(), AL_EFFECT_TYPE, AL_EFFECT_REVERB);

	if (::alGetError() == AL_NO_ERROR)
	{
		is_std_reverb_ = true;
		return reverb_effect;
	}
#endif

	throw EfxLtFilterException{"Reverb effect not supported."};
}

FilterObject EfxLtFilter::make_direct_filter()
{
	auto direct_filter = make_filter_object(efx_symbols_);

	clear_error();

	efx_symbols_.alFilteri(direct_filter.get(), AL_FILTER_TYPE, AL_FILTER_LOWPASS);

	if (::alGetError() == AL_NO_ERROR)
	{
		return direct_filter;
	}

	throw EfxLtFilterException{"Failed to create low-pass filter."};
}

const EfxLtFilter::AlXReverbDescriptors& EfxLtFilter::make_xreverb_descriptors()
{
	if (is_std_reverb_)
	{
		return get_std_reverb_descriptors();
	}
	else
	{
		return get_eax_reverb_descriptors();
	}
}

void EfxLtFilter::make_info()
{
	if (is_std_reverb_)
	{
		info_.name = "Standard Reverb";
	}
	else
	{
		info_.name = "EAX Reverb";
	}
}

void EfxLtFilter::ensure_lt_reverb_filter(
	const LTSOUNDFILTERDATA& lt_filter_data)
{
	if (lt_filter_data.uiFilterType != FilterReverb)
	{
		throw EfxLtFilterException{"Expected LT reverb filter."};
	}
}

void EfxLtFilter::ensure_al_source(
	::ALuint al_source)
{
	if (al_source == 0)
	{
		throw EfxLtFilterException{"Null AL source."};
	}
}

bool EfxLtFilter::detect_al_softx_filter_gain_ex()
{
	const auto extension_name = "AL_SOFTX_filter_gain_ex";

	const auto has_al_softx_filter_gain_ex = (::alIsExtensionPresent(extension_name) != AL_FALSE);

	if (has_al_softx_filter_gain_ex)
	{
		info_.feature_names.emplace_back(extension_name);

		return true;
	}
	else
	{
		return false;
	}
}

void EfxLtFilter::detect_max_low_pass_gain()
{
	max_low_pass_gain_ = AL_LOWPASS_MAX_GAIN;

	auto is_detected = false;

	if (!is_detected)
	{
		is_detected = detect_al_softx_filter_gain_ex();
	}

	if (is_detected)
	{
		max_low_pass_gain_ = AudioUtils::level_mb_to_gain(::EAXBUFFER_MAXDIRECT);
	}
}

void EfxLtFilter::set_eax_reverb_defaults()
{
	eax_reverb_ = ::EAXREVERB_PRESETS[::EAX_ENVIRONMENT_GENERIC];
}

void EfxLtFilter::set_efx_reverb_density()
{
	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::density];

	const auto eax_environment_size = eax_reverb_.flEnvironmentSize;

	const auto efx_density = ul::Algorithm::clamp(
		(eax_environment_size * eax_environment_size * eax_environment_size) / 16.0F,
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_density
	));
}

void EfxLtFilter::set_efx_reverb_diffusion()
{
	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::diffusion];

	const auto efx_environment_diffusion = ul::Algorithm::clamp(
		eax_reverb_.flEnvironmentDiffusion,
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_environment_diffusion
	));
}

void EfxLtFilter::set_efx_reverb_gain()
{
	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::gain];

	const auto efx_room = ul::Algorithm::clamp(
		AudioUtils::level_mb_to_gain(static_cast<float>(eax_reverb_.lRoom)),
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_room
	));
}

void EfxLtFilter::set_efx_reverb_gain_hf()
{
	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::gain_hf];

	const auto efx_room_hf = ul::Algorithm::clamp(
		AudioUtils::level_mb_to_gain(static_cast<float>(eax_reverb_.lRoomHF)),
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_room_hf
	));
}

void EfxLtFilter::set_efx_reverb_gain_lf()
{
	if (is_std_reverb_)
	{
		return;
	}

	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::gain_lf];

	const auto efx_room_lf = ul::Algorithm::clamp(
		AudioUtils::level_mb_to_gain(static_cast<float>(eax_reverb_.lRoomLF)),
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_room_lf
	));
}

void EfxLtFilter::set_efx_reverb_decay_time()
{
	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::decay_time];

	const auto efx_decay_time = ul::Algorithm::clamp(
		eax_reverb_.flDecayTime,
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_decay_time
	));
}

void EfxLtFilter::set_efx_reverb_decay_hf_ratio()
{
	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::decay_hf_ratio];

	const auto efx_decay_hf_ratio = ul::Algorithm::clamp(
		eax_reverb_.flDecayHFRatio,
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_decay_hf_ratio
	));
}

void EfxLtFilter::set_efx_reverb_decay_lf_ratio()
{
	if (is_std_reverb_)
	{
		return;
	}

	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::decay_lf_ratio];

	const auto efx_decay_lf_ratio = ul::Algorithm::clamp(
		eax_reverb_.flDecayLFRatio,
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_decay_lf_ratio
	));
}

void EfxLtFilter::set_efx_reverb_reflections_gain()
{
	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::reflections_gain];

	const auto efx_reflections = ul::Algorithm::clamp(
		AudioUtils::level_mb_to_gain(static_cast<float>(eax_reverb_.lReflections)),
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_reflections
	));
}

void EfxLtFilter::set_efx_reverb_reflections_delay()
{
	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::reflections_delay];

	const auto efx_reflections_delay = ul::Algorithm::clamp(
		eax_reverb_.flReflectionsDelay,
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_reflections_delay
	));
}

void EfxLtFilter::set_efx_reverb_reflections_pan()
{
	if (is_std_reverb_)
	{
		return;
	}

	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::reflections_pan];

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectfv(
		reverb_effect_.get(),
		descriptor.param,
		&eax_reverb_.vReflectionsPan.x
	));
}

void EfxLtFilter::set_efx_reverb_late_reverb_gain()
{
	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::late_reverb_gain];

	const auto efx_reverb = ul::Algorithm::clamp(
		AudioUtils::level_mb_to_gain(static_cast<float>(eax_reverb_.lReverb)),
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_reverb
	));
}

void EfxLtFilter::set_efx_reverb_late_reverb_delay()
{
	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::late_reverb_delay];

	const auto efx_reverb_delay = ul::Algorithm::clamp(
		eax_reverb_.flReverbDelay,
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_reverb_delay
	));
}

void EfxLtFilter::set_efx_reverb_late_reverb_pan()
{
	if (is_std_reverb_)
	{
		return;
	}

	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::late_reverb_pan];

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectfv(
		reverb_effect_.get(),
		descriptor.param,
		&eax_reverb_.vReverbPan.x
	));
}

void EfxLtFilter::set_efx_reverb_echo_time()
{
	if (is_std_reverb_)
	{
		return;
	}

	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::echo_time];

	const auto efx_echo_time = ul::Algorithm::clamp(
		eax_reverb_.flEchoTime,
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_echo_time
	));
}

void EfxLtFilter::set_efx_reverb_echo_depth()
{
	if (is_std_reverb_)
	{
		return;
	}

	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::echo_depth];

	const auto efx_echo_depth = ul::Algorithm::clamp(
		eax_reverb_.flEchoDepth,
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_echo_depth
	));
}

void EfxLtFilter::set_efx_reverb_modulation_time()
{
	if (is_std_reverb_)
	{
		return;
	}

	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::modulation_time];

	const auto efx_modulation_time = ul::Algorithm::clamp(
		eax_reverb_.flModulationTime,
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_modulation_time
	));
}

void EfxLtFilter::set_efx_reverb_modulation_depth()
{
	if (is_std_reverb_)
	{
		return;
	}

	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::modulation_depth];

	const auto efx_modulation_depth = ul::Algorithm::clamp(
		eax_reverb_.flModulationDepth,
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_modulation_depth
	));
}

void EfxLtFilter::set_efx_reverb_air_absorption_gain_hf()
{
	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::air_absorption_gain_hf];

	const auto efx_air_absorption_hf = ul::Algorithm::clamp(
		AudioUtils::level_mb_to_gain(eax_reverb_.flAirAbsorptionHF),
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_air_absorption_hf
	));
}

void EfxLtFilter::set_efx_reverb_hf_reference()
{
	if (is_std_reverb_)
	{
		return;
	}

	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::hf_reference];

	const auto efx_hf_reference = ul::Algorithm::clamp(
		eax_reverb_.flHFReference,
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_hf_reference
	));
}

void EfxLtFilter::set_efx_reverb_lf_reference()
{
	if (is_std_reverb_)
	{
		return;
	}

	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::lf_reference];

	const auto efx_lf_reference = ul::Algorithm::clamp(
		eax_reverb_.flLFReference,
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_lf_reference
	));
}

void EfxLtFilter::set_efx_reverb_room_rolloff_factor()
{
	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::room_rolloff_factor];

	const auto efx_room_rolloff_factor = ul::Algorithm::clamp(
		eax_reverb_.flRoomRolloffFactor,
		descriptor.min_value,
		descriptor.max_value
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffectf(
		reverb_effect_.get(),
		descriptor.param,
		efx_room_rolloff_factor
	));
}

void EfxLtFilter::set_efx_reverb_decay_hf_limit()
{
	const auto& descriptor = al_xreverb_descriptors_[XReverbIndex::decay_hf_limit];

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alEffecti(
		reverb_effect_.get(),
		descriptor.param,
		(eax_reverb_.ulFlags & EAXREVERBFLAGS_DECAYHFLIMIT) != 0
	));
}

void EfxLtFilter::set_efx_reverb_all()
{
	set_efx_reverb_density();
	set_efx_reverb_diffusion();
	set_efx_reverb_gain();
	set_efx_reverb_gain_hf();
	set_efx_reverb_gain_lf();
	set_efx_reverb_decay_time();
	set_efx_reverb_decay_hf_ratio();
	set_efx_reverb_decay_lf_ratio();
	set_efx_reverb_reflections_gain();
	set_efx_reverb_reflections_delay();
	set_efx_reverb_reflections_pan();
	set_efx_reverb_late_reverb_gain();
	set_efx_reverb_late_reverb_delay();
	set_efx_reverb_late_reverb_pan();
	set_efx_reverb_echo_time();
	set_efx_reverb_echo_depth();
	set_efx_reverb_modulation_time();
	set_efx_reverb_modulation_depth();
	set_efx_reverb_air_absorption_gain_hf();
	set_efx_reverb_hf_reference();
	set_efx_reverb_lf_reference();
	set_efx_reverb_room_rolloff_factor();
	set_efx_reverb_decay_hf_limit();
}

void EfxLtFilter::set_efx_slot_effect_gain(
	float gain)
{
	const auto efx_gain = ul::Algorithm::clamp(gain, 0.0F, 1.0F);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alAuxiliaryEffectSlotf(
		effect_slot_.get(),
		AL_EFFECTSLOT_GAIN,
		efx_gain
	));
}

void EfxLtFilter::mute_efx_slot_effect()
{
	if (is_listener_muted_)
	{
		return;
	}

	is_listener_muted_ = true;

	set_efx_slot_effect_gain(0.0F);
}

void EfxLtFilter::unmute_efx_slot_effect()
{
	if (!is_listener_muted_)
	{
		return;
	}

	is_listener_muted_ = false;

	set_efx_slot_effect_gain(1.0F);
}

void EfxLtFilter::set_efx_effect()
{
	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alAuxiliaryEffectSloti(
		effect_slot_.get(),
		AL_EFFECTSLOT_EFFECT,
		reverb_effect_.get()
	));
}

void EfxLtFilter::set_reverb_environment(
	std::uint32_t environment)
{
	assert(
		environment >= ::EAXREVERB_MINENVIRONMENT &&
		environment <= ::EAX20REVERB_MAXENVIRONMENT);

	is_reverb_dirty_ = true;
	eax_reverb_ = ::EAXREVERB_PRESETS[environment];

	set_efx_reverb_all();
}

void EfxLtFilter::set_reverb_room(
	std::int32_t room)
{
	assert(
		room >= ::EAXREVERB_MINROOM &&
		room <=	::EAXREVERB_MAXROOM);

	if (eax_reverb_.lRoom == room)
	{
		return;
	}

	is_reverb_dirty_ = true;
	eax_reverb_.lRoom = room;

	set_efx_reverb_gain();
}

void EfxLtFilter::set_reverb_room_hf(
	std::int32_t room_hf)
{
	assert(
		room_hf >= ::EAXREVERB_MINROOMHF &&
		room_hf <= ::EAXREVERB_MAXROOMHF);

	if (eax_reverb_.lRoomHF == room_hf)
	{
		return;
	}

	is_reverb_dirty_ = true;
	eax_reverb_.lRoomHF = room_hf;

	set_efx_reverb_gain_hf();
}

void EfxLtFilter::set_reverb_room_rolloff_factor(
	float room_rolloff_factor)
{
	assert(
		room_rolloff_factor >= ::EAXREVERB_MINROOMROLLOFFFACTOR &&
		room_rolloff_factor <= ::EAXREVERB_MAXROOMROLLOFFFACTOR);

	if (eax_reverb_.flRoomRolloffFactor == room_rolloff_factor)
	{
		return;
	}

	is_reverb_dirty_ = true;
	eax_reverb_.flRoomRolloffFactor = room_rolloff_factor;

	set_efx_reverb_room_rolloff_factor();
}

void EfxLtFilter::set_reverb_decay_time(
	float decay_time)
{
	assert(
		decay_time >= ::EAXREVERB_MINDECAYTIME &&
		decay_time <= ::EAXREVERB_MAXDECAYTIME);

	if (eax_reverb_.flDecayTime == decay_time)
	{
		return;
	}

	is_reverb_dirty_ = true;
	eax_reverb_.flDecayTime = decay_time;

	set_efx_reverb_decay_time();
}

void EfxLtFilter::set_reverb_decay_hf_ratio(
	float decay_hf_ratio)
{
	assert(
		decay_hf_ratio >= ::EAXREVERB_MINDECAYHFRATIO &&
		decay_hf_ratio <= ::EAXREVERB_MAXDECAYHFRATIO);

	if (eax_reverb_.flDecayHFRatio == decay_hf_ratio)
	{
		return;
	}

	is_reverb_dirty_ = true;
	eax_reverb_.flDecayHFRatio = decay_hf_ratio;

	set_efx_reverb_decay_hf_ratio();
}

void EfxLtFilter::set_reverb_reflections(
	std::int32_t reflections)
{
	assert(
		reflections >= ::EAXREVERB_MINREFLECTIONS &&
		reflections <= ::EAXREVERB_MAXREFLECTIONS);

	if (eax_reverb_.lReflections == reflections)
	{
		return;
	}

	is_reverb_dirty_ = true;
	eax_reverb_.lReflections = reflections;

	set_efx_reverb_reflections_gain();
}

void EfxLtFilter::set_reverb_reflections_delay(
	float reflections_delay)
{
	assert(
		reflections_delay >= ::EAXREVERB_MINREFLECTIONSDELAY &&
		reflections_delay <= ::EAXREVERB_MAXREFLECTIONSDELAY);

	if (eax_reverb_.flReflectionsDelay == reflections_delay)
	{
		return;
	}

	is_reverb_dirty_ = true;
	eax_reverb_.flReflectionsDelay = reflections_delay;

	set_efx_reverb_reflections_delay();
}

void EfxLtFilter::set_reverb_reverb(
	std::int32_t reverb)
{
	assert(
		reverb >= ::EAXREVERB_MINREVERB &&
		reverb <= ::EAXREVERB_MAXREVERB);

	if (eax_reverb_.lReverb == reverb)
	{
		return;
	}

	is_reverb_dirty_ = true;
	eax_reverb_.lReverb = reverb;

	set_efx_reverb_late_reverb_gain();
}

void EfxLtFilter::set_reverb_reverb_delay(
	float reverb_delay)
{
	assert(
		reverb_delay >= ::EAXREVERB_MINREVERBDELAY &&
		reverb_delay <= ::EAXREVERB_MAXREVERBDELAY);

	if (eax_reverb_.flReverbDelay == reverb_delay)
	{
		return;
	}

	is_reverb_dirty_ = true;
	eax_reverb_.flReverbDelay = reverb_delay;

	set_efx_reverb_late_reverb_delay();
}

void EfxLtFilter::set_reverb_diffusion(
	float diffusion)
{
	assert(
		diffusion >= ::EAXREVERB_MINENVIRONMENTDIFFUSION &&
		diffusion <= ::EAXREVERB_MAXENVIRONMENTDIFFUSION);

	if (eax_reverb_.flEnvironmentDiffusion == diffusion)
	{
		return;
	}

	is_reverb_dirty_ = true;
	eax_reverb_.flEnvironmentDiffusion = diffusion;

	set_efx_reverb_diffusion();
}

void EfxLtFilter::set_reverb_environment_size(
	float environment_size)
{
	assert(
		environment_size >= ::EAXREVERB_MINENVIRONMENTSIZE &&
		environment_size <= ::EAXREVERB_MAXENVIRONMENTSIZE);

	if (eax_reverb_.flEnvironmentSize == environment_size)
	{
		return;
	}

	is_reverb_dirty_ = true;
	eax_reverb_.flEnvironmentSize = environment_size;

	set_efx_reverb_density();
}

void EfxLtFilter::set_reverb_air_absorption_hf(
	float air_absorption_hf)
{
	assert(
		air_absorption_hf >= ::EAXREVERB_MINAIRABSORPTIONHF &&
		air_absorption_hf <= ::EAXREVERB_MAXAIRABSORPTIONHF);

	if (eax_reverb_.flAirAbsorptionHF == air_absorption_hf)
	{
		return;
	}

	is_reverb_dirty_ = true;
	eax_reverb_.flAirAbsorptionHF = air_absorption_hf;

	set_efx_reverb_air_absorption_gain_hf();
}

void EfxLtFilter::set_source_direct(
	::ALuint al_source,
	int direct)
{
	assert(
		direct >= ::EAXBUFFER_MINDIRECT &&
		direct <= ::EAXBUFFER_MAXDIRECT);

	const auto direct_gain = ul::Algorithm::clamp(
		AudioUtils::level_mb_to_gain(static_cast<float>(direct)),
		AL_LOWPASS_MIN_GAIN,
		max_low_pass_gain_
	);

	LTJS_OAL_ENSURE_CALL_DEBUG(efx_symbols_.alFilterf(
		direct_filter_.get(),
		AL_LOWPASS_GAIN,
		direct_gain
	));

	LTJS_OAL_ENSURE_CALL_DEBUG(::alSourcei(
		al_source,
		AL_DIRECT_FILTER,
		static_cast<::ALint>(direct_filter_.get())
	));
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // oal
} // ltjs
