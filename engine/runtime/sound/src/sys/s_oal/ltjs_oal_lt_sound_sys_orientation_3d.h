#ifndef LTJS_OAL_LT_SOUND_SYS_ORIENTATION_3D_INCLUDED
#define LTJS_OAL_LT_SOUND_SYS_ORIENTATION_3D_INCLUDED


#include "ltjs_oal_lt_sound_sys_vector_3d.h"


namespace ltjs
{


class OalLtSoundSysOrientation3d
{
public:
	OalLtSoundSysOrientation3d() noexcept;

	OalLtSoundSysOrientation3d(
		const float at_x,
		const float at_y,
		const float at_z,
		const float up_x,
		const float up_y,
		const float up_z) noexcept;

	OalLtSoundSysOrientation3d(
		const OalLtSoundSysVector3d& at,
		const OalLtSoundSysVector3d& up) noexcept;


	float& operator[](
		const int index);

	float operator[](
		const int index) const;

	bool operator==(
		const OalLtSoundSysOrientation3d& that) const noexcept;


	bool has_nan() const noexcept;

	OalLtSoundSysOrientation3d to_rhs() const noexcept;

	const float* get_c_array() const noexcept;


private:
	float items_[6];
}; // Orientation3d


} // ltjs


#endif // !LTJS_OAL_LT_SOUND_SYS_ORIENTATION_3D_INCLUDED
