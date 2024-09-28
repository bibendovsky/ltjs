#ifndef LTJS_AUDIO_SAMPLE_CONVERTER_INCLUDED
#define LTJS_AUDIO_SAMPLE_CONVERTER_INCLUDED

#include <cstdint>

// ==========================================================================

namespace ltjs {

class AudioSampleConverter
{
public:
	// Unsigned 8-bit to signed 16-bit.
	static std::int16_t u8_to_s16(std::uint8_t src_sample) noexcept;

	// Signed 16-bit to unsigned 8-bit.
	static std::uint8_t s16_to_u8(std::int16_t src_sample) noexcept;

	// Floating point 32-bit to unsigned 8-bit.
	static std::uint8_t f32_to_u8(float src_sample) noexcept;

	// Floating point 32-bit to signed 16-bit.
	static std::int16_t f32_to_s16(float src_sample) noexcept;
};

} // namespace ltjs

#endif // LTJS_AUDIO_SAMPLE_CONVERTER_INCLUDED
