#ifndef LTJS_AUDIO_UTILS_INCLUDED
#define LTJS_AUDIO_UTILS_INCLUDED


#include "iltsound.h"
#include "ltjs_audio_decoder.h"


namespace ltjs
{


struct AudioUtils
{
	static constexpr auto lt_min_volume = sint32{1};
	static constexpr auto lt_max_volume = sint32{127};
	static constexpr auto lt_max_volume_delta = lt_max_volume - lt_min_volume;

	static constexpr auto ds_min_volume = -10000;
	static constexpr auto ds_max_volume = 0;
	static constexpr auto ds_max_volume_delta = ds_max_volume - ds_min_volume;

	static constexpr auto lt_min_pan = sint32{1};
	static constexpr auto lt_max_pan = sint32{127};
	static constexpr auto lt_max_pan_delta = lt_max_pan - lt_min_pan;
	static constexpr auto lt_pan_center = lt_min_pan + (lt_max_pan_delta / 2);
	static constexpr auto lt_max_pan_side_delta = lt_max_pan - lt_pan_center;

	static constexpr auto ds_min_pan = -10000;
	static constexpr auto ds_max_pan = 10000;
	static constexpr auto ds_pan_center = 0;
	static constexpr auto ds_max_pan_side_delta = ds_max_pan - ds_pan_center;

	static constexpr auto gain_min = 0.0F;
	static constexpr auto gain_max = 1.0F;
	static constexpr auto gain_max_delta = gain_max - gain_min;
	static constexpr auto gain_center = gain_min + (gain_max_delta / 2.0F);

	static constexpr auto ds_default_min_distance = 1.0F;
	static constexpr auto ds_default_max_distance = 1'000'000'000.0F;

	static constexpr auto ds_min_doppler_factor = 0.0F;
	static constexpr auto ds_max_doppler_factor = 10.0F;
	static constexpr auto ds_default_doppler_factor = 1.0F;


	static constexpr auto eax_environment_count = 26;

	static constexpr auto eax_decay_hf_limit_flag = 0B0010'0000;
 
	static constexpr auto eax_min_room = -10000;
	static constexpr auto eax_max_room = 0;
	static constexpr auto eax_default_room = -1000;

	static constexpr auto eax_min_room_hf = -10'000;
	static constexpr auto eax_max_room_hf = 0;
	static constexpr auto eax_default_room_hf = -100;

	static constexpr auto eax_min_room_rolloff_factor = 0.0F;
	static constexpr auto eax_max_room_rolloff_factor = 10.0F;
	static constexpr auto eax_default_room_rolloff_factor = 0.0F;

	static constexpr auto eax_min_decay_time = 0.1F;
	static constexpr auto eax_max_decay_time = 20.0F;
	static constexpr auto eax_default_decay_time = 1.49F;

	static constexpr auto eax_min_decay_hf_ratio = 0.1F;
	static constexpr auto eax_max_decay_hf_ratio = 2.0F;
	static constexpr auto eax_default_decay_hf_ratio = 0.83F;

	static constexpr auto eax_min_reflections = -10'000;
	static constexpr auto eax_max_reflections = 1'000;
	static constexpr auto eax_default_reflections = -2'602;

	static constexpr auto eax_min_reflections_delay = 0.0F;
	static constexpr auto eax_max_reflections_delay = 0.3F;
	static constexpr auto eax_default_reflections_delay = 0.007F;

	static constexpr auto eax_min_reverb = -10'000;
	static constexpr auto eax_max_reverb = 2'000;
	static constexpr auto eax_default_reverb = 200;

	static constexpr auto eax_min_reverb_delay = 0.0F;
	static constexpr auto eax_max_reverb_delay = 0.1F;
	static constexpr auto eax_default_reverb_delay = 0.011F;

	static constexpr auto eax_min_environment = 0;
	static constexpr auto eax_max_environment = eax_environment_count - 1;
	static constexpr auto eax_default_environment = eax_min_environment;

	static constexpr auto eax_min_environment_size = 1.0F;
	static constexpr auto eax_max_environment_size = 100.0F;
	static constexpr auto eax_default_environment_size = 7.5F;

