#include "ltjs_oal_lt_sound_sys_orientation_3d.h"

#include <cassert>
#include <cmath>


namespace ltjs
{


OalLtSoundSysOrientation3d::OalLtSoundSysOrientation3d() noexcept
	:
	items_{}
{
}

OalLtSoundSysOrientation3d::OalLtSoundSysOrientation3d(
	const float at_x,
	const float at_y,
	const float at_z,
	const float up_x,
	const float up_y,
	const float up_z) noexcept
	:
	items_{at_x, at_y, at_z, up_x, up_y, up_z}
{
}

OalLtSoundSysOrientation3d::OalLtSoundSysOrientation3d(
	const OalLtSoundSysVector3d& at,
	const OalLtSoundSysVector3d& up) noexcept
	:
	items_{at[0], at[1], at[2], up[0], up[1], up[2]}
{
}

float& OalLtSoundSysOrientation3d::operator[](
	const int index)
{
	assert(index >= 0 && index < 6);
	return items_[index];
}

float OalLtSoundSysOrientation3d::operator[](
	const int index) const
{
	assert(index >= 0 && index < 6);
	return items_[index];
}

bool OalLtSoundSysOrientation3d::operator==(
	const OalLtSoundSysOrientation3d& that) const noexcept
{
	return
		items_[0] == that.items_[0] &&
		items_[1] == that.items_[1] &&
		items_[2] == that.items_[2] &&
		items_[3] == that.items_[3] &&
		items_[4] == that.items_[4] &&
		items_[5] == that.items_[5]
	;
}

bool OalLtSoundSysOrientation3d::has_nan() const noexcept
{
	return
		std::isnan(items_[0]) ||
		std::isnan(items_[1]) ||
		std::isnan(items_[2]) ||
		std::isnan(items_[3]) ||
		std::isnan(items_[4]) ||
		std::isnan(items_[5])
	;
}

OalLtSoundSysOrientation3d OalLtSoundSysOrientation3d::to_rhs() const noexcept
{
	return OalLtSoundSysOrientation3d{items_[0], items_[1], -items_[2], items_[3], items_[4], -items_[5]};
}

const float* OalLtSoundSysOrientation3d::get_c_array() const noexcept
{
	return items_;
}


} // ltjs
