#include "ltjs_volume_utils.h"
#include <cmath>
#include "bibendovsky_spul_algorithm.h"


namespace ltjs
{


namespace ul = bibendovsky::spul;


sint32 VolumeUtils::clamp_lt_volume(
	const sint32 lt_volume)
{
	return ul::Algorithm::clamp(lt_volume, min_lt_volume, max_lt_volume);
}

long VolumeUtils::clamp_ds_volume(
	const long ds_volume)
{
	return ul::Algorithm::clamp(ds_volume, min_ds_volume, max_ds_volume);
}

long VolumeUtils::lt_to_ds(
	const sint32 lt_volume)
{
	const auto clamped_lt_volume = clamp_lt_volume(lt_volume);

	auto t =
		(1.0 - (static_cast<double>(min_lt_volume) / static_cast<double>(clamped_lt_volume))) *
		(static_cast<double>(max_lt_volume) / static_cast<double>(max_lt_volume_delta));

	const auto attenuation = std::sqrt(static_cast<double>(max_lt_volume));

	t = std::pow(t, attenuation);

	const auto ds_volume = min_ds_volume + static_cast<long>(t * max_ds_volume_delta);
	const auto clamped_ds_volume = clamp_lt_volume(ds_volume);

	return clamped_ds_volume;
}

float VolumeUtils::ds_to_gain(
	const long ds_volume)
{
	const auto clamped_ds_volume = clamp_lt_volume(ds_volume);
	return static_cast<float>(std::pow(10.0, static_cast<double>(clamped_ds_volume) / 2000.0));
}

float VolumeUtils::lt_to_gain(
	const sint32 lt_volume)
{
	const auto ds_volume = lt_to_ds(lt_volume);
	const auto gain = ds_to_gain(ds_volume);

	return gain;
}


} // ltjs
