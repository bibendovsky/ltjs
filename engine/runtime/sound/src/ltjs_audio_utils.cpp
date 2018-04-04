#include "ltjs_audio_utils.h"
#include <cmath>
#include <array>
#include "bibendovsky_spul_algorithm.h"


namespace ltjs
{


namespace ul = bibendovsky::spul;


struct AudioUtils::Detail
{
	static constexpr auto max_volume_values = static_cast<int>(lt_max_volume + 1);

	// LT volume -> DirectSound volume
	using VolumeDsTable = std::array<long, max_volume_values>;

	// LT volume -> gain
	using VolumeGainTable = std::array<float, max_volume_values>;


	static constexpr auto max_pan_values = static_cast<int>(lt_max_pan + 1);


	// LT pan -> DirectSound pan
	using PanDsTable = std::array<long, max_pan_values>;

	// LT pan -> gain
	using PanGainTable = std::array<float, max_pan_values>;


	static VolumeDsTable volume_ds_table;
	static VolumeGainTable volume_gain_table;

	static PanDsTable pan_ds_table;
	static PanGainTable pan_gain_table;


	static sint32 clamp_lt_volume(
		const sint32 lt_volume)
	{
		return ul::Algorithm::clamp(lt_volume, lt_min_volume, lt_max_volume);
	}

	static long clamp_ds_volume(
		const long ds_volume)
	{
		return ul::Algorithm::clamp(ds_volume, ds_min_volume, ds_max_volume);
	}

	static long lt_volume_to_ds_volume(
		const sint32 lt_volume)
	{
		const auto clamped_lt_volume = clamp_lt_volume(lt_volume);

		auto t =
			(1.0 - (static_cast<double>(lt_min_volume) / static_cast<double>(clamped_lt_volume))) *
			(static_cast<double>(lt_max_volume) / static_cast<double>(lt_max_volume_delta));

		const auto attenuation = std::sqrt(static_cast<double>(lt_max_volume));

		t = std::pow(t, attenuation);

		const auto ds_volume = ds_min_volume + static_cast<long>(t * ds_max_volume_delta);
		const auto clamped_ds_volume = clamp_ds_volume(ds_volume);

		return clamped_ds_volume;
	}

	static float ds_volume_to_gain(
		const long ds_volume)
	{
		const auto clamped_ds_volume = clamp_ds_volume(ds_volume);
		return static_cast<float>(std::pow(10.0, static_cast<double>(clamped_ds_volume) / 2000.0));
	}

	static float lt_volume_to_gain(
		const sint32 lt_volume)
	{
		const auto ds_volume = lt_volume_to_ds_volume(lt_volume);
		const auto gain = ds_volume_to_gain(ds_volume);

		return gain;
	}

	static sint32 clamp_lt_pan(
		const sint32 lt_pan)
	{
		return ul::Algorithm::clamp(lt_pan, lt_min_pan, lt_max_pan);
	}

	static long clamp_ds_pan(
		const long ds_pan)
	{
		return ul::Algorithm::clamp(ds_pan, ds_min_pan, ds_max_pan);
	}

	static long lt_pan_to_ds_pan(
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

	static float ds_pan_to_gain(
		const long ds_pan)
	{
		const auto clamped_ds_pan = clamp_ds_pan(ds_pan);

		if (clamped_ds_pan == ds_pan_center)
		{
			return 1.0F;
		}

		const auto is_signed = (clamped_ds_pan < 0);

		auto gain = static_cast<float>(std::pow(10.0, static_cast<double>(-std::abs(clamped_ds_pan)) / 2000.0));

		if (is_signed)
		{
			gain = -gain;
		}

		return gain;
	}

	static float lt_pan_to_gain(
		const sint32 lt_pan)
	{
		const auto ds_pan = lt_pan_to_ds_pan(lt_pan);

		auto gain = ds_pan_to_gain(ds_pan);

		if (ds_pan == ds_pan_center)
		{
			if (lt_pan < lt_pan_center)
			{
				gain = -gain;
			}
		}

		return gain;
	}

	static void initialize_tables()
	{
		for (auto i = 0; i < max_volume_values; ++i)
		{
			auto& ds_item = volume_ds_table[i];
			auto& gain_item = volume_gain_table[i];

			ds_item = {};
			gain_item = {};

			if (i >= lt_min_volume && i <= lt_max_volume)
			{
				ds_item = lt_volume_to_ds_volume(i);
				gain_item = lt_volume_to_gain(i);
			}
		}

		for (auto i = 0; i < max_pan_values; ++i)
		{
			auto& ds_item = pan_ds_table[i];
			auto& gain_item = pan_gain_table[i];

			ds_item = {};
			gain_item = {};

			if (i >= lt_min_pan && i <= lt_max_pan)
			{
				ds_item = lt_pan_to_ds_pan(i);

				auto wtf = false;

				if (ds_item == 0)
				{
					wtf = true;
				}

				gain_item = lt_pan_to_gain(i);
			}
		}
	}
}; // AudioUtils::Detail


AudioUtils::Detail::VolumeDsTable AudioUtils::Detail::volume_ds_table;
AudioUtils::Detail::VolumeGainTable AudioUtils::Detail::volume_gain_table;

AudioUtils::Detail::PanDsTable AudioUtils::Detail::pan_ds_table;
AudioUtils::Detail::PanGainTable AudioUtils::Detail::pan_gain_table;


sint32 AudioUtils::clamp_lt_volume(
	const sint32 lt_volume)
{
	return Detail::clamp_lt_volume(lt_volume);
}

long AudioUtils::clamp_ds_volume(
	const long ds_volume)
{
	return Detail::clamp_ds_volume(ds_volume);
}

long AudioUtils::lt_volume_to_ds_volume(
	const sint32 lt_volume)
{
	return Detail::volume_ds_table[Detail::clamp_lt_volume(lt_volume)];
}

float AudioUtils::lt_volume_to_gain(
	const sint32 lt_volume)
{
	return Detail::volume_gain_table[Detail::clamp_lt_volume(lt_volume)];
}

sint32 AudioUtils::clamp_lt_pan(
	const sint32 lt_pan)
{
	return Detail::clamp_lt_pan(lt_pan);
}

long AudioUtils::clamp_ds_pan(
	const long ds_pan)
{
	return Detail::clamp_ds_pan(ds_pan);
}

long AudioUtils::lt_pan_to_ds_pan(
	const sint32 lt_pan)
{
	return Detail::pan_ds_table[Detail::clamp_lt_pan(lt_pan)];
}

float AudioUtils::lt_pan_to_gain(
	const sint32 lt_pan)
{
	return Detail::pan_gain_table[Detail::clamp_lt_pan(lt_pan)];
}

void AudioUtils::initialize()
{
	Detail::initialize_tables();
}


} // ltjs
