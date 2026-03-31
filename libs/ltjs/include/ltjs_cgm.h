/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Computer Graphics Math

#ifndef LTJS_CGM_INCLUDED
#define LTJS_CGM_INCLUDED

namespace ltjs::cgm {

struct alignas(16) Vec4
{
	float x;
	float y;
	float z;
	float w;
};

struct alignas(16) Mat4
{
	float _11;
	float _12;
	float _13;
	float _14;

	float _21;
	float _22;
	float _23;
	float _24;

	float _31;
	float _32;
	float _33;
	float _34;

	float _41;
	float _42;
	float _43;
	float _44;

	static const Mat4 identity;
};

// =====================================

Vec4 operator-(const Vec4& v) noexcept;
Vec4 operator-(const Vec4& v1, const Vec4& v2) noexcept;
Vec4 operator/(const Vec4& v, float s) noexcept;

// =====================================

Mat4 operator*(const Mat4& m1, const Mat4& m2) noexcept;
Mat4& operator*=(Mat4& m1, const Mat4& m2) noexcept;

// =====================================

float dot(const Vec4& v1, const Vec4& v2) noexcept;
float magnitude(const Vec4& v) noexcept;
Vec4 normalize(const Vec4& v) noexcept;
Vec4 cross(const Vec4& v1, const Vec4& v2) noexcept;
Mat4 transpose(const Mat4& m) noexcept;
Vec4 transform_vec3(const Vec4& v, const Mat4& m) noexcept;

Mat4 ortho_lh(float width, float height, float near_z, float far_z) noexcept;
Mat4 perspective_lh(float width, float height, float near_z, float far_z) noexcept;
Mat4 perspective_fov_lh(float vertical_fov, float aspect_ratio, float near_z, float far_z) noexcept;
Mat4 look_at_lh(const Vec4& eye_position, const Vec4& focus_position, const Vec4& up_direction) noexcept;

} // namespace ltjs::cgm

#endif // LTJS_CGM_INCLUDED
