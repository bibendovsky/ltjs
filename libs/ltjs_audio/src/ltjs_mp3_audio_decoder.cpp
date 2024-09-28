#include "ltjs_mp3_audio_decoder.h"

#include <cassert>

#include <algorithm>

#include "ltjs_audio_limits.h"
#include "ltjs_audio_sample_converter.h"

// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244 4267 4456)
#endif

#define MINIMP3_IMPLEMENTATION
#include "minimp3_ex.h"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// ==========================================================================

namespace ltjs {

namespace ul = bibendovsky::spul;

// ==========================================================================

Mp3AudioDecoder::Mp3AudioDecoder() noexcept
{
	mp3_io_.read = &Mp3AudioDecoder::read_callback_minimp3;
	mp3_io_.seek = &Mp3AudioDecoder::seek_callback_minimp3;
}

Mp3AudioDecoder::Mp3AudioDecoder(const Mp3AudioDecoderOpenParam& param) noexcept
	:
	Mp3AudioDecoder{}
{
	open(param);
}

Mp3AudioDecoder::~Mp3AudioDecoder()
{
	close_mp3_context();
}

bool Mp3AudioDecoder::open(const Mp3AudioDecoderOpenParam& param) noexcept
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

void Mp3AudioDecoder::close() noexcept
{
	close_mp3_context();
	is_open_ = false;
	is_failed_ = false;
	src_channel_count_ = 0;
	src_sample_rate_ = 0;
	dst_bit_depth_ = 0;
	dst_frame_size_ = 0;
	frame_count_ = 0;
	dst_total_byte_count_ = 0;
	dst_total_byte_offset_ = 0;
	cache_byte_count_ = 0;
	cache_byte_offset_ = 0;
	convert_func_ = nullptr;
}

int Mp3AudioDecoder::decode(void* buffer, int buffer_size) noexcept
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
		fill_cache();

		if (cache_byte_offset_ == cache_byte_count_)
		{
			break;
		}

		const auto byte_count = std::min(cache_byte_count_ - cache_byte_offset_, buffer_size - dst_offset);
		std::copy_n(&converted_sample_cache_[cache_byte_offset_], byte_count, &dst_bytes[dst_offset]);
		dst_offset += byte_count;
		cache_byte_offset_ += byte_count;
		dst_total_byte_offset_ += byte_count;
	}

	return dst_offset;
}

bool Mp3AudioDecoder::set_clamped_position(int frame_offset) noexcept
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

	const auto current_frame_offset = dst_total_byte_offset_ / dst_frame_size_;

	if (frame_offset == current_frame_offset)
	{
		return true;
	}

	if (mp3dec_ex_seek(&mp3_context_, frame_offset) != 0)
	{
		assert(false && "MP3 failed to seek.");
		is_failed_ = true;
		return false;
	}

	dst_total_byte_offset_ = frame_offset * dst_frame_size_;
	cache_byte_count_ = 0;
	cache_byte_offset_ = 0;
	return true;
}

bool Mp3AudioDecoder::is_open() const noexcept
{
	return is_open_;
}

bool Mp3AudioDecoder::is_failed() const noexcept
{
	return is_failed_;
}

int Mp3AudioDecoder::get_channel_count() const noexcept
{
	assert(is_open());
	return src_channel_count_;
}

int Mp3AudioDecoder::get_bit_depth() const noexcept
{
	assert(is_open());
	return dst_bit_depth_;
}

int Mp3AudioDecoder::get_sample_rate() const noexcept
{
	assert(is_open());
	return src_sample_rate_;
}

int Mp3AudioDecoder::get_frame_count() const noexcept
{
	assert(is_open());
	return frame_count_;
}

int Mp3AudioDecoder::get_data_size() const noexcept
{
	assert(is_open());
	return dst_total_byte_count_;
}

std::size_t Mp3AudioDecoder::read_callback_minimp3(void* buf, std::size_t size, void* user_data) noexcept
{
	if (buf == nullptr)
	{
		assert(false && "Null buffer.");
		return 0;
	}

	if (size > INT32_MAX)
	{
		assert(false && "Unsupported buffer size.");
		return 0;
	}

	if (user_data == nullptr)
	{
		assert(false && "Null user data.");
		return 0;
	}

	const auto read_size = static_cast<ul::Stream*>(user_data)->read(buf, static_cast<int>(size));

	if (read_size < 0)
	{
		assert(false && "Failed to read from stream.");
		return 0;
	}

	return static_cast<std::size_t>(read_size);
}

