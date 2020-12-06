#ifndef LTJS_AUDIO_UTILS_INCLUDED
#define LTJS_AUDIO_UTILS_INCLUDED


#include "iltsound.h"
#include "ltjs_audio_decoder.h"


namespace ltjs
{


struct AudioUtils
{
	static constexpr auto min_gain = 0.0F;
	static constexpr auto max_gain = 1.0F;
	static constexpr auto default_gain = max_gain;

	static constexpr auto min_level_mb = -10'000;

	static constexpr auto min_lt_volume = 0;
	static constexpr auto max_lt_volume = 127;
	static constexpr auto max_lt_volume_delta = max_lt_volume - min_lt_volume;

	static constexpr auto min_lt_pan = 0;
	static constexpr auto max_lt_pan = 128;
	static constexpr auto lt_pan_center = min_lt_pan + ((max_lt_pan - min_lt_pan) / 2);

	static constexpr auto max_left_lt_pan_delta = lt_pan_center - min_lt_pan;
	static constexpr auto max_right_lt_pan_delta = max_lt_pan - lt_pan_center;

	// -------------------------------------------------------------------------
	// Generic stream.

	// (MUSIC_MIN_VOL; profilemgr.h)
	static constexpr auto min_generic_stream_level_mb = -2'500;

	// (MUSIC_MAX_VOL; profilemgr.h)
	static constexpr auto max_generic_stream_level_mb = 5'000;

	static constexpr auto max_generic_stream_level_mb_delta = max_generic_stream_level_mb - min_generic_stream_level_mb;

	// Generic stream.
	// -------------------------------------------------------------------------

	static constexpr auto min_ds_level_mb = min_level_mb;
	static constexpr auto max_ds_level_mb = 0;


	//
	// Converts a LithTech volume to a gain.
	//
	// Parameters:
	//    - lt_volume - a LithTech volume.
	//
	// Returns:
	//    A gain [0..1].
	//
	static float lt_volume_to_gain(
		int lt_volume) noexcept;

	//
	// Converts a LithTech volume to a DirectSound one.
	//
	// Parameters:
	//    - lt_volume - a LithTech volume.
	//
	// Returns:
	//    - A DirectSound volume.
	//
	static int lt_volume_to_ds_level_mb(
		int lt_volume) noexcept;


	//
	// Converts LithTech pan to DirectSound one.
	//
	// Parameters:
	//    - lt_pan - a LithTech pan.
	//
	// Returns:
	//    - A DirectSound pan.
	//
	static int lt_pan_to_ds_pan(
		int lt_pan) noexcept;

	//
	// Converts LithTech pan to gains.
	//
	// Parameters:
	//    - lt_pan - a LithTech pan.
	//    - left_gain - a gain for left channel.
	//    - right_gain - a gain for right channel.
	//
	static void lt_pan_to_gains(
		int lt_pan,
		float& left_gain,
		float& right_gain) noexcept;

	//
	// Converts level (millibel) to gain.
	//
	// Parameters:
	//    - level_mb - level (millibel).
	//
	// Returns:
	//    A gain.
	//
	static float level_mb_to_gain(
		float level_mb) noexcept;

	//
	// Converts generic stream's level (millibel) to gain.
	//
	// Parameters:
	//    - level_mb - level (millibel).
	//
	// Returns:
	//    A gain.
	//
	static float generic_stream_level_mb_to_gain(
		int level_mb) noexcept;

	//
	// Converts generic stream's level (millibel) to DirectSound one.
	//
	// Parameters:
	//    - level_mb - level (millibel).
	//
	// Returns:
	//    Level (millibel).
	//
	static int generic_stream_level_mb_to_ds_level_mb(
		int level_mb) noexcept;


	//
	// Allocates a block of memory.
	//
	// Parameters:
	//    - size - a size of the block.
	//
	// Returns:
	//    - The allocated memory block.
	//
	static void* allocate(
		const std::size_t storage_size);

	//
	// Deallocates a block of memory.
	//
	// Parameters:
	//    - ptr - a pointer to the block to deallocate.
	//
	static void deallocate(
		void* storage_ptr);

	//
	// Decodes MP3 in-memory file (wrapped in WAVE).
	//
	static sint32 decode_mp3(
		AudioDecoder& audio_decoder,
		const void* storage_ptr,
		const uint32 storage_size,
		void*& wav_data_ptr,
		uint32& wav_data_size);


	//
	// Extracts a WAVE size from the memory block.
	//
	// Parameters:
	//    - raw_data - a data to extract the size from.
	//
	// Returns:
	//    - An extracted WAVE size.
	//
	// Notes:
	//    - The size of the provided data should be at least 12 bytes.
	//
	static int extract_wave_size(
		const void* raw_data);


private:
	struct Detail;
}; // AudioUtils


} // ltjs


#endif // LTJS_AUDIO_UTILS_INCLUDED
