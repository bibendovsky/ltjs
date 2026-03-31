/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Computer Graphics Math

#include "ltjs_cgm.h"
#include <cmath>

namespace ltjs::cgm {

Vec4 operator-(const Vec4& v) noexcept
{
	return Vec4{-v.x, -v.y, -v.z, -v.w};
}

Vec4 operator-(const Vec4& v1, const Vec4& v2) noexcept
{
	return Vec4{v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w};
}

Vec4 operator/(const Vec4& v, float s) noexcept
{
	return Vec4{v.x / s, v.y / s, v.z / s, v.w / s};
}

// =====================================

const Mat4 Mat4::identity{
	1.0F, 0.0F, 0.0F, 0.0F,
	0.0F, 1.0F, 0.0F, 0.0F,
	0.0F, 0.0F, 1.0F, 0.0F,
	0.0F, 0.0F, 0.0F, 1.0F};

// =====================================

Mat4 operator*(const Mat4& m1, const Mat4& m2) noexcept
{
	return Mat4{
		m2._11 * m1._11 + m2._21 * m1._12 + m2._31 * m1._13 + m2._41 * m1._14,
		m2._12 * m1._11 + m2._22 * m1._12 + m2._32 * m1._13 + m2._42 * m1._14,
		m2._13 * m1._11 + m2._23 * m1._12 + m2._33 * m1._13 + m2._43 * m1._14,
		m2._14 * m1._11 + m2._24 * m1._12 + m2._34 * m1._13 + m2._44 * m1._14,
		//
		m2._11 * m1._21 + m2._21 * m1._22 + m2._31 * m1._23 + m2._41 * m1._24,
		m2._12 * m1._21 + m2._22 * m1._22 + m2._32 * m1._23 + m2._42 * m1._24,
		m2._13 * m1._21 + m2._23 * m1._22 + m2._33 * m1._23 + m2._43 * m1._24,
		m2._14 * m1._21 + m2._24 * m1._22 + m2._34 * m1._23 + m2._44 * m1._24,
		//
		m2._11 * m1._31 + m2._21 * m1._32 + m2._31 * m1._33 + m2._41 * m1._34,
		m2._12 * m1._31 + m2._22 * m1._32 + m2._32 * m1._33 + m2._42 * m1._34,
		m2._13 * m1._31 + m2._23 * m1._32 + m2._33 * m1._33 + m2._43 * m1._34,
		m2._14 * m1._31 + m2._24 * m1._32 + m2._34 * m1._33 + m2._44 * m1._34,
		//
		m2._11 * m1._41 + m2._21 * m1._42 + m2._31 * m1._43 + m2._41 * m1._44,
		m2._12 * m1._41 + m2._22 * m1._42 + m2._32 * m1._43 + m2._42 * m1._44,
		m2._13 * m1._41 + m2._23 * m1._42 + m2._33 * m1._43 + m2._43 * m1._44,
		m2._14 * m1._41 + m2._24 * m1._42 + m2._34 * m1._43 + m2._44 * m1._44};
}

Mat4& operator*=(Mat4& m1, const Mat4& m2) noexcept
{
	return m1 = m1 * m2;
}

// =====================================

float dot(const Vec4& v1, const Vec4& v2) noexcept
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

float magnitude(const Vec4& v) noexcept
{
	return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

Vec4 normalize(const Vec4& v) noexcept
{
	return v / magnitude(v);
}

/*
 *               | ay * bz - az * by |
 * cross(a, b) = | az * bx - ax * bz |
 *               | ax * by - ay * bx |
 */
Vec4 cross(const Vec4& v1, const Vec4& v2) noexcept
{
	return Vec4{
		v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x,
		0.0F};
}

Mat4 transpose(const Mat4& m) noexcept
{
	return Mat4{
		m._11, m._21, m._31, m._41,
		m._12, m._22, m._32, m._42,
		m._13, m._23, m._33, m._43,
		m._14, m._24, m._34, m._44};
}

/*
 * For any "w":
 * x' = x * m11 + y * m21 + z * m31 + w * m41
 * y' = x * m12 + y * m22 + z * m32 + w * m42
 * z' = x * m13 + y * m23 + z * m33 + w * m43
 * w' = x * m14 + y * m24 + z * m34 + w * m44
 * 
 * For "w=1":
 * x' = x * m11 + y * m21 + z * m31 + m41
 * y' = x * m12 + y * m22 + z * m32 + m42
 * z' = x * m13 + y * m23 + z * m33 + m43
 * w' = 1
 */
Vec4 transform_vec3(const Vec4& v, const Mat4& m) noexcept
{
	return Vec4{
		v.x * m._11 + v.y * m._21 + v.z * m._31 + m._41,
		v.x * m._12 + v.y * m._22 + v.z * m._32 + m._42,
		v.x * m._13 + v.y * m._23 + v.z * m._33 + m._43,
		1.0F};
}

Mat4 ortho_lh(float width, float height, float near_z, float far_z) noexcept
{
	const float range = 1.0F / (far_z - near_z);
	const float m11 = 2.0F / width;
	const float m22 = 2.0F / height;
	const float m33 = range;
	const float m43 = -range * near_z;
	return Mat4{
		 m11, 0.0F, 0.0F, 0.0F,
		0.0F,  m22, 0.0F, 0.0F,
		0.0F, 0.0F,  m33, 0.0F,
		0.0F, 0.0F,  m43, 1.0F};
}

Mat4 perspective_lh(float width, float height, float near_z, float far_z) noexcept
{
	const float range = far_z / (far_z - near_z);
	const float m11 = 2 * near_z / width;
	const float m22 = 2 * near_z / height;
	const float m33 = range;
	const float m43 = -range * near_z;
	return Mat4{
		 m11, 0.0F, 0.0F, 0.0F,
		0.0F,  m22, 0.0F, 0.0F,
		0.0F, 0.0F,  m33, 1.0F,
		0.0F, 0.0F,  m43, 0.0F};
}

Mat4 perspective_fov_lh(float vertical_fov, float aspect_ratio, float near_z, float far_z) noexcept
{
	const float height = 1.0F / std::tan(0.5F * vertical_fov);
	const float width = height / aspect_ratio;
	const float range = far_z / (far_z - near_z);
	const float m11 = width;
	const float m22 = height;
	const float m33 = range;
	const float m43 = -range * near_z;
	return Mat4{
		 m11, 0.0F, 0.0F, 0.0F,
		0.0F,  m22, 0.0F, 0.0F,
		0.0F, 0.0F,  m33, 1.0F,
		0.0F, 0.0F,  m43, 0.0F};
}

/*
 * Eye - E
 * At - A
 * Up - U
 * 
 * Forward: z = (A - E) / ||A - E||
 * Right: x = cross(U, z) / ||cross(U, z)||
 * Up: y = cross(z, x)
 * 
 *     |     xx          yx          zx      0 |
 * M = |     xy          yy          zy      0 |
 *     |     xz          yz          zz      0 |
 *     | dot(-x, E)  dot(-y, E)  dot(-z, E)  1 |
 */
Mat4 look_at_lh(const Vec4& eye_position, const Vec4& focus_position, const Vec4& up_direction) noexcept
{
	const Vec4 forward = normalize(focus_position - eye_position);
	const Vec4 right = normalize(cross(up_direction, forward));
	const Vec4 up = cross(forward, right);
	const float m11 = right.x;
	const float m12 = up.x;
	const float m13 = forward.x;
	const float m21 = right.y;
	const float m22 = up.y;
	const float m23 = forward.y;
	const float m31 = right.z;
	const float m32 = up.z;
	const float m33 = forward.z;
	const float m41 = dot(-right, eye_position);
	const float m42 = dot(-up, eye_position);
	const float m43 = dot(-forward, eye_position);
	return Mat4{
		m11, m12, m13, 0.0F,
		m21, m22, m23, 0.0F,
		m31, m32, m33, 0.0F,
		m41, m42, m43, 1.0F};
}

} // namespace ltjs::cgm
