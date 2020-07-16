#ifndef LTJS_AUDIO_UTILS_INCLUDED
#define LTJS_AUDIO_UTILS_INCLUDED


#include "iltsound.h"
#include "ltjs_audio_decoder.h"


namespace ltjs
{


struct AudioUtils
{
	static sint32 get_lt_min_volume();
	static sint32 get_lt_max_volume();
	static sint32 get_lt_max_volume_delta();

	// Minimum volume in millibels.
	static int get_mb_min_volume();

	// Maximum volume in millibels.
	static int get_mb_max_volume();

	static int get_ds_min_volume();
	static int get_ds_max_volume();
	static int get_ds_max_volume_delta();

	static sint32 get_lt_min_pan();
	static sint32 get_lt_max_pan();
	static sint32 get_lt_max_pan_delta();
	static sint32 get_lt_pan_center();
	static sint32 get_lt_max_pan_side_delta();

	static int get_ds_min_pan();
	static int get_ds_max_pan();
	static int get_ds_pan_center();
	static int get_ds_max_pan_side_delta();

	static float get_ds_default_min_distance();
	static float get_ds_default_max_distance();

	static float get_ds_min_doppler_factor();
	static float get_ds_max_doppler_factor();
	static float get_ds_default_doppler_factor();


	static int get_eax_environment_count();

	static int get_eax_decay_hf_limit_flag();
 
	static int get_eax_min_room();
	static int get_eax_max_room();
	static int get_eax_default_room();

	static int get_eax_min_room_hf();
	static int get_eax_max_room_hf();
	static int get_eax_default_room_hf();

	static float get_eax_min_room_rolloff_factor();
	static float get_eax_max_room_rolloff_factor();
	static float get_eax_default_room_rolloff_factor();

	static float get_eax_min_decay_time();
	static float get_eax_max_decay_time();
	static float get_eax_default_decay_time();

	static float get_eax_min_decay_hf_ratio();
	static float get_eax_max_decay_hf_ratio();
	static float get_eax_default_decay_hf_ratio();

	static int get_eax_min_reflections();
	static int get_eax_max_reflections();
	static int get_eax_default_reflections();

	static float get_eax_min_reflections_delay();
	static float get_eax_max_reflections_delay();
	static float get_eax_default_reflections_delay();

	static int get_eax_min_reverb();
	static int get_eax_max_reverb();
	static int get_eax_default_reverb();

	static float get_eax_min_reverb_delay();
	static float get_eax_max_reverb_delay();
	static float get_eax_default_reverb_delay();

	static int get_eax_min_environment();
	static int get_eax_max_environment();
	static int get_eax_default_environment();

	static float get_eax_min_environment_size();
	static float get_eax_max_environment_size();
	static float get_eax_default_environment_size();

	static float get_eax_min_environment_diffusion();
	static float get_eax_max_environment_diffusion();
	static float get_eax_default_environment_diffusion();

	static float get_eax_min_air_absorption_hf();
	static float get_eax_max_air_absorption_hf();
	static float get_eax_default_airabsorption_hf();


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
	// Converts a volume in millibels to a gain.
	//
	// Parameters:
	//    - mb_value - a volume in millibels [mb_min_volume..mb_max_volume].
	//
	// Returns:
	//    - A gain.
	//
	static float mb_volume_to_gain(
		const int mb_volume);

	//
	// Converts a gain to a volume in millibels.
	//
	// Parameters:
	//    - gain - a gain.
	//
	// Returns:
	//    - A volume in millibels [mb_min_volume..mb_max_volume].
	//
	static int gain_to_mb_volume(
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

	//
	// Initializes internal data.
	//
	static void initialize();


private:
	struct Detail;
}; // AudioUtils


} // ltjs


#endif // LTJS_AUDIO_UTILS_INCLUDED
