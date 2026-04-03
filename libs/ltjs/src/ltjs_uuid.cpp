/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Universally unique identifier (UUID)

/*
 * References:
 *   - Universally Unique IDentifiers (UUIDs)
 *     https://www.rfc-editor.org/rfc/rfc9562
 *   - GUID structure (guiddef.h)
 *     https://learn.microsoft.com/en-us/windows/win32/api/guiddef/ns-guiddef-guid
 */

#include "ltjs_uuid.h"
#include <cassert>

namespace ltjs {

class Uuid::Detail
{
public:
	static int char_digit(char c)
	{
		if (c >= '0' && c <= '9')
		{
			return c - '0';
		}
		if (c >= 'a' && c <= 'f')
		{
			return 0x0A + c - 'a';
		}
		if (c >= 'A' && c <= 'F')
		{
			return 0x0A + c - 'A';
		}
		return -1;
	}

	static int chars_to_byte(const char* chars)
	{
		const int nibble_1 = char_digit(chars[0]);
		const int nibble_2 = char_digit(chars[1]);
		if (nibble_1 < 0 || nibble_2 < 0)
		{
			return -1;
		}
		return (nibble_1 * 16) | nibble_2;
	}

	static bool parse_part(const char* string, int byte_index, int byte_count, Uuid& uuid)
	{
		for (int i = 0; i < byte_count; ++i)
		{
			const int byte = chars_to_byte(string + i * 2);
			if (byte < 0)
			{
				return false;
			}
			uuid.value_[byte_index + i] = static_cast<std::uint8_t>(byte);
		}
		return true;
	}

	static bool parse_without_hyphens(const char* string, Uuid& uuid)
	{
		return parse_part(string, 0, 16, uuid);
	}

	static bool parse_with_hyphens(const char* string, Uuid& uuid)
	{
		if (string[8] != '-' || string[13] != '-' || string[18] != '-' || string[23] != '-')
		{
			return false;
		}
		if (!parse_part(string + 0, 0, 4, uuid))
		{
			return false;
		}
		if (!parse_part(string + 9, 4, 2, uuid))
		{
			return false;
		}
		if (!parse_part(string + 14, 6, 2, uuid))
		{
			return false;
		}
		if (!parse_part(string + 19, 8, 2, uuid))
		{
			return false;
		}
		if (!parse_part(string + 24, 10, 6, uuid))
		{
			return false;
		}
		return true;
	}

	static bool parse_with_hyphens_and_braces(const char* string, Uuid& uuid)
	{
		if (string[0] != '{' || string[37] != '}')
		{
			return false;
		}
		return parse_with_hyphens(string + 1, uuid);
	}

	static void parse(const char* string, Uuid& uuid)
	{
		if (string == nullptr)
		{
			return;
		}
		const std::size_t string_length = std::string::traits_type::length(string);
		bool is_parsed = false;
		switch (string_length)
		{
			case 32:
				is_parsed = parse_without_hyphens(string, uuid);
				break;
			case 36:
				is_parsed = parse_with_hyphens(string, uuid);
				break;
			case 38:
				is_parsed = parse_with_hyphens_and_braces(string, uuid);
				break;
			default:
				return;
		}
		if (!is_parsed)
		{
			for (int i = 0; i < 16; ++i)
			{
				uuid.value_[i] = 0;
			}
		}
	}

	static char nibble_to_char(int nibble, StringCase string_case)
	{
		if (nibble >= 0 && nibble <= 9)
		{
			return static_cast<char>(nibble + '0');
		}
		if (nibble >= 0xA || nibble <= 0xF)
		{
			switch (string_case)
			{
				case StringCase::lower: return static_cast<char>(nibble - 0xA + 'a');
				case StringCase::upper: return static_cast<char>(nibble - 0xA + 'A');
				default:                return '\0';
			}
		}
		return '\0';
	}

	static void byte_to_chars(int byte, StringCase string_case, char* chars)
	{
		const char nibble_char_1 = nibble_to_char((byte / 16) & 0xF, string_case);
		const char nibble_char_2 = nibble_to_char( byte       & 0xF, string_case);
		chars[0] = nibble_char_1;
		chars[1] = nibble_char_2;
	}