int Mp3AudioDecoder::seek_callback_minimp3(std::uint64_t position, void* user_data) noexcept
{
	if (position > INT64_MAX)
	{
		assert(false && "Unsupported position.");
		return -1;
	}

	if (user_data == nullptr)
	{
		assert(false && "Null user data.");
		return -1;
	}

	if (!static_cast<ul::Stream*>(user_data)->set_position(static_cast<std::int64_t>(position)))
	{
		assert(false && "Failed to seek stream.");
		return -1;
	}

	return 0;
}

void Mp3AudioDecoder::convert_f32_to_8_bit(int sample_count) noexcept
{
	std::transform(
		mp3_sample_cache_,
		&mp3_sample_cache_[sample_count],
		converted_sample_cache_,
		&AudioSampleConverter::f32_to_u8);
}

void Mp3AudioDecoder::convert_f32_to_16_bit(int sample_count) noexcept
{
	std::transform(
		mp3_sample_cache_,
		&mp3_sample_cache_[sample_count],
		reinterpret_cast<std::int16_t*>(converted_sample_cache_),
		&AudioSampleConverter::f32_to_s16);
}

bool Mp3AudioDecoder::validate(const Mp3AudioDecoderOpenParam& param) noexcept
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

	if (param.sample_rate < AudioLimits::min_sample_rate ||
		param.sample_rate > AudioLimits::max_sample_rate)
	{
		assert(false && "Unsupported sample rate.");
		return false;
	}

	if (param.frame_count < 0 || param.frame_count > AudioLimits::max_frame_count)
	{
		assert(false && "Unsupported frame count.");
		return false;
	}

	switch (param.dst_bit_depth)
	{
		case 8:
		case 16:
			break;

		default:
			assert(false && "Unsupported bit depth.");
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

void Mp3AudioDecoder::close_mp3_context() noexcept
{
	if (!is_mp3_context_dirty_)
	{
		return;
	}

	is_mp3_context_dirty_ = false;
	mp3dec_ex_close(&mp3_context_);
}

bool Mp3AudioDecoder::open_internal(const Mp3AudioDecoderOpenParam& param) noexcept
{
	mp3_io_.read_data = param.stream;
	mp3_io_.seek_data = param.stream;

	if (mp3dec_ex_open_cb(&mp3_context_, &mp3_io_, MP3D_SEEK_TO_SAMPLE) != 0)
	{
		assert(false && "Failed to initialize MP3 context.");
		return false;
	}

	if (mp3_context_.info.channels != param.channel_count)
	{
		assert(false && "Channel count mismatch.");
		return false;
	}

	if (mp3_context_.info.hz != param.sample_rate)
	{
		assert(false && "Sample rate mismatch.");
		return false;
	}

	if (param.frame_count > 0)
	{
		const auto mp3_max_sample_count = std::max(mp3_context_.samples, mp3_context_.detected_samples);
		const auto mp3_frame_count = mp3_max_sample_count / param.channel_count;

		if (mp3_frame_count != static_cast<unsigned int>(param.frame_count))
		{
			assert(false && "Frame count mismatch.");
			return false;
		}
	}

	switch (param.dst_bit_depth)
	{
		case 8: convert_func_ = &Mp3AudioDecoder::convert_f32_to_8_bit; break;
		case 16: convert_func_ = &Mp3AudioDecoder::convert_f32_to_16_bit; break;
		default: assert(false && "Unsupported bit depth."); return false;
	}

	src_channel_count_ = mp3_context_.info.channels;
	src_sample_rate_ = mp3_context_.info.hz;
	dst_bit_depth_ = param.dst_bit_depth;
	dst_frame_size_ = src_channel_count_ * (param.dst_bit_depth / 8);
	frame_count_ = param.frame_count;
	dst_total_byte_count_ = frame_count_ * dst_frame_size_;
	return true;
}

void Mp3AudioDecoder::fill_cache() noexcept
{
	if (cache_byte_offset_ != cache_byte_count_)
	{
		return;
	}

	const auto sample_count = static_cast<int>(mp3dec_ex_read(&mp3_context_, mp3_sample_cache_, sample_cache_capacity));

	if (sample_count == 0)
	{
		return;
	}

	(this->*convert_func_)(sample_count);

	const auto dst_byte_depth = dst_bit_depth_ / 8;
	const auto dst_rest_byte_count = dst_total_byte_count_ - dst_total_byte_offset_;
	cache_byte_offset_ = 0;
	cache_byte_count_ = std::min(sample_count * dst_byte_depth, dst_rest_byte_count);
}

} // namespace ltjs
