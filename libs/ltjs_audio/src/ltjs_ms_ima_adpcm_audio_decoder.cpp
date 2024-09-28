/*
References:
  - IMA ADPCM
    https://wiki.multimedia.cx/index.php/IMA_ADPCM
  - Microsoft IMA ADPCM
    https://wiki.multimedia.cx/index.php/Microsoft_IMA_ADPCM
  - Recommended Practices for Enhancing Digital Audio Compatibility in Multimedia Systems
    http://www.cs.columbia.edu/~hgs/audio/dvi/IMA_ADPCM.pdf
*/

#include "ltjs_ms_ima_adpcm_audio_decoder.h"

#include <cassert>

#include <algorithm>

#include "ltjs_audio_limits.h"

// ==========================================================================

namespace ltjs {

namespace ul = bibendovsky::spul;

// ==========================================================================

const MsImaAdpcmAudioDecoder::IndexTable MsImaAdpcmAudioDecoder::index_table =
{
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8,
};

const MsImaAdpcmAudioDecoder::StepsizeTable MsImaAdpcmAudioDecoder::stepsize_table =
{
	    7,     8,     9,    10,    11,    12,    13,    14,
	   16,    17,    19,    21,    23,    25,    28,    31,
	   34,    37,    41,    45,    50,    55,    60,    66,
	   73,    80,    88,    97,   107,   118,   130,   143,
	  157,   173,   190,   209,   230,   253,   279,   307,
	  337,   371,   408,   449,   494,   544,   598,   658,
	  724,   796,   876,   963,  1060,  1166,  1282,  1411,
	 1552,  1707,  1878,  2066,  2272,  2499,  2749,  3024,
	 3327,  3660,  4026,  4428,  4871,  5358,  5894,  6484,
	 7132,  7845,  8630,  9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
	32767,
};

// --------------------------------------------------------------------------

MsImaAdpcmAudioDecoder::MsImaAdpcmAudioDecoder() noexcept = default;

MsImaAdpcmAudioDecoder::MsImaAdpcmAudioDecoder(const MsImaAdpcmAudioDecoderOpenParam& param) noexcept
{
	open(param);
}

MsImaAdpcmAudioDecoder::~MsImaAdpcmAudioDecoder() = default;

bool MsImaAdpcmAudioDecoder::open(const MsImaAdpcmAudioDecoderOpenParam& param) noexcept
{
	close();

	if (!validate(param))
	{
		return false;
	}

	if (!open_internal(param))
	{
		close();
		return false;
	}

	is_open_ = true;
	return is_open();
}

void MsImaAdpcmAudioDecoder::close() noexcept
{
	is_open_ = false;
	is_failed_ = false;
	dst_channel_count_ = 0;
	dst_bit_depth_ = 0;
	dst_sample_rate_ = 0;
	total_frame_count_ = 0;
	total_byte_count_ = 0;
	total_byte_offset_ = 0;
	block_size_ = 0;
	block_header_size_ = 0;
	cache_byte_count_ = 0;
	cache_byte_offset_ = 0;
	stream_ = nullptr;
}

int MsImaAdpcmAudioDecoder::decode(void* buffer, int buffer_size) noexcept
{
	if (!is_open())
	{
		assert(false && "Closed.");
		return -1;
	}

	if (is_failed())
	{
		assert(false && "Failed.");
		return -1;
	}

	if (buffer == nullptr)
	{
		assert(false && "Null buffer.");
		return -1;
	}

	if (buffer_size < 0)
	{
		assert(false && "Negative buffer size.");
		return -1;
	}

	auto dst_bytes = static_cast<std::uint8_t*>(buffer);
	auto dst_offset = 0;

	while (dst_offset < buffer_size)
	{
		if (!fill_cache())
		{
			return -1;
		}

		if (cache_byte_offset_ == cache_byte_count_)
		{
			break;
		}

		const auto byte_count_to_copy = std::min(cache_byte_count_ - cache_byte_offset_, buffer_size - dst_offset);
		auto src_bytes = &sample_cache_[cache_byte_offset_];
		std::copy_n(src_bytes, byte_count_to_copy, &dst_bytes[dst_offset]);
		dst_offset += byte_count_to_copy;
		total_byte_offset_ += byte_count_to_copy;
		cache_byte_offset_ += byte_count_to_copy;
	}

	return dst_offset;
}

bool MsImaAdpcmAudioDecoder::set_clamped_position(int frame_offset) noexcept
{
	if (!is_open())
	{
		assert(false && "Closed.");
		return false;
	}

	if (is_failed())
	{
		assert(false && "Failed.");
		return false;
	}

	if (frame_offset < 0)
	{
		assert(false && "Negative frame offset.");
		frame_offset = 0;
	}

	if (frame_offset > total_frame_count_)
	{
		frame_offset = total_frame_count_;
	}

	if (frame_offset == total_frame_count_)
	{
		total_byte_offset_ = total_byte_count_;
		return true;
	}

	const auto sample_offset = frame_offset * dst_channel_count_;
	const auto byte_offset = sample_offset * dst_sample_size;

	if (byte_offset == total_byte_offset_)
	{
		return true;
	}

	const auto samples_per_block = nibbles_per_byte * (block_size_ - block_header_size_);
	const auto block_index = sample_offset / samples_per_block;
	const auto block_byte_offset = block_index * block_size_;

	if (!stream_->set_position(block_byte_offset))
	{
		assert(false && "Failed to seek a stream.");
		is_failed_ = true;
		return false;
	}

	const auto block_sample_offset = block_index * samples_per_block;
	const auto block_frame_offset = block_sample_offset / dst_channel_count_;

	if (block_frame_offset == frame_offset)
	{
		cache_byte_count_ = 0;
		cache_byte_offset_ = 0;
		return true;
	}

	fill_cache();

	const auto rest_byte_count = (frame_offset - block_frame_offset) * dst_channel_count_ * dst_sample_size;

	if (rest_byte_count >= cache_byte_count_)
	{
		assert(false && "Not enough room in cache.");
		is_failed_ = true;
		return false;
	}

	total_byte_offset_ = byte_offset;
	cache_byte_offset_ = rest_byte_count;
	return true;
}

bool MsImaAdpcmAudioDecoder::is_open() const noexcept
{
	return is_open_;
}

bool MsImaAdpcmAudioDecoder::is_failed() const noexcept
{
	return is_failed_;
}

int MsImaAdpcmAudioDecoder::get_channel_count() const noexcept
{
	assert(is_open());
	return dst_channel_count_;
}

int MsImaAdpcmAudioDecoder::get_bit_depth() const noexcept
{
	assert(is_open());
	return dst_bit_depth;
}

int MsImaAdpcmAudioDecoder::get_sample_rate() const noexcept
{
	assert(is_open());
	return dst_sample_rate_;
}

int MsImaAdpcmAudioDecoder::get_frame_count() const noexcept
{
	assert(is_open());
	return total_frame_count_;
}

int MsImaAdpcmAudioDecoder::get_data_size() const noexcept
{
	assert(is_open());
	return total_byte_count_;
}

bool MsImaAdpcmAudioDecoder::validate(const MsImaAdpcmAudioDecoderOpenParam& param)
{
	switch (param.channel_count)
	{
		case 1:
		case 2:
			break;

		default:
			assert(false && "Unsupported channel count.");
			return false;
	}

	switch (param.bit_depth)
	{
		case 4:
			break;

		default:
			assert(false && "Unsupported bit depth.");
			return false;
	}

	if (param.sample_rate < AudioLimits::min_sample_rate ||
		param.sample_rate > AudioLimits::max_sample_rate)
	{
		assert(false && "Unsupported sample rate.");
		return false;
	}

	switch (param.channel_count)
	{
		case 1:
			if (param.block_size <= 4 || param.block_size > max_block_size)
			{
				assert(false && "Unsupported mono block size.");
				return false;
			}

			break;

		case 2:
			if (param.block_size <= 8 || param.block_size > max_block_size)
			{
				assert(false && "Unsupported stereo block size.");
				return false;
			}

			break;

		default:
			assert(false && "Unsupported channel count.");
			return false;
	}

	if (param.frame_count < 0 || param.frame_count > AudioLimits::max_frame_count)
	{
		assert(false && "Unsupported frame count.");
		return false;
	}

	if (param.stream == nullptr)
	{
		assert(false && "Null stream.");
		return false;
	}

	if (!param.stream->is_open())
	{
		assert(false && "Closed stream.");
		return false;
	}

	if (!param.stream->is_readable())
	{
		assert(false && "Non-readable stream.");
		return false;
	}

	if (!param.stream->is_seekable())
	{
		assert(false && "Non-seekable stream.");
		return false;
	}

	return true;
}

int MsImaAdpcmAudioDecoder::clamp_index(int index)
{
	return clamp(index, 0, stepsize_table_size - 1);
}

int MsImaAdpcmAudioDecoder::clamp_sample(int sample)
{
	return clamp(sample, static_cast<int>(INT16_MIN), static_cast<int>(INT16_MAX));
}

void MsImaAdpcmAudioDecoder::decode_block_header()
{
	auto bytes = block_cache_;

	for (auto i_channel = 0; i_channel < dst_channel_count_; ++i_channel)
	{
		predicted_samples_[i_channel] = static_cast<std::int16_t>(static_cast<std::uint16_t>((bytes[1] << 8) | bytes[0]));
		stepsize_indices_[i_channel] = clamp_index(bytes[2]);
		bytes += 4;
	}
}

int MsImaAdpcmAudioDecoder::get_lo_nibble(int byte) noexcept
{
	return byte & 0B1111;
}

int MsImaAdpcmAudioDecoder::get_hi_nibble(int byte) noexcept
{
	return byte >> 4;
}

std::int16_t MsImaAdpcmAudioDecoder::decode_nibble(int nibble, int& predicted_sample, int& step_index)
{
	const auto stepsize = stepsize_table[step_index];
	auto difference = (((2 * (nibble & 0B0111)) + 1) * stepsize) / 8;

	if ((nibble & 0B1000) != 0)
	{
		difference = -difference;
	}

	predicted_sample = clamp_sample(predicted_sample + difference);
	step_index = clamp_index(step_index + index_table[nibble]);
	return static_cast<std::int16_t>(predicted_sample);
}

bool MsImaAdpcmAudioDecoder::fill_cache()
{
	if (cache_byte_offset_ != cache_byte_count_)
	{
		return true;
	}

	cache_byte_count_ = 0;
	cache_byte_offset_ = 0;

	const auto block_size = stream_->read(block_cache_, block_size_);

	if (block_size < 0)
	{
		assert(false && "Failed to read a block from a stream.");
		is_failed_ = true;
		return false;
	}

	if (block_size <= block_header_size_)
	{
		return true;
	}

	decode_block_header();

	if (dst_channel_count_ == 1)
	{
		decode_mono(block_size);
	}
	else
	{
		decode_stereo(block_size);
	}

	return true;
}

void MsImaAdpcmAudioDecoder::decode_mono(int block_size)
{
	const auto rest_block_byte_count = dst_sample_size * nibbles_per_byte * (block_size - block_header_size_);
	const auto dst_rest_byte_count = total_byte_count_ - total_byte_offset_;
	cache_byte_count_ = std::min(rest_block_byte_count, dst_rest_byte_count);
	auto dst_samples = reinterpret_cast<std::int16_t*>(sample_cache_);
	auto& predicted_sample = predicted_samples_[0];
	auto& stepsize_index = stepsize_indices_[0];

	std::for_each(
		&block_cache_[block_header_size_],
		&block_cache_[block_size],
		[&dst_samples, &predicted_sample, &stepsize_index](std::uint8_t src_byte)
		{
			dst_samples[0] = decode_nibble(get_lo_nibble(src_byte), predicted_sample, stepsize_index);
			dst_samples[1] = decode_nibble(get_hi_nibble(src_byte), predicted_sample, stepsize_index);
			dst_samples += 2;
		}
	);
}

void MsImaAdpcmAudioDecoder::decode_stereo(int block_size)
{
	const auto aligned_block_size = (block_size / 8) * 8;
	const auto rest_block_byte_count = dst_sample_size * nibbles_per_byte * (aligned_block_size - block_header_size_);
	const auto dst_rest_byte_count = total_byte_count_ - total_byte_offset_;
	cache_byte_count_ = std::min(rest_block_byte_count, dst_rest_byte_count);
	auto dst_samples = reinterpret_cast<std::int16_t*>(sample_cache_);

	for (auto i_byte = block_header_size_; i_byte < aligned_block_size; i_byte += 8)
	{
		auto src_bytes = &block_cache_[i_byte];

		for (auto i_channel = 0; i_channel < 2; ++i_channel)
		{
			auto& predicted_sample = predicted_samples_[i_channel];
			auto& stepsize_index = stepsize_indices_[i_channel];

			for (auto i_sample = 0; i_sample < 4; ++i_sample)
			{
				const auto src_byte = *src_bytes++;
				dst_samples[0] = decode_nibble(get_lo_nibble(src_byte), predicted_sample, stepsize_index);
				dst_samples[1] = decode_nibble(get_hi_nibble(src_byte), predicted_sample, stepsize_index);
				dst_samples += 2;
			}
		}
	}
}

bool MsImaAdpcmAudioDecoder::open_internal(const MsImaAdpcmAudioDecoderOpenParam& param) noexcept
{
	stream_ = param.stream;
	dst_channel_count_ = param.channel_count;
	dst_bit_depth_ = dst_bit_depth;
	dst_sample_rate_ = param.sample_rate;
	total_frame_count_ = param.frame_count;
	total_byte_count_ = dst_byte_depth * dst_channel_count_ * total_frame_count_;
	block_size_ = param.block_size;
	block_header_size_ = dst_channel_count_ * 4;
	return true;
}

} // namespace ltjs