	static constexpr auto eax_min_environment_diffusion = 0.0F;
	static constexpr auto eax_max_environment_diffusion = 1.0F;
	static constexpr auto eax_default_environment_diffusion = 1.0F;

	static constexpr auto eax_min_air_absorption_hf = -100.0F;
	static constexpr auto eax_max_air_absorption_hf = 0.0F;
	static constexpr auto eax_default_airabsorption_hf = -5.0F;


	//
	// Clamps a LithTech volume.
	//
	// Parameters:
	//    - lt_volume - a LithTech volume to clamp.
	//
	// Returns:
	//    - A clamped LithTech volume.
	//
	static sint32 clamp_lt_volume(
		const sint32 lt_volume);

	//
	// Clamps a DirectSound volume.
	//
	// Parameters:
	//    - ds_volume - a DirectSound volume to clamp.
	//
	// Returns:
	//    - A clamped DirectSound volume.
	//
	static int clamp_ds_volume(
		const int ds_volume);

	//
	// Converts a LithTech volume to a DirectSound one.
	//
	// Parameters:
	//    - lt_volume - a LithTech volume.
	//
	// Returns:
	//    - A DirectSound volume.
	//
	static int lt_volume_to_ds_volume(
		const sint32 lt_volume);

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
		const sint32 lt_volume);


	//
	// Clamps a LithTech pan.
	//
	// Parameters:
	//    - lt_pan - a LithTech pan to clamp.
	//
	// Returns:
	//    - A clamped LithTech pan.
	//
	static sint32 clamp_lt_pan(
		const sint32 lt_pan);

	//
	// Clamps a DirectSound pan.
	//
	// Parameters:
	//    - ds_pan - a DirectSound pan to clamp.
	//
	// Returns:
	//    - A clamped DirectSound pan.
	//
	static int clamp_ds_pan(
		const int ds_pan);

	//
	// Converts a LithTech pan to a DirectSound one.
	//
	// Parameters:
	//    - lt_pan - a LithTech pan.
	//
	// Returns:
	//    - A DirectSound pan.
	//
	static int lt_pan_to_ds_pan(
		const sint32 lt_pan);

	//
	// Converts a LithTech pan to a gain.
	//
	// Parameters:
	//    - lt_pan - a LithTech pan.
	//
	// Returns:
	//    A gain [-1..1].
	//
	static float lt_pan_to_gain(
		const sint32 lt_pan);

	//
	// Converts a DirectSound volume to a gain.
	//
	// Parameters:
	//    - ds_volume - a DirectSound volume.
	//
	// Returns:
	//    A gain [0..1].
	//
	static float ds_volume_to_gain(
		const int ds_volume);

	//
	// Converts a value in millibel to a gain.
	//
	// Parameters:
	//    - mb_value - a value in millibel.
	//
	// Returns:
	//    A gain [0.000'01..100'000].
	//
	static float mb_f_to_gain(
		const float mb_value);

	//
	// Converts a value in millibel to a gain.
	//
	// Parameters:
	//    - mb_value - a value in millibel.
	//
	// Returns:
	//    A gain [0.000'01..100'000].
	//
	static float mb_to_gain(
		const int mb_value);

	//
	// Converts a gain to a value in millibel.
	//
	// Parameters:
	//    - gain - a gain.
	//
	// Returns:
	//    A value in millibel [-10'000..10'000].
	//
	static int gain_to_mb(
		const float gain);


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
		const std::size_t size);

	//
	// Deallocates a block of memory.
	//
	// Parameters:
	//    - ptr - a pointer to the block to deallocate.
	//
	static void deallocate(
		void* ptr);

	//
	// Decodes MP3 in-memory file (wrapped in WAVE).
	//
	static sint32 decode_mp3(
		AudioDecoder& audio_decoder,
		const void* src_data_ptr,
		const uint32 src_size,
		void*& dst_wav,
		uint32& dst_wav_size);


	//
	// Initializes internal data.
	//
	static void initialize();


private:
	struct Detail;
}; // AudioUtils


} // ltjs


#endif // LTJS_AUDIO_UTILS_INCLUDED
