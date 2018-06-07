#include "ltjs_audio_utils.h"
#include <cmath>
#include <array>
#include <memory>
#include "bibendovsky_spul_algorithm.h"
#include "bibendovsky_spul_endian.h"
#include "bibendovsky_spul_memory_stream.h"
#include "bibendovsky_spul_riff_four_ccs.h"
#include "bibendovsky_spul_wave_format.h"
#include "bibendovsky_spul_wave_four_ccs.h"



namespace ltjs
{


namespace ul = bibendovsky::spul;


struct AudioUtils::Detail
{
	static constexpr auto max_volume_values = static_cast<int>(lt_max_volume + 1);

	// LT volume -> DirectSound volume
	using LtVolumeToDsVolumeTable = std::array<int, max_volume_values>;

	// LT volume -> gain
	using LtVolumeToGainTable = std::array<float, max_volume_values>;


	static constexpr auto max_pan_values = static_cast<int>(lt_max_pan + 1);


	// LT pan -> DirectSound pan
	using LtPanDsPanTable = std::array<int, max_pan_values>;

	// LT pan -> gain
	using LtPanToGainTable = std::array<float, max_pan_values>;


	static constexpr auto ds_max_volumes = ds_max_volume - ds_min_volume + 1;

	// DS volume -> gain
	// [0..10000]
	using DsVolumeToGainTable = std::array<float, ds_max_volumes>;

	static constexpr auto mb_volume_to_gain_zero_index = 0 - mb_min_volume;
	static constexpr auto mb_volume_max_values = mb_max_volume - mb_min_volume + 1;

	using MbVolumeToGainTable = std::array<float, mb_volume_max_values>;


	struct UPtrDeleter
	{
		void operator()(
			char* ptr)
		{
			deallocate(ptr);
		}
	}; // UPtrDeleter


	static LtVolumeToDsVolumeTable lt_volume_to_ds_volume_table;
	static LtVolumeToGainTable lt_volume_to_gain_table;

	static LtPanDsPanTable lt_pan_to_ds_pan_table;
	static LtPanToGainTable lt_pan_to_gain_table;

	static MbVolumeToGainTable mb_volume_to_gain_table;


	static sint32 clamp_lt_volume(
		const sint32 lt_volume)
	{
		return ul::Algorithm::clamp(lt_volume, lt_min_volume, lt_max_volume);
	}

	static int clamp_ds_volume(
		const int ds_volume)
	{
		return ul::Algorithm::clamp(ds_volume, ds_min_volume, ds_max_volume);
	}

	static int lt_volume_to_ds_volume(
		const sint32 lt_volume)
	{
		const auto clamped_lt_volume = clamp_lt_volume(lt_volume);

		auto t =
			(1.0 - (static_cast<double>(lt_min_volume) / static_cast<double>(clamped_lt_volume))) *
			(static_cast<double>(lt_max_volume) / static_cast<double>(lt_max_volume_delta));

		const auto attenuation = std::sqrt(static_cast<double>(lt_max_volume));

		t = std::pow(t, attenuation);

		const auto ds_volume = ds_min_volume + static_cast<int>(t * ds_max_volume_delta);
		const auto clamped_ds_volume = clamp_ds_volume(ds_volume);

		return clamped_ds_volume;
	}

	static float ds_volume_to_gain(
		const int ds_volume)
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

	static int clamp_ds_pan(
		const int ds_pan)
	{
		return ul::Algorithm::clamp(ds_pan, ds_min_pan, ds_max_pan);
	}

	static int lt_pan_to_ds_pan(
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
		auto ds_pan = static_cast<int>((t * ds_max_pan_side_delta) / max_rolloff_divider);

		if (lt_pan < lt_pan_center)
		{
			ds_pan = -ds_pan;
		}

		return ds_pan;
	}

	static float ds_pan_to_gain(
		const int ds_pan)
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
			auto& ds_item = lt_volume_to_ds_volume_table[i];
			auto& gain_item = lt_volume_to_gain_table[i];

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
			auto& ds_item = lt_pan_to_ds_pan_table[i];
			auto& gain_item = lt_pan_to_gain_table[i];

			ds_item = {};
			gain_item = {};

			if (i >= lt_min_pan && i <= lt_max_pan)
			{
				ds_item = lt_pan_to_ds_pan(i);
				gain_item = lt_pan_to_gain(i);
			}
		}

		for (auto i = 0; i < mb_volume_max_values; ++i)
		{
			const auto mb_value = i - mb_volume_to_gain_zero_index;
			mb_volume_to_gain_table[i] = std::pow(10.0F, mb_value / 2'000.0F);
		}
	}

	static void* allocate(
		const std::size_t storage_size)
	{
		return ::operator new(storage_size);
	}

	static void deallocate(
		void* storage_ptr)
	{
		return ::operator delete(storage_ptr);
	}
}; // AudioUtils::Detail


AudioUtils::Detail::LtVolumeToDsVolumeTable AudioUtils::Detail::lt_volume_to_ds_volume_table;
AudioUtils::Detail::LtVolumeToGainTable AudioUtils::Detail::lt_volume_to_gain_table;

