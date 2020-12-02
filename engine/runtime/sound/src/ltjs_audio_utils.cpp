#include "ltjs_audio_utils.h"

#include <cassert>
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
	struct UPtrDeleter
	{
		void operator()(
			char* ptr)
		{
			deallocate(ptr);
		}
	}; // UPtrDeleter


	static float gain_to_level_mb(
		float gain) noexcept
	{
		return 2'000.0F * std::log10(gain);
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


float AudioUtils::lt_volume_to_gain(
	int lt_volume) noexcept
{
	assert(lt_volume >= min_lt_volume && lt_volume <= max_lt_volume);

	const auto gain =
		static_cast<float>(lt_volume - min_lt_volume) /
			static_cast<float>(max_lt_volume_delta);

	return gain;
}

int AudioUtils::lt_volume_to_ds_level_mb(
	int lt_volume) noexcept
{
	assert(lt_volume >= min_lt_volume && lt_volume <= max_lt_volume);

	if (lt_volume == min_lt_volume)
	{
		return min_ds_level_mb;
	}
	else if (lt_volume == max_lt_volume)
	{
		return max_ds_level_mb;
	}
	else
	{
		const auto gain = lt_volume_to_gain(lt_volume);
		const auto level_mb = static_cast<int>(Detail::gain_to_level_mb(gain));

		return level_mb;
	}
}

int AudioUtils::lt_pan_to_ds_pan(
	int lt_pan) noexcept
{
	assert(lt_pan >= min_lt_pan && lt_pan <= max_lt_pan);

	//
	// There is no need to scale output DirectSound level to 33 dB,
	// like it done in original code (ConvertLinPanToLogPan; s_dx8.cpp).
	//
	// Minimum gain (1.0 / 64) gives aproximately 36 dB.
	//

	if (lt_pan < lt_pan_center)
	{
		const auto gain = 1.0F -
			(static_cast<float>(max_left_lt_pan_delta - lt_pan) /
				static_cast<float>(max_left_lt_pan_delta));

		const auto level_mb = static_cast<int>(+Detail::gain_to_level_mb(gain));

		return level_mb;
	}
	else if (lt_pan > lt_pan_center)
	{
		const auto gain = 1.0F -
			(static_cast<float>(lt_pan - lt_pan_center) /
				static_cast<float>(max_right_lt_pan_delta));

		const auto level_mb = static_cast<int>(-Detail::gain_to_level_mb(gain));

		return level_mb;
	}
	else
	{
		return max_ds_level_mb;
	}
}

void AudioUtils::lt_pan_to_gains(
	int lt_pan,
	float& left_gain,
	float& right_gain) noexcept
{
	assert(lt_pan >= min_lt_pan && lt_pan <= max_lt_pan);

	if (lt_pan < lt_pan_center)
	{
		left_gain = max_gain;

		right_gain =
			static_cast<float>(lt_pan_center - lt_pan) /
				static_cast<float>(max_left_lt_pan_delta);
	}
	else if (lt_pan > lt_pan_center)
	{
		left_gain =
			static_cast<float>(lt_pan - lt_pan_center) /
				static_cast<float>(max_right_lt_pan_delta);

		right_gain = max_gain;
	}
	else
	{
		left_gain = max_gain;
		right_gain = max_gain;
	}
}

float AudioUtils::level_mb_to_gain(
	float level_mb) noexcept
{
	if (level_mb <= min_level_mb)
	{
		return 0.0F;
	}
	else
	{
		return std::pow(10.0F, level_mb / 2'000.0F);
	}
}

float AudioUtils::generic_stream_level_mb_to_gain(
	int level_mb) noexcept
{
	assert(level_mb >= min_generic_stream_level_mb && level_mb <= max_generic_stream_level_mb);

	const auto gain =
		static_cast<float>(level_mb - min_generic_stream_level_mb) /
			static_cast<float>(max_generic_stream_level_mb_delta);

	return gain;
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


} // ltjs
