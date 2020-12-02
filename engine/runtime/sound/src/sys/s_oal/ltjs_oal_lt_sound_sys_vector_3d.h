#ifndef LTJS_OAL_LT_SOUND_SYS_VECTOR_3D_INCLUDED
#define LTJS_OAL_LT_SOUND_SYS_VECTOR_3D_INCLUDED


namespace ltjs
{


class OalLtSoundSysVector3d
{
public:
	OalLtSoundSysVector3d() noexcept;

	OalLtSoundSysVector3d(
		float x,
		float y,
		float z) noexcept;


	float& operator[](
		int index);

	float operator[](
		int index) const;

	bool operator==(
		const OalLtSoundSysVector3d& rhs) const noexcept;


	bool has_nan() const noexcept;

	OalLtSoundSysVector3d to_rhs() const noexcept;

	const float* get_c_array() const noexcept;


private:
	float items_[3];
}; // OalLtSoundSysVector3d


}; // ltjs


#endif // !LTJS_OAL_LT_SOUND_SYS_VECTOR_3D_INCLUDED
