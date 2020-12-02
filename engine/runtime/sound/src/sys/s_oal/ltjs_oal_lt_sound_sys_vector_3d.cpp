#include "ltjs_oal_lt_sound_sys_vector_3d.h"

#include <cassert>
#include <cmath>


namespace ltjs
{


OalLtSoundSysVector3d::OalLtSoundSysVector3d() noexcept
	:
	items_{}
{
}

OalLtSoundSysVector3d::OalLtSoundSysVector3d(
	float x,
	float y,
	float z) noexcept
	:
	items_{x, y, z}
{
}


float& OalLtSoundSysVector3d::operator[](
	int index)
{
	assert(index >= 0 && index < 3);
	return items_[index];
}

float OalLtSoundSysVector3d::operator[](
	int index) const
{
	assert(index >= 0 && index < 3);
	return items_[index];
}

bool OalLtSoundSysVector3d::operator==(
	const OalLtSoundSysVector3d& rhs) const noexcept
{
	return
		items_[0] == rhs.items_[0] &&
		items_[1] == rhs.items_[1] &&
		items_[2] == rhs.items_[2]
	;
}

bool OalLtSoundSysVector3d::has_nan() const noexcept
{
	return std::isnan(items_[0]) || std::isnan(items_[1]) || std::isnan(items_[2]);
}

OalLtSoundSysVector3d OalLtSoundSysVector3d::to_rhs() const noexcept
{
	return OalLtSoundSysVector3d{items_[0], items_[1], -items_[2]};
}

const float* OalLtSoundSysVector3d::get_c_array() const noexcept
{
	return items_;
}


}; // ltjs
