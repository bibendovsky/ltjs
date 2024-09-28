#include "ltjs_audio_sample_converter.h"

// ==========================================================================

static_assert(sizeof(int) >= sizeof(std::int32_t), "Unsupported data model.");

// ==========================================================================

namespace ltjs {

std::int16_t AudioSampleConverter::u8_to_s16(std::uint8_t src_sample) noexcept
{
	return static_cast<std::int16_t>(257 * src_sample - 32768);
}

std::uint8_t AudioSampleConverter::s16_to_u8(std::int16_t src_sample) noexcept
{
	return static_cast<std::uint8_t>((src_sample + 32768) / 257);
}

std::uint8_t AudioSampleConverter::f32_to_u8(float src_sample) noexcept
{
	return static_cast<std::uint8_t>((src_sample * 127.5F) + 127.5F);
}

std::int16_t AudioSampleConverter::f32_to_s16(float src_sample) noexcept
{
	return static_cast<std::int16_t>(src_sample * 32767.5F - 0.5F);
}

} // namespace ltjs
