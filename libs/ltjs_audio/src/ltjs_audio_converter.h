#ifndef LTJS_AUDIO_CONVERTER_INCLUDED
#define LTJS_AUDIO_CONVERTER_INCLUDED

#include <cstdint>

#include "ltjs_audio_limits.h"

// ==========================================================================

namespace ltjs {

struct AudioConverterOpenParam
{
	int src_channel_count;
	int src_bit_depth;
	int src_sample_rate;
	int dst_channel_count;
	int dst_bit_depth;
	int dst_sample_rate;
};

// ==========================================================================

class AudioConverter
{
public:
	void close() noexcept;
	bool open(const AudioConverterOpenParam& param) noexcept;
	bool reset() noexcept;
	int fill(const void* buffer, int buffer_size) noexcept;
	int convert(void* buffer, int buffer_size) noexcept;
	bool is_open() const noexcept;
	bool is_filled() const noexcept;

private:
	using FrameCache = std::uint8_t[AudioLimits::max_frame_size];
	using ConvertFormatFunc = void (*)(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes);
	using ConvertFunc = int (AudioConverter::*)(void* buffer, int buffer_size);

private:
	bool is_open_{};
	int src_channel_count_{};
	int dst_channel_count_{};
	int src_bit_depth_{};
	int dst_bit_depth_{};
	int src_sample_rate_{};
	int dst_sample_rate_{};
	int src_frame_size_{};
	int dst_frame_size_{};
	int cache_byte_count_{};
	int cache_byte_offset_{};
	int sample_rate_counter_{};
	const std::uint8_t* cache_{};
	ConvertFormatFunc convert_format_func_{};
	ConvertFunc convert_func_{};
	FrameCache frame_cache_{};

private:
	static bool validate(const AudioConverterOpenParam& param) noexcept;

	static void convert_format_c1u8_to_c1u8(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept;
	static void convert_format_c2u8_to_c2u8(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept;
	static void convert_format_c1s16_to_c1s16(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept;
	static void convert_format_c2s16_to_c2s16(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept;

	static void convert_format_c1u8_to_c1s16(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept;
	static void convert_format_c1u8_to_c2u8(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept;
	static void convert_format_c1u8_to_c2s16(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept;

	static void convert_format_c1s16_to_c1u8(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept;
	static void convert_format_c1s16_to_c2u8(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept;
	static void convert_format_c1s16_to_c2s16(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept;

	static void convert_format_c2u8_to_c1u8(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept;
	static void convert_format_c2u8_to_c1s16(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept;
	static void convert_format_c2u8_to_c2s16(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept;

	static void convert_format_c2s16_to_c1u8(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept;
	static void convert_format_c2s16_to_c1s16(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept;
	static void convert_format_c2s16_to_c2u8(const std::uint8_t* src_bytes, std::uint8_t* dst_bytes) noexcept;

	bool set_convert_format_func(const AudioConverterOpenParam& param) noexcept;
	bool set_convert_func(const AudioConverterOpenParam& param) noexcept;
	bool set_funcs(const AudioConverterOpenParam& param) noexcept;

	int convert_without_conversion(void* buffer, int buffer_size) noexcept;
	int convert_with_format(void* buffer, int buffer_size) noexcept;
	int convert_with_format_and_sample_rate(void* buffer, int buffer_size) noexcept;
};

} // namespace ltjs

#endif // LTJS_AUDIO_CONVERTER_INCLUDED
