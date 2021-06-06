#include "ltjs_oal_eax_lt_filter.h"

#include <cassert>
#include <cmath>

#include "bibendovsky_spul_algorithm.h"

#include "ltjs_exception.h"

#include "ltjs_eax_api.h"
#include "ltjs_oal_efx_symbols.h"
#include "ltjs_oal_object.h"


namespace ul = bibendovsky::spul;


namespace ltjs
{
namespace oal
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class EaxLtFilterException :
	public SafeException
{
public:
	explicit EaxLtFilterException(
		const char* message)
		:
		SafeException{message}
	{
	}
}; // EaxLtFilterException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

struct EaxLtFilterAssertEaxAlResult
{
	explicit EaxLtFilterAssertEaxAlResult(
		::ALenum eax_al_result)
	{
		assert(eax_al_result == AL_NO_ERROR);
	}
}; // EaxLtFilterAssertEaxAlResult

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

EaxLtFilter::EaxLtFilter()
{
	detect_eax();

	set_eax_reverb_defaults();

	is_reverb_dirty_ = true;
	eax_reverb_room_ = eax_reverb_.lRoom;

	set_eax_listener();
}

// -------------------------------------------------------------------------
// LtFilter

const LtFilterInfo& EaxLtFilter::get_info() const noexcept
{
	return info_;
}

void EaxLtFilter::initialize_source(
	::ALuint al_source,
	int& lt_filter_direct_mb)
{
	ensure_al_source(al_source);

	lt_filter_direct_mb = ::EAXBUFFER_DEFAULTDIRECT;

	set_source_direct(al_source, lt_filter_direct_mb);
}

void EaxLtFilter::set_listener(
	LtFilterState filter_state,
	const LTSOUNDFILTERDATA& lt_filter_data)
{
	ensure_lt_reverb_filter(lt_filter_data);

	if (filter_state == LtFilterState::disable)
	{
		if (!is_listener_muted_)
		{
			is_listener_muted_ = true;
			is_reverb_dirty_ = true;
			eax_reverb_room_ = eax_reverb_.lRoom;
			eax_reverb_.lRoom = ::EAXREVERB_MINROOM;
		}
	}
	else
	{
		if (is_listener_muted_)
		{
			is_listener_muted_ = false;
			is_reverb_dirty_ = true;
			eax_reverb_.lRoom = eax_reverb_room_;
		}
	}

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

	if (is_reverb_dirty_)
	{
		is_reverb_dirty_ = false;
		set_eax_listener();
	}
}

void EaxLtFilter::set_source(
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

void EaxLtFilter::detect_eax()
{
	const auto has_eax_extension = (::alIsExtensionPresent("EAX2.0") != AL_FALSE);

	if (!has_eax_extension)
	{
		throw EaxLtFilterException{"EAX extension not found."};
	}

	eax_get_ = reinterpret_cast<::EAXGet>(::alGetProcAddress("EAXGet"));
	eax_set_ = reinterpret_cast<::EAXSet>(::alGetProcAddress("EAXSet"));

	if (eax_get_ == nullptr || eax_set_ == nullptr)
	{
		throw EaxLtFilterException{"EAX functions not found."};
	}

	dummy_source_object_ = make_source_object();

	info_.name = "EAX 2.0";
}

void EaxLtFilter::ensure_lt_reverb_filter(
	const LTSOUNDFILTERDATA& lt_filter_data)
{
	if (lt_filter_data.uiFilterType != FilterReverb)
	{
		throw EaxLtFilterException{"Expected LT reverb filter."};
	}
}

void EaxLtFilter::ensure_al_source(
	::ALuint al_source)
{
	if (al_source == 0)
	{
		throw EaxLtFilterException{"Null AL source."};
	}
}

EAXLISTENERPROPERTIES EaxLtFilter::make_eax_listener(
	const EAXREVERBPROPERTIES& eax_reverb) noexcept
{
	auto eax_listener = ::EAXLISTENERPROPERTIES{};

	eax_listener.dwEnvironment = eax_reverb.ulEnvironment;
	eax_listener.flEnvironmentSize = eax_reverb.flEnvironmentSize;
	eax_listener.flEnvironmentDiffusion = eax_reverb.flEnvironmentDiffusion;
	eax_listener.lRoom = eax_reverb.lRoom;
	eax_listener.lRoomHF = eax_reverb.lRoomHF;
	eax_listener.flDecayTime = eax_reverb.flDecayTime;
	eax_listener.flDecayHFRatio = eax_reverb.flDecayHFRatio;
	eax_listener.lReflections = eax_reverb.lReflections;
	eax_listener.flReflectionsDelay = eax_reverb.flReflectionsDelay;
	eax_listener.lReverb = eax_reverb.lReverb;
	eax_listener.flReverbDelay = eax_reverb.flReverbDelay;
	eax_listener.flAirAbsorptionHF = eax_reverb.flAirAbsorptionHF;
	eax_listener.flRoomRolloffFactor = eax_reverb.flRoomRolloffFactor;
	eax_listener.dwFlags = eax_reverb.ulFlags;

	return eax_listener;
}

void EaxLtFilter::set_eax_reverb_defaults()
{
	eax_reverb_ = make_eax_listener(::EAXREVERB_PRESETS[::EAX_ENVIRONMENT_GENERIC]);
}

void EaxLtFilter::set_reverb_environment(
	std::uint32_t environment)
{
	assert(
		environment >= ::EAXREVERB_MINENVIRONMENT &&
		environment <= ::EAX20REVERB_MAXENVIRONMENT);

	is_reverb_dirty_ = true;
	eax_reverb_ = make_eax_listener(::EAXREVERB_PRESETS[environment]);
}

void EaxLtFilter::set_reverb_room(
	std::int32_t room)
{
	assert(
		room >= ::EAXREVERB_MINROOM &&
		room <=	::EAXREVERB_MAXROOM);

	if (is_listener_muted_)
	{
		if (eax_reverb_room_ == room)
		{
			return;
		}

		eax_reverb_room_ = room;
	}
	else
	{
		if (eax_reverb_.lRoom == room)
		{
			return;
		}

		is_reverb_dirty_ = true;
		eax_reverb_.lRoom = room;
	}
}

void EaxLtFilter::set_reverb_room_hf(
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
}

void EaxLtFilter::set_reverb_room_rolloff_factor(
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
}

void EaxLtFilter::set_reverb_decay_time(
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
}

void EaxLtFilter::set_reverb_decay_hf_ratio(
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
}

void EaxLtFilter::set_reverb_reflections(
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
}

void EaxLtFilter::set_reverb_reflections_delay(
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
}

void EaxLtFilter::set_reverb_reverb(
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
}

void EaxLtFilter::set_reverb_reverb_delay(
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
}

void EaxLtFilter::set_reverb_diffusion(
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
}

void EaxLtFilter::set_reverb_environment_size(
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
}

void EaxLtFilter::set_reverb_air_absorption_hf(
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
}

void EaxLtFilter::set_eax_listener()
{
	EaxLtFilterAssertEaxAlResult{eax_set_(
		&::DSPROPSETID_EAX20_ListenerProperties,
		::DSPROPERTY_EAXLISTENER_ALLPARAMETERS,
		0,
		&eax_reverb_,
		static_cast<::ALuint>(sizeof(eax_reverb_))
	)};
}

void EaxLtFilter::set_source_direct(
	::ALuint al_source,
	std::int32_t direct)
{
	assert(
		direct >= ::EAXBUFFER_MINDIRECT &&
		direct <= ::EAXBUFFER_MAXDIRECT);

	EaxLtFilterAssertEaxAlResult{eax_set_(
		&::DSPROPSETID_EAX20_BufferProperties,
		::DSPROPERTY_EAXBUFFER_DIRECT,
		al_source,
		&direct,
		static_cast<::ALuint>(sizeof(direct))
	)};
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // oal
} // ltjs
