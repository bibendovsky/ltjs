/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Universally unique identifier (UUID)

#ifndef LTJS_UUID_INCLUDED
#define LTJS_UUID_INCLUDED

#include <cstdint>
#include <string>

namespace ltjs {

class Uuid
{
public:
	static constexpr int class_size = 16;

	using Array = std::array<std::uint8_t, 16>;

	enum class StringFormat
	{
		none,
		without_hyphens,         // AAAAAAAABBBBCCCCDDDDEEEEEEEEEEEE
		with_hyphens,            // AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE
		with_hyphens_and_braces  // {AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE}
	};

	enum class StringCase
	{
		none,
		lower,
		upper
	};

	static const Uuid nil;

	Uuid() = default;
	explicit Uuid(const std::uint8_t (&bytes)[16]) noexcept;
	Uuid(
		std::uint8_t v_0,
		std::uint8_t v_1,
		std::uint8_t v_2,
		std::uint8_t v_3,
		std::uint8_t v_4,
		std::uint8_t v_5,
		std::uint8_t v_6,
		std::uint8_t v_7,
		std::uint8_t v_8,
		std::uint8_t v_9,
		std::uint8_t v10,
		std::uint8_t v11,
		std::uint8_t v12,
		std::uint8_t v13,
		std::uint8_t v14,
		std::uint8_t v15) noexcept;

	explicit Uuid(const char* string);

	bool is_nil() const noexcept;
	std::string to_string(
		StringFormat string_format = StringFormat::with_hyphens,
		StringCase string_case = StringCase::lower) const;
	// Creates UUID from the Common Object Model (COM) GUID.
	static bool from_ms_com_guid_octets_n(const void* data, int data_size, Uuid& uuid) noexcept;
	static bool are_equal(const Uuid& a, const Uuid& b) noexcept;

private:
	class Detail;
	using Value = std::uint8_t[16];

	Value value_{};
};

// =====================================

bool operator==(const Uuid& a, const Uuid& b) noexcept;
bool operator!=(const Uuid& a, const Uuid& b) noexcept;

} // namespace ltjs

#endif // LTJS_UUID_INCLUDED