	static void to_string_part(int byte_count, const std::uint8_t* bytes, StringCase string_case, char* string)
	{
		for (int i = 0; i < byte_count; ++i)
		{
			byte_to_chars(bytes[i], string_case, string + i * 2);
		}
	}

	static void to_string_without_hyphens(StringCase string_case, char* string, const std::uint8_t* bytes)
	{
		to_string_part(16, bytes, string_case, string);
	}

	static void to_string_with_hyphens(StringCase string_case, char* string, const std::uint8_t* bytes)
	{
		to_string_part(4, bytes + 0, string_case, string + 0);
		string[8] = '-';
		to_string_part(2, bytes + 4, string_case, string + 9);
		string[13] = '-';
		to_string_part(2, bytes + 6, string_case, string + 14);
		string[18] = '-';
		to_string_part(2, bytes + 8, string_case, string + 19);
		string[23] = '-';
		to_string_part(6, bytes + 10, string_case, string + 24);
	}

	static void to_string_with_hyphens_and_braces(StringCase string_case, char* string, const std::uint8_t* bytes)
	{
		string[0] = '{';
		to_string_with_hyphens(string_case, string + 1, bytes);
		string[37] = '}';
	}
};

// -------------------------------------

const Uuid Uuid::nil{};

// -------------------------------------

Uuid::Uuid(const std::uint8_t (&bytes)[16]) noexcept
	:
	value_{
		bytes[ 0], bytes[ 1], bytes[ 2], bytes[ 3], bytes[ 4], bytes[ 5], bytes[ 6], bytes[ 7],
		bytes[ 8], bytes[ 9], bytes[10], bytes[11], bytes[12], bytes[13], bytes[14], bytes[15]}
{}

Uuid::Uuid(
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
	std::uint8_t v15) noexcept
	:
	value_{
		v_0, v_1, v_2, v_3, v_4, v_5, v_6, v_7,
		v_8, v_9, v10, v11, v12, v13, v14, v15}
{}

Uuid::Uuid(const char* string)
	:
	value_{}
{
	Detail::parse(string, *this);
}

bool Uuid::is_nil() const noexcept
{
	return *this == nil;
}

std::string Uuid::to_string(StringFormat string_format, StringCase string_case) const
{
	std::string result{};
	switch (string_format)
	{
		case StringFormat::without_hyphens:
			result.resize(32);
			Detail::to_string_without_hyphens(string_case, result.data(), value_);
			break;
		case StringFormat::with_hyphens:
			result.resize(36);
			Detail::to_string_with_hyphens(string_case, result.data(), value_);
			break;
		case StringFormat::with_hyphens_and_braces:
			result.resize(38);
			Detail::to_string_with_hyphens_and_braces(string_case, result.data(), value_);
			break;
		default:
			break;
	}
	return result;
}

bool Uuid::from_ms_com_guid_octets_n(const void* data, int data_size, Uuid& uuid) noexcept
{
	if (data == nullptr)
	{
		assert(false && "Null data.");
		return false;
	}
	if (data_size < 16)
	{
		assert(false && "Not enough data.");
		return false;
	}
	const std::uint8_t* const octets = static_cast<const std::uint8_t*>(data);
	// group 1 (Data1)
	uuid.value_[ 0] = octets[ 3];
	uuid.value_[ 1] = octets[ 2];
	uuid.value_[ 2] = octets[ 1];
	uuid.value_[ 3] = octets[ 0];
	// group 2 (Data2)
	uuid.value_[ 4] = octets[ 5];
	uuid.value_[ 5] = octets[ 4];
	// group 3 (Data3)
	uuid.value_[ 6] = octets[ 7];
	uuid.value_[ 7] = octets[ 6];
	//
	for (int i = 8; i < 16; ++i)
	{
		uuid.value_[i] = octets[i];
	}
	return true;
}

bool Uuid::are_equal(const Uuid& a, const Uuid& b) noexcept
{
	for (int i = 0; i < 16; ++i)
	{
		if (a.value_[i] != b.value_[i])
		{
			return false;
		}
	}
	return true;
}

// =====================================

bool operator==(const Uuid& a, const Uuid& b) noexcept
{
	return Uuid::are_equal(a, b);
}

bool operator!=(const Uuid& a, const Uuid& b) noexcept
{
	return !(a == b);
}

} // namespace ltjs
