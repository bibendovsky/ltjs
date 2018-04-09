#include "ltjs_audio_utils.h"
#include <cmath>
#include <array>
#include <memory>
#include "bibendovsky_spul_algorithm.h"
#include "bibendovsky_spul_memory_stream.h"


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


	static constexpr auto ds_max_volumes = ds_max_volume - ds_min_volume + 1;

	// DS volume -> gain
	// [0..10000]
	using DsVolumeToGainTable = std::array<float, ds_max_volumes>;


	struct UPtrDeleter
	{
		void operator()(
			char* ptr)
		{
			deallocate(ptr);
		}
	}; // UPtrDeleter


	static VolumeDsTable volume_ds_table;
	static VolumeGainTable volume_gain_table;

	static PanDsTable pan_ds_table;
	static PanGainTable pan_gain_table;

	static DsVolumeToGainTable ds_volume_to_gain_table;


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
				gain_item = lt_pan_to_gain(i);
			}
		}

		for (auto i = 0; i < ds_max_volumes; ++i)
		{
			ds_volume_to_gain_table[i] = static_cast<float>(std::pow(10.0, static_cast<double>(-i) / 2'000.0));
		}
	}

	static void* allocate(
		const std::size_t size)
	{
		return ::operator new(size);
	}

	static void deallocate(
		void* ptr)
	{
		return ::operator delete(ptr);
	}
}; // AudioUtils::Detail


AudioUtils::Detail::VolumeDsTable AudioUtils::Detail::volume_ds_table;
AudioUtils::Detail::VolumeGainTable AudioUtils::Detail::volume_gain_table;

AudioUtils::Detail::PanDsTable AudioUtils::Detail::pan_ds_table;
AudioUtils::Detail::PanGainTable AudioUtils::Detail::pan_gain_table;

AudioUtils::Detail::DsVolumeToGainTable AudioUtils::Detail::ds_volume_to_gain_table;


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

float AudioUtils::ds_volume_to_gain(
	const long ds_volume)
{
	return Detail::ds_volume_to_gain_table[-Detail::clamp_ds_volume(ds_volume)];
}

float AudioUtils::mb_to_gain(
	const int mb_value)
{
	const auto clamped_mb_value = ul::Algorithm::clamp(mb_value, -10'000, 10'000);
	return std::pow(10.0F, clamped_mb_value / 2'000.0F);
}

int AudioUtils::gain_to_mb(
	const float gain)
{
	const auto clamped_gain = ul::Algorithm::clamp(gain, 0.000'01F, 100'000.0F);
	return static_cast<int>(2'000.0F * std::log10(clamped_gain));
}

void* AudioUtils::allocate(
	const std::size_t size)
{
	return Detail::allocate(size);
}

void AudioUtils::deallocate(
	void* ptr)
{
	Detail::deallocate(ptr);
}

sint32 AudioUtils::decode_mp3(
	AudioDecoder& audio_decoder,
	const void* src_data_ptr,
	const uint32 src_size,
	void*& dst_wav,
	uint32& dst_wav_size)
{
	dst_wav = nullptr;
	dst_wav_size = 0;

	auto memory_stream = ul::MemoryStream{src_data_ptr, static_cast<int>(src_size), ul::Stream::OpenMode::read};

	if (!memory_stream.is_open())
	{
		return false;
	}

	if (!audio_decoder.open(&memory_stream))
	{
		return false;
	}

	if (!audio_decoder.is_mp3())
	{
		return false;
	}

	const auto max_decoded_size = audio_decoder.get_data_size();

	const auto header_size =
		4 + 4 + // "RIFF" + size
		4 + // WAVE
		4 + 4 + ul::WaveFormatEx::packed_size + // "fmt " + size + format_size
		4 + 4 + // "data" + size
		0;

	auto decoded_data = std::unique_ptr<char, Detail::UPtrDeleter>{
		static_cast<char*>(Detail::allocate(header_size + max_decoded_size))
	};

	if (!decoded_data)
	{
		return false;
	}

	const auto decoded_size = audio_decoder.decode(decoded_data.get() + header_size, max_decoded_size);

	if (decoded_size <= 0)
	{
		return false;
	}

	const auto wave_size = header_size + decoded_size;

	auto header = decoded_data.get();

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

	const auto wave_format_ex = audio_decoder.get_wave_format_ex();
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

	dst_wav = decoded_data.release();
	dst_wav_size = wave_size;

	return true;
}

void AudioUtils::initialize()
{
	Detail::initialize_tables();
}


} // ltjs
