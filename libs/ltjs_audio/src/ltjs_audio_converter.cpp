#include "ltjs_audio_converter.h"

#include <cassert>

#include <algorithm>

#include "ltjs_audio_sample_converter.h"

// ==========================================================================

static_assert(sizeof(int) >= sizeof(std::int32_t), "Unsupported data model.");

// ==========================================================================

namespace ltjs {

void AudioConverter::close() noexcept
{
	is_open_ = false;
	src_channel_count_ = 0;
	dst_channel_count_ = 0;
	src_bit_depth_ = 0;
	dst_bit_depth_ = 0;
	src_sample_rate_ = 0;
	dst_sample_rate_ = 0;
	src_frame_size_ = 0;
	dst_frame_size_ = 0;
	cache_byte_count_ = 0;
	cache_byte_offset_ = 0;
	sample_rate_counter_ = 0;
	cache_ = nullptr;
	convert_format_func_ = nullptr;
	convert_func_ = nullptr;
}

bool AudioConverter::open(const AudioConverterOpenParam& param) noexcept
{
	close();

	if (!validate(param))
	{
		return false;
	}

	src_channel_count_ = param.src_channel_count;
	dst_channel_count_ = param.dst_channel_count;
	src_bit_depth_ = param.src_bit_depth;
	dst_bit_depth_ = param.dst_bit_depth;
	src_sample_rate_ = param.src_sample_rate;
	dst_sample_rate_ = param.dst_sample_rate;
	src_frame_size_ = param.src_channel_count * (param.src_bit_depth / 8);
	dst_frame_size_ = param.dst_channel_count * (param.dst_bit_depth / 8);
	sample_rate_counter_ = dst_sample_rate_;

	if (!set_funcs(param))
	{
		return false;
	}

	is_open_ = true;
	return is_open();
}

bool AudioConverter::reset() noexcept
{
	if (!is_open())
	{
		assert(false && "Closed.");
		return false;
	}

	cache_byte_count_ = 0;
	cache_byte_offset_ = 0;
	sample_rate_counter_ = dst_sample_rate_;
	cache_ = nullptr;
	return true;
}

int AudioConverter::fill(const void* buffer, int buffer_size) noexcept
{
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

	if (!is_open())
	{
		assert(false && "Closed.");
		return -1;
	}

	if (cache_byte_offset_ != cache_byte_count_)
	{
		return 0;
	}

	if ((buffer_size % src_frame_size_) != 0)
	{
		assert(false && "Unaligned buffer size.");
		return -1;
	}

	cache_ = static_cast<const std::uint8_t*>(buffer);
	cache_byte_count_ = buffer_size;
	cache_byte_offset_ = 0;
	return buffer_size;
}

int AudioConverter::convert(void* buffer, int buffer_size) noexcept
{
	if (!is_open())
	{
		assert(false && "Closed.");
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

	if ((buffer_size % dst_frame_size_) != 0)
	{
		assert(false && "Invalid buffer size.");
		return -1;
	}

	assert(convert_func_ != nullptr);
	return (this->*convert_func_)(buffer, buffer_size);
}

bool AudioConverter::is_open() const noexcept
{
	return is_open_;
}

bool AudioConverter::is_filled() const noexcept
{
	assert(is_open());
	return cache_byte_offset_ != cache_byte_count_;
}

bool AudioConverter::validate(const AudioConverterOpenParam& param) noexcept
{
	switch (param.src_channel_count)
	{
		case 1:
		case 2:
			break;

		default:
			assert(false && "Unsupported source channel count.");
			return false;
	}

	switch (param.src_bit_depth)
	{
		case 8:
		case 16:
			break;

		default:
			assert(false && "Unsupported source bit depth.");
			return false;
	}

	if (param.src_sample_rate < AudioLimits::min_sample_rate || param.src_sample_rate > AudioLimits::max_sample_rate)
	{
		assert(false && "Unsupported source sample rate.");
		return false;
	}

	switch (param.dst_channel_count)
	{
		case 1:
		case 2:
			break;

		default:
			assert(false && "Unsupported source destination count.");
			return false;
	}

	switch (param.dst_bit_depth)
	{
		case 8:
		case 16:
			break;

		default:
			assert(false && "Unsupported destination bit depth.");
			return false;
	}

	if (param.dst_sample_rate < AudioLimits::min_sample_rate || param.dst_sample_rate > AudioLimits::max_sample_rate)
	{
		assert(false && "Unsupported destination sample rate.");
		return false;
	}

	return true;
}

void AudioConverter::convert_format_c1u8_to_c1u8(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept
{
	dst_bytes[0] = src_bytes[0];
}

void AudioConverter::convert_format_c2u8_to_c2u8(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept
{
	dst_bytes[0] = src_bytes[0];
	dst_bytes[1] = src_bytes[1];
}

void AudioConverter::convert_format_c1s16_to_c1s16(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept
{
	const auto src_samples = reinterpret_cast<const std::int16_t*>(src_bytes);
	const auto dst_samples = reinterpret_cast<std::int16_t*>(dst_bytes);
	dst_samples[0] = src_samples[0];
}

void AudioConverter::convert_format_c2s16_to_c2s16(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept
{
	const auto src_samples = reinterpret_cast<const std::int16_t*>(src_bytes);
	const auto dst_samples = reinterpret_cast<std::int16_t*>(dst_bytes);
	dst_samples[0] = src_samples[0];
	dst_samples[1] = src_samples[1];
}

void AudioConverter::convert_format_c1u8_to_c1s16(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept
{
	const auto dst_samples = reinterpret_cast<std::int16_t*>(dst_bytes);
	dst_samples[0] = AudioSampleConverter::u8_to_s16(src_bytes[0]);
}

void AudioConverter::convert_format_c1u8_to_c2u8(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept
{
	const auto sample_u8 = src_bytes[0];
	dst_bytes[0] = sample_u8;
	dst_bytes[1] = sample_u8;
}

void AudioConverter::convert_format_c1u8_to_c2s16(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept
{
	const auto sample_s16 = AudioSampleConverter::u8_to_s16(src_bytes[0]);
	const auto dst_samples = reinterpret_cast<std::int16_t*>(dst_bytes);
	dst_samples[0] = sample_s16;
	dst_samples[1] = sample_s16;
}

void AudioConverter::convert_format_c1s16_to_c1u8(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept
{
	const auto src_samples = reinterpret_cast<const std::int16_t*>(src_bytes);
	dst_bytes[0] = AudioSampleConverter::s16_to_u8(src_samples[0]);
}

void AudioConverter::convert_format_c1s16_to_c2u8(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept
{
	const auto src_samples = reinterpret_cast<const std::int16_t*>(src_bytes);
	const auto sample_u8 = AudioSampleConverter::s16_to_u8(src_samples[0]);
	dst_bytes[0] = sample_u8;
	dst_bytes[1] = sample_u8;
}

void AudioConverter::convert_format_c1s16_to_c2s16(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept
{
	const auto src_samples = reinterpret_cast<const std::int16_t*>(src_bytes);
	const auto dst_samples = reinterpret_cast<std::int16_t*>(dst_bytes);
	const auto sample_s16 = src_samples[0];
	dst_samples[0] = sample_s16;
	dst_samples[1] = sample_s16;
}

void AudioConverter::convert_format_c2u8_to_c1u8(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept
{
	dst_bytes[0] = static_cast<std::uint8_t>((src_bytes[0] + src_bytes[1]) / 2);
}

void AudioConverter::convert_format_c2u8_to_c1s16(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept
{
	const auto sample_s16_0 = AudioSampleConverter::u8_to_s16(src_bytes[0]);
	const auto sample_s16_1 = AudioSampleConverter::u8_to_s16(src_bytes[1]);
	const auto dst_samples = reinterpret_cast<std::int16_t*>(dst_bytes);
	dst_samples[0] = static_cast<std::int16_t>((sample_s16_0 + sample_s16_1) / 2);
}

void AudioConverter::convert_format_c2u8_to_c2s16(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept
{
	const auto dst_samples = reinterpret_cast<std::int16_t*>(dst_bytes);
	dst_samples[0] = AudioSampleConverter::u8_to_s16(src_bytes[0]);
	dst_samples[1] = AudioSampleConverter::u8_to_s16(src_bytes[1]);
}

void AudioConverter::convert_format_c2s16_to_c1u8(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept
{
	const auto src_samples = reinterpret_cast<const std::int16_t*>(src_bytes);
	const auto sample_s16 = static_cast<std::int16_t>((src_samples[0] + src_samples[1]) / 2);
	dst_bytes[0] = AudioSampleConverter::s16_to_u8(sample_s16);
}

void AudioConverter::convert_format_c2s16_to_c1s16(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept
{
	const auto src_samples = reinterpret_cast<const std::int16_t*>(src_bytes);
	const auto dst_samples = reinterpret_cast<std::int16_t*>(dst_bytes);
	dst_samples[0] = static_cast<std::int16_t>((src_samples[0] + src_samples[1]) / 2);
}

void AudioConverter::convert_format_c2s16_to_c2u8(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept
{
	const auto src_samples = reinterpret_cast<const std::int16_t*>(src_bytes);
	dst_bytes[0] = AudioSampleConverter::s16_to_u8(src_samples[0]);
	dst_bytes[1] = AudioSampleConverter::s16_to_u8(src_samples[1]);
}

bool AudioConverter::set_convert_format_func(const AudioConverterOpenParam& param) noexcept
{
	const auto s_c1 = param.src_channel_count == 1;
	const auto s_c2 = param.src_channel_count == 2;
	const auto s_u8 = param.src_bit_depth == 8;
	const auto s_s16 = param.src_bit_depth == 16;

	const auto d_c1 = param.dst_channel_count == 1;
	const auto d_c2 = param.dst_channel_count == 2;
	const auto d_u8 = param.dst_bit_depth == 8;
	const auto d_s16 = param.dst_bit_depth == 16;

	if (false) {}
	else if (s_c1 && s_u8 && d_c1 && d_u8)
	{
		convert_format_func_ = &AudioConverter::convert_format_c1u8_to_c1u8;
	}
	else if (s_c2 && s_u8 && d_c2 && d_u8)
	{
		convert_format_func_ = &AudioConverter::convert_format_c2u8_to_c2u8;
	}
	else if (s_c1 && s_s16 && d_c1 && d_s16)
	{
		convert_format_func_ = &AudioConverter::convert_format_c1s16_to_c1s16;
	}
	else if (s_c2 && s_s16 && d_c2 && d_s16)
	{
		convert_format_func_ = &AudioConverter::convert_format_c2s16_to_c2s16;
	}
	else if (s_c1 && s_u8 && d_c1 && d_s16)
	{
		convert_format_func_ = &AudioConverter::convert_format_c1u8_to_c1s16;
	}
	else if (s_c1 && s_u8 && d_c2 && d_u8)
	{
		convert_format_func_ = &AudioConverter::convert_format_c1u8_to_c2u8;
	}
	else if (s_c1 && s_u8 && d_c2 && d_s16)
	{
		convert_format_func_ = &AudioConverter::convert_format_c1u8_to_c2s16;
	}
	else if (s_c1 && s_s16 && d_c1 && d_u8)
	{
		convert_format_func_ = &AudioConverter::convert_format_c1s16_to_c1u8;
	}
	else if (s_c1 && s_s16 && d_c2 && d_u8)
	{
		convert_format_func_ = &AudioConverter::convert_format_c1s16_to_c2u8;
	}
	else if (s_c1 && s_s16 && d_c2 && d_s16)
	{
		convert_format_func_ = &AudioConverter::convert_format_c1s16_to_c2s16;
	}
	else if (s_c2 && s_u8 && d_c1 && d_u8)
	{
		convert_format_func_ = &AudioConverter::convert_format_c2u8_to_c1u8;
	}
	else if (s_c2 && s_u8 && d_c1 && d_s16)
	{
		convert_format_func_ = &AudioConverter::convert_format_c2u8_to_c1s16;
	}
	else if (s_c2 && s_u8 && d_c2 && d_s16)
	{
		convert_format_func_ = &AudioConverter::convert_format_c2u8_to_c2s16;
	}
	else if (s_c2 && s_s16 && d_c1 && d_u8)
	{
		convert_format_func_ = &AudioConverter::convert_format_c2s16_to_c1u8;
	}
	else if (s_c2 && s_s16 && d_c1 && d_s16)
	{
		convert_format_func_ = &AudioConverter::convert_format_c2s16_to_c1s16;
	}
	else if (s_c2 && s_s16 && d_c2 && d_u8)
	{
		convert_format_func_ = &AudioConverter::convert_format_c2s16_to_c2u8;
	}

	if (convert_format_func_ == nullptr)
	{
		assert(false && "Unknown format.");
		return false;
	}

	return true;
}

bool AudioConverter::set_convert_func(const AudioConverterOpenParam& param) noexcept
{
	const auto is_format_different =
		param.src_channel_count != param.dst_channel_count ||
		param.src_bit_depth != param.dst_bit_depth;

	const auto is_rate_different =
		param.src_sample_rate != param.dst_sample_rate;

	if (false) {}
	else if (!is_format_different && !is_rate_different)
	{
		convert_func_ = &AudioConverter::convert_without_conversion;
	}
	else if (is_format_different && !is_rate_different)
	{
		convert_func_ = &AudioConverter::convert_with_format;
	}
	else
	{
		convert_func_ = &AudioConverter::convert_with_format_and_sample_rate;
	}

	return true;
}

bool AudioConverter::set_funcs(const AudioConverterOpenParam& param) noexcept
{
	if (!set_convert_format_func(param))
	{
		return false;
	}

	if (!set_convert_func(param))
	{
		return false;
	}

	return true;
}

int AudioConverter::convert_without_conversion(void* buffer, int buffer_size) noexcept
{
	const auto dst_bytes = static_cast<std::uint8_t*>(buffer);
	auto dst_offset = 0;

	while (dst_offset < buffer_size)
	{
		if (cache_byte_offset_ == cache_byte_count_)
		{
			break;
		}

		const auto byte_count = std::min(buffer_size - dst_offset, cache_byte_count_ - cache_byte_offset_);
		std::copy_n(&cache_[cache_byte_offset_], byte_count, &dst_bytes[dst_offset]);
		dst_offset += byte_count;
		cache_byte_offset_ += byte_count;
	}

	return dst_offset;
}

int AudioConverter::convert_with_format(void* buffer, int buffer_size) noexcept
{
	const auto dst_bytes = static_cast<std::uint8_t*>(buffer);
	auto dst_offset = 0;

	while (dst_offset < buffer_size)
	{
		if (cache_byte_offset_ == cache_byte_count_)
		{
			break;
		}

		const auto rest_src_frame_count = (cache_byte_count_ - cache_byte_offset_) / src_frame_size_;
		const auto rest_dst_frame_count = (buffer_size - dst_offset) / dst_frame_size_;
		const auto frame_count = std::min(rest_src_frame_count, rest_dst_frame_count);

		for (auto i = 0; i < frame_count; ++i)
		{
			convert_format_func_(&cache_[cache_byte_offset_], &dst_bytes[dst_offset]);
			cache_byte_offset_ += src_frame_size_;
			dst_offset += dst_frame_size_;
		}
	}

	return dst_offset;
}

int AudioConverter::convert_with_format_and_sample_rate(void* buffer, int buffer_size) noexcept
{
	const auto dst_bytes = static_cast<std::uint8_t*>(buffer);
	auto dst_offset = 0;

	while (dst_offset < buffer_size)
	{
		if (sample_rate_counter_ >= dst_sample_rate_)
		{
			if (cache_byte_offset_ == cache_byte_count_)
			{
				break;
			}

			convert_format_func_(&cache_[cache_byte_offset_], frame_cache_);
			cache_byte_offset_ += src_frame_size_;
			sample_rate_counter_ -= dst_sample_rate_;
		}

		sample_rate_counter_ += src_sample_rate_;
		std::copy_n(frame_cache_, dst_frame_size_, &dst_bytes[dst_offset]);
		dst_offset += dst_frame_size_;
	}

	return dst_offset;
}

} // namespace ltjs
