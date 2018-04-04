#include "ltjs_audio_utils.h"
#include <cmath>
#include "bibendovsky_spul_algorithm.h"


namespace ltjs
{


namespace ul = bibendovsky::spul;


sint32 AudioUtils::clamp_lt_volume(
	const sint32 lt_volume)
{
	return ul::Algorithm::clamp(lt_volume, min_lt_volume, max_lt_volume);
}

long AudioUtils::clamp_ds_volume(
	const long ds_volume)
{
	return ul::Algorithm::clamp(ds_volume, min_ds_volume, max_ds_volume);
}

long AudioUtils::lt_volume_to_ds_volume(
	const sint32 lt_volume)
{
	const auto clamped_lt_volume = clamp_lt_volume(lt_volume);

	auto t =
		(1.0 - (static_cast<double>(min_lt_volume) / static_cast<double>(clamped_lt_volume))) *
		(static_cast<double>(max_lt_volume) / static_cast<double>(max_lt_volume_delta));

	const auto attenuation = std::sqrt(static_cast<double>(max_lt_volume));

	t = std::pow(t, attenuation);

	const auto ds_volume = min_ds_volume + static_cast<long>(t * max_ds_volume_delta);
	const auto clamped_ds_volume = clamp_ds_volume(ds_volume);

	return clamped_ds_volume;
}

float AudioUtils::ds_volume_to_gain(
	const long ds_volume)
{
	const auto clamped_ds_volume = clamp_ds_volume(ds_volume);
	return static_cast<float>(std::pow(10.0, static_cast<double>(clamped_ds_volume) / 2000.0));
}

float AudioUtils::lt_to_gain(
	const sint32 lt_volume)
{
	const auto ds_volume = lt_volume_to_ds_volume(lt_volume);
	const auto gain = ds_volume_to_gain(ds_volume);

	return gain;
}

sint32 AudioUtils::clamp_lt_pan(
	const sint32 lt_pan)
{
	return ul::Algorithm::clamp(lt_pan, lt_min_pan, lt_max_pan);
}

long AudioUtils::clamp_ds_pan(
	const long ds_pan)
{
	return ul::Algorithm::clamp(ds_pan, ds_min_pan, ds_max_pan);
}

long AudioUtils::lt_pan_to_ds_pan(
	const sint32 lt_pan)
{
	static const auto max_rolloff_divider = 3.0;
	static const auto attenuation_factor = 6.0;
	static const auto attenuation = attenuation_factor * std::sqrt(static_cast<double>(lt_pan_center));

	const auto distance_from_center = std::abs(lt_pan - lt_pan_center);

	// avoid divide by 0
	if (distance_from_center == 0)
	{
		return ds_pan_center;
	}

	auto t =
		(1.0 - (static_cast<double>(lt_min_pan) / static_cast<double>(distance_from_center))) *
		(static_cast<double>(lt_pan_center) / static_cast<double>(lt_max_pan_side_delta));

	t = std::pow(t, attenuation);

	// this essentially clamps the maximum rolloff
	// DX allows 100dB max. This allows only 33 dB
	auto ds_pan = static_cast<long>((t * ds_max_pan_side_delta) / max_rolloff_divider);

	if (lt_pan < lt_pan_center)
	{
		ds_pan = -ds_pan;
	}

	return ds_pan;
}

float AudioUtils::ds_pan_to_gain(
	const long ds_pan)
{
	const auto clamped_ds_pan = clamp_ds_pan(ds_pan);

	if (clamped_ds_pan == 0)
	{
		return {};
	}

	const auto is_signed = (clamped_ds_pan < 0);

	auto gain = static_cast<float>(std::pow(10.0, static_cast<double>(-std::abs(clamped_ds_pan)) / 2000.0));

	if (is_signed)
	{
		gain = -gain;
	}

	return gain;
}

float AudioUtils::lt_pan_to_gain(
	const sint32 lt_pan)
{
	const auto ds_pan = lt_pan_to_ds_pan(lt_pan);
	const auto gain = ds_pan_to_gain(ds_pan);

	return gain;
}


} // ltjs
