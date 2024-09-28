#ifndef LTJS_AUDIO_DECODER_LIMITS_INCLUDED
#define LTJS_AUDIO_DECODER_LIMITS_INCLUDED

#include <cstdint>

namespace ltjs {

struct AudioLimits
{
	static constexpr auto min_channels = 1;
	static constexpr auto max_channels = 2;

	static constexpr auto max_bit_depth = 16;
	static constexpr auto max_byte_depth = max_bit_depth / 8;

	static constexpr auto max_frame_size = max_channels * max_byte_depth;

	static constexpr auto min_sample_rate = 1;
	static constexpr auto max_sample_rate = 192'000;

	static constexpr auto max_data_size = INT32_MAX;
	static constexpr auto max_frame_count = max_data_size / max_frame_size;
};

} // namespace ltjs

#endif // LTJS_AUDIO_DECODER_LIMITS_INCLUDED
