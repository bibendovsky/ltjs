#ifndef LTJS_AUDIO_UTILS_INCLUDED
#define LTJS_AUDIO_UTILS_INCLUDED


#include "iltsound.h"


namespace ltjs
{


struct AudioUtils
{
	static constexpr auto min_lt_volume = sint32{1};
	static constexpr auto max_lt_volume = sint32{127};
	static constexpr auto max_lt_volume_delta = max_lt_volume - min_lt_volume;

	static constexpr auto min_ds_volume = long{-10000};
	static constexpr auto max_ds_volume = long{0};
	static constexpr auto max_ds_volume_delta = max_ds_volume - min_ds_volume;

	static constexpr auto min_gain = 0.0F;
	static constexpr auto max_gain = 1.0F;
	static constexpr auto max_gain_delta = max_ds_volume - min_ds_volume;


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
	// Converts a DirectSound volume to a gain.
	//
	// Parameters:
	//    - ds_volume - a DirectSound volume.
	//
	// Returns:
	//    - A gain [0..1].
	//
	static float ds_volume_to_gain(
		const long ds_volume);

	//
	// Converts a LithTech volume to a gain.
	//
	// Parameters:
	//    - lt_volume - a LithTech volume.
	//
	// Returns:
	//    A gain [0..1].
	//
	static float lt_to_gain(
		const sint32 lt_volume);
}; // AudioUtils


} // ltjs


#endif // LTJS_AUDIO_UTILS_INCLUDED
