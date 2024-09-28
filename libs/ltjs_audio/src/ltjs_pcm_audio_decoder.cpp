#include "ltjs_pcm_audio_decoder.h"

#include <cassert>

#include <algorithm>

#include "ltjs_audio_limits.h"

// ==========================================================================

namespace ltjs {

namespace ul = bibendovsky::spul;

PcmAudioDecoder::PcmAudioDecoder() noexcept = default;

PcmAudioDecoder::PcmAudioDecoder(const PcmAudioDecoderOpenParam& param) noexcept
{
	open(param);
}

PcmAudioDecoder::~PcmAudioDecoder() = default;

bool PcmAudioDecoder::open(const PcmAudioDecoderOpenParam& param) noexcept
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
	return is_open_;
}

void PcmAudioDecoder::close() noexcept
{
	is_open_ = false;
	is_failed_ = false;
	channel_count_ = 0;
	bit_depth_ = 0;
	sample_rate_ = 0;
	frame_count_ = 0;
	frame_size_ = 0;
	total_byte_count_ = 0;
	total_byte_offset_ = 0;
	cache_byte_count_ = 0;
	cache_byte_offset_ = 0;
	stream_ = nullptr;
}

int PcmAudioDecoder::decode(void* buffer, int buffer_size) noexcept
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

		const auto byte_count = std::min(cache_byte_count_ - cache_byte_offset_, buffer_size - dst_offset);
		std::copy_n(&cache_[cache_byte_offset_], byte_count, &dst_bytes[dst_offset]);
		dst_offset += byte_count;
		cache_byte_offset_ += byte_count;
		total_byte_offset_ += byte_count;
	}

	return dst_offset;
}

bool PcmAudioDecoder::set_clamped_position(int frame_offset) noexcept
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

	if (frame_offset > frame_count_)
	{
		frame_offset = frame_count_;
	}

	if (frame_offset == 0)
	{
		// Rewind.
		if (!stream_->set_position(0))
		{
			assert(false && "Stream failed to set a new position.");
			is_failed_ = true;
			return false;
		}

		total_byte_offset_ = 0;
		cache_byte_count_ = 0;
		cache_byte_offset_ = 0;
		return true;
	}

	const auto byte_offset = frame_offset * frame_size_;

	if (byte_offset == total_byte_offset_)
	{
		// Same position.
		return true;
	}

	if (!stream_->set_position(byte_offset))
	{
		assert(false && "Stream failed to set a new position.");
		is_failed_ = true;
		return false;
	}

	total_byte_offset_ = byte_offset;
	cache_byte_count_ = 0;
	cache_byte_offset_ = 0;
	return true;
}

bool PcmAudioDecoder::is_open() const noexcept
{
	return is_open_;
}

bool PcmAudioDecoder::is_failed() const noexcept
{
	return is_failed_;
}

int PcmAudioDecoder::get_channel_count() const noexcept
{
	assert(is_open());
	return channel_count_;
}

int PcmAudioDecoder::get_bit_depth() const noexcept
{
	assert(is_open());
	return bit_depth_;
}

int PcmAudioDecoder::get_sample_rate() const noexcept
{
	assert(is_open());
	return sample_rate_;
}

int PcmAudioDecoder::get_frame_count() const noexcept
{
	assert(is_open());
	return frame_count_;
}

int PcmAudioDecoder::get_data_size() const noexcept
{
	assert(is_open());
	return total_byte_count_;
}

bool PcmAudioDecoder::validate(const PcmAudioDecoderOpenParam& param) noexcept
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
		case 8:
		case 16:
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

bool PcmAudioDecoder::open_internal(const PcmAudioDecoderOpenParam& param) noexcept
{
	stream_ = param.stream;
	const auto stream_size = stream_->get_size();

	if (stream_size < 0)
	{
		assert(false && "Failed to get stream size.");
		return false;
	}

	if (stream_size > AudioLimits::max_data_size)
	{
		assert(false && "Unsupported stream size.");
		return false;
	}

	channel_count_ = param.channel_count;
	bit_depth_ = param.bit_depth;
	sample_rate_ = param.sample_rate;
	frame_size_ = channel_count_ * (bit_depth_ / 8);
	const auto src_data_size = static_cast<int>(stream_size);
	frame_count_ = src_data_size / frame_size_;

	if (frame_count_ * frame_size_ != src_data_size)
	{
		assert(false && "Data size mismatch.");
		return false;
	}

	total_byte_count_ = frame_count_ * frame_size_;
	return true;
}

bool PcmAudioDecoder::fill_cache() noexcept
{
	if (cache_byte_offset_ != cache_byte_count_)
	{
		return true;
	}

	const auto byte_count = std::min(cache_capacity, total_byte_count_ - total_byte_offset_);
	const auto read_byte_count = stream_->read(cache_, byte_count);

	if (read_byte_count < 0)
	{
		assert(false && "Failed to cache data from a stream.");
		is_failed_ = true;
		return false;
	}

	cache_byte_count_ = read_byte_count;
	cache_byte_offset_ = 0;
	return true;
}

} // namespace ltjs