AudioUtils::Detail::LtPanDsPanTable AudioUtils::Detail::lt_pan_to_ds_pan_table;
AudioUtils::Detail::LtPanToGainTable AudioUtils::Detail::lt_pan_to_gain_table;

AudioUtils::Detail::MbVolumeToGainTable AudioUtils::Detail::mb_volume_to_gain_table;


sint32 AudioUtils::clamp_lt_volume(
	const sint32 lt_volume)
{
	return Detail::clamp_lt_volume(lt_volume);
}

int AudioUtils::clamp_ds_volume(
	const int ds_volume)
{
	return Detail::clamp_ds_volume(ds_volume);
}

int AudioUtils::lt_volume_to_ds_volume(
	const sint32 lt_volume)
{
	return Detail::lt_volume_to_ds_volume_table[Detail::clamp_lt_volume(lt_volume)];
}

float AudioUtils::lt_volume_to_gain(
	const sint32 lt_volume)
{
	return Detail::lt_volume_to_gain_table[Detail::clamp_lt_volume(lt_volume)];
}

sint32 AudioUtils::clamp_lt_pan(
	const sint32 lt_pan)
{
	return Detail::clamp_lt_pan(lt_pan);
}

int AudioUtils::clamp_ds_pan(
	const int ds_pan)
{
	return Detail::clamp_ds_pan(ds_pan);
}

int AudioUtils::lt_pan_to_ds_pan(
	const sint32 lt_pan)
{
	return Detail::lt_pan_to_ds_pan_table[Detail::clamp_lt_pan(lt_pan)];
}

float AudioUtils::lt_pan_to_gain(
	const sint32 lt_pan)
{
	return Detail::lt_pan_to_gain_table[Detail::clamp_lt_pan(lt_pan)];
}

float AudioUtils::ds_volume_to_gain(
	const int ds_volume)
{
	return mb_volume_to_gain(Detail::clamp_ds_volume(ds_volume));
}

float AudioUtils::mb_volume_to_gain(
	const int mb_volume)
{
	const auto clamped_mb_volume = ul::Algorithm::clamp(mb_volume, mb_min_volume, mb_max_volume);
	return Detail::mb_volume_to_gain_table[clamped_mb_volume + Detail::mb_volume_to_gain_zero_index];
}

void* AudioUtils::allocate(
	const std::size_t storage_size)
{
	return Detail::allocate(storage_size);
}

void AudioUtils::deallocate(
	void* storage_ptr)
{
	Detail::deallocate(storage_ptr);
}

sint32 AudioUtils::decode_mp3(
	AudioDecoder& audio_decoder,
	const void* src_data_ptr,
	const uint32 src_data_size,
	void*& dst_wav_ptr,
	uint32& dst_wav_size)
{
	dst_wav_ptr = nullptr;
	dst_wav_size = 0;

	auto memory_stream = ul::MemoryStream{src_data_ptr, static_cast<int>(src_data_size), ul::Stream::OpenMode::read};

	if (!memory_stream.is_open())
	{
		return false;
	}

	auto decoder_param = ltjs::AudioDecoder::OpenParam{};
	decoder_param.stream_ptr_ = &memory_stream;

	if (!audio_decoder.open(decoder_param))
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
		4 + 4 + ul::WaveFormatEx::class_size + // "fmt " + size + format_size
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

	*reinterpret_cast<std::uint32_t*>(header) = ul::WaveFormatEx::class_size;
	header += 4;

	const auto wave_format_ex = audio_decoder.get_wave_format_ex();
	*reinterpret_cast<ul::WaveFormatEx*>(header) = wave_format_ex;
	header += ul::WaveFormatEx::class_size;

	// fill in DATA chunk
	header[0] = 'd';
	header[1] = 'a';
	header[2] = 't';
	header[3] = 'a';
	header += 4;

	*reinterpret_cast<std::uint32_t*>(header) = decoded_size;
	header += 4;

	dst_wav_ptr = decoded_data.release();
	dst_wav_size = wave_size;

	return true;
}

int AudioUtils::extract_wave_size(
	const void* raw_data)
{
	if (!raw_data)
	{
		return 0;
	}

	auto header = static_cast<const std::uint32_t*>(raw_data);

	const auto riff_id = ul::Endian::little(header[0]);

	if (riff_id != ul::RiffFourCcs::riff)
	{
		return 0;
	}

	const auto riff_size = ul::Endian::little(header[1]);

	constexpr auto min_riff_size =
		4 + // "WAVE"
		4 + 4 + ul::PcmWaveFormat::class_size + // "fmt " + size + pcm_wave_format
		4 + 4 + 1 + // "data" + size + min_data_size
		0;

	if (riff_size < min_riff_size)
	{
		return 0;
	}

	const auto wave_id = ul::Endian::little(header[2]);

	if (wave_id != ul::WaveFourCcs::wave)
	{
		return 0;
	}

	return riff_size + 8;
}

void AudioUtils::initialize()
{
	Detail::initialize_tables();
}


} // ltjs
