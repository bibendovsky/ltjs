#ifndef LTJS_AUDIO_UTILS_INCLUDED
#define LTJS_AUDIO_UTILS_INCLUDED


#include "iltsound.h"


namespace ltjs
{


struct AudioUtils
{
	static constexpr auto lt_min_volume = sint32{1};
	static constexpr auto lt_max_volume = sint32{127};
	static constexpr auto lt_max_volume_delta = lt_max_volume - lt_min_volume;

	static constexpr auto ds_min_volume = long{-10000};
	static constexpr auto ds_max_volume = long{0};
	static constexpr auto ds_max_volume_delta = ds_max_volume - ds_min_volume;

	static constexpr auto lt_min_pan = sint32{1};
	static constexpr auto lt_max_pan = sint32{127};
	static constexpr auto lt_max_pan_delta = lt_max_pan - lt_min_pan;
	static constexpr auto lt_pan_center = lt_min_pan + (lt_max_pan_delta / 2);
	static constexpr auto lt_max_pan_side_delta = lt_max_pan - lt_pan_center;

	static constexpr auto ds_min_pan = -10000L;
	static constexpr auto ds_max_pan = 10000L;
	static constexpr auto ds_pan_center = 0L;
	static constexpr auto ds_max_pan_side_delta = ds_max_pan - ds_pan_center;

	static constexpr auto gain_min = 0.0F;
	static constexpr auto gain_max = 1.0F;
	static constexpr auto gain_max_delta = gain_max - gain_min;
	static constexpr auto gain_center = gain_min + (gain_max_delta / 2.0F);


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
	static long clamp_ds_volume(
		const long ds_volume);

	//
	// Converts a LithTech volume to a DirectSound one.
	//
	// Parameters:
	//    - lt_volume - a LithTech volume.
	//
	// Returns:
	//    - A DirectSound volume.
	//
	static long lt_volume_to_ds_volume(
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
	static long clamp_ds_pan(
		const long ds_pan);

	//
	// Converts a LithTech pan to a DirectSound one.
	//
	// Parameters:
	//    - lt_pan - a LithTech pan.
	//
	// Returns:
	//    - A DirectSound pan.
	//
	static long lt_pan_to_ds_pan(
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
	// Initializes internal data.
	//
	static void initialize();


private:
	struct Detail;
}; // AudioUtils


} // ltjs


#endif // LTJS_AUDIO_UTILS_INCLUDED
