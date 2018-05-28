/*

Source Port Utility Library

Copyright (c) 2018 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.

*/


//
// Universally unique identifier (UUID).
//


#include "bibendovsky_spul_precompiled.h"
#include "bibendovsky_spul_endian.h"
#include "bibendovsky_spul_uuid.h"
#include <algorithm>
#include <string>


namespace bibendovsky
{
namespace spul
{


class Uuid::Detail
{
public:
	static int char_digit(
		const char c)
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

	static int chars_to_byte(
		const char* const chars)
	{
		const auto nibble_1 = char_digit(chars[0]);
		const auto nibble_2 = char_digit(chars[1]);

		if (nibble_1 < 0 || nibble_2 < 0)
		{
			return -1;
		}

		return (nibble_1 << 4) | nibble_2;
	}

	static bool parse_part(
		const char* const string,
		const int byte_index,
		const int byte_count,
		Uuid& uuid)
	{
		for (auto i = 0; i < byte_count; ++i)
		{
			const auto byte = chars_to_byte(string + (i * 2));

			if (byte < 0)
			{
				return false;
			}

			uuid.value_[byte_index + i] = static_cast<std::uint8_t>(byte);
		}

		return true;
	}

	static bool parse_without_hyphens(
		const char* const string,
		Uuid& uuid)
	{
		return parse_part(string, 0, 16, uuid);
	}

	static bool parse_with_hyphens(
		const char* const string,
		Uuid& uuid)
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

	static bool parse_with_hyphens_and_braces(
		const char* const string,
		Uuid& uuid)
	{
		if (string[0] != '{' || string[37] != '}')
		{
			return false;
		}

		return parse_with_hyphens(string + 1, uuid);
	}

	static void parse(
		const char* const string,
		Uuid& uuid)
	{
		if (!string)
		{
			return;
		}

		const auto string_length = std::string::traits_type::length(string);

		auto parse_result = false;

		switch (string_length)
		{
		case 32:
			parse_result = parse_without_hyphens(string, uuid);
			break;

		case 36:
			parse_result = parse_with_hyphens(string, uuid);
			break;

		case 38:
			parse_result = parse_with_hyphens_and_braces(string, uuid);
			break;

		default:
			return;
		}

		if (!parse_result)
		{
			uuid.value_.fill(0);
		}
	}


	static char nibble_to_char(
		const int nibble,
		const StringCase string_case)
	{
		if (nibble >= 0 && nibble <= 9)
		{
			return nibble + '0';
		}

		if (nibble >= 0xA || nibble <= 0xF)
		{
			switch (string_case)
			{
			case StringCase::lower:
				return nibble - 0xA + 'a';

			case StringCase::upper:
				return nibble - 0xA + 'A';

			default:
				return '\0';
			}
		}

		return '\0';
	}

	static void byte_to_chars(
		const int byte,
		const StringCase string_case,
		char* const chars)
	{
		const auto nibble_char_1 = nibble_to_char((byte >> 4) & 0xF, string_case);
		const auto nibble_char_2 = nibble_to_char((byte >> 0) & 0xF, string_case);

		chars[0] = nibble_char_1;
		chars[1] = nibble_char_2;
	}

	static void to_string_part(
		const int byte_count,
		const std::uint8_t* const bytes,
		const StringCase string_case,
		char* const string)
	{
		for (auto i = 0; i < byte_count; ++i)
		{
			byte_to_chars(bytes[i], string_case, string + (i * 2));
		}
	}

	static void to_string_without_hyphens(
		const StringCase string_case,
		char* const string,
		const std::uint8_t* const bytes)
	{
		to_string_part(16, bytes, string_case, string);
	}

	static void to_string_with_hyphens(
		const StringCase string_case,
		char* const string,
		const std::uint8_t* const bytes)
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

	static void to_string_with_hyphens_and_braces(
		const StringCase string_case,
		char* const string,
		const std::uint8_t* const bytes)
	{
		string[0] = '{';
		to_string_with_hyphens(string_case, string + 1, bytes);
		string[37] = '}';
	}

	static bool is_endian_type_valid(
		const EndianType endian_type)
	{
		switch (endian_type)
		{
		case EndianType::big:
		case EndianType::little_mixed:
			return true;

		default:
			return false;
		}
	}
}; // Uuid::Detail


Uuid::Uuid()
	:
	value_{}
{
}

Uuid::Uuid(
	const std::uint32_t a,
	const std::uint16_t b,
	const std::uint16_t c,
	const std::uint8_t d,
	const std::uint8_t e,
	const std::uint8_t f,
	const std::uint8_t g,
	const std::uint8_t h,
	const std::uint8_t i,
	const std::uint8_t j,
	const std::uint8_t k)
	:
	value_{
		static_cast<std::uint8_t>((a >> 24) & 0xFF),
		static_cast<std::uint8_t>((a >> 16) & 0xFF),
		static_cast<std::uint8_t>((a >> 8) & 0xFF),
		static_cast<std::uint8_t>((a >> 0) & 0xFF),
		static_cast<std::uint8_t>((b >> 8) & 0xFF),
		static_cast<std::uint8_t>((b >> 0) & 0xFF),
		static_cast<std::uint8_t>((c >> 8) & 0xFF),
		static_cast<std::uint8_t>((c >> 0) & 0xFF),
		d, e, f, g, h, i, j, k}
{
}

Uuid::Uuid(
	const std::uint32_t a,
	const std::uint16_t b,
	const std::uint16_t c,
	const std::uint8_t (&d)[8])
	:
	value_{
		static_cast<std::uint8_t>((a >> 24) & 0xFF),
		static_cast<std::uint8_t>((a >> 16) & 0xFF),
		static_cast<std::uint8_t>((a >> 8) & 0xFF),
		static_cast<std::uint8_t>((a >> 0) & 0xFF),
		static_cast<std::uint8_t>((b >> 8) & 0xFF),
		static_cast<std::uint8_t>((b >> 0) & 0xFF),
		static_cast<std::uint8_t>((c >> 8) & 0xFF),
		static_cast<std::uint8_t>((c >> 0) & 0xFF),
		d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7]}
{
}

Uuid::Uuid(
	const std::uint8_t (&value)[class_size])
	:
	value_{
		value[0], value[1], value[2], value[3], value[4], value[5], value[6], value[7],
		value[8], value[9], value[10], value[11], value[12], value[13], value[14], value[15]}
{
}

Uuid::Uuid(
	const char* const string)
	:
	value_{}
{
	Detail::parse(string, *this);
}

bool Uuid::is_nil() const
{
	return std::all_of(
		value_.cbegin(),
		value_.cend(),
		[](const auto item)
		{
			return item == 0;
		}
	);
}

const Uuid::Array& Uuid::get_array() const
{
	return value_;
}

std::string Uuid::to_string(
	const StringFormat string_format,
	const StringCase string_case) const
{
	auto result = std::string{};

	switch (string_format)
	{
	case StringFormat::without_hyphens:
		result.resize(32);
		Detail::to_string_without_hyphens(string_case, &result[0], value_.data());
		break;

	case StringFormat::with_hyphens:
		result.resize(36);
		Detail::to_string_with_hyphens(string_case, &result[0], value_.data());
		break;

	case StringFormat::with_hyphens_and_braces:
		result.resize(38);
		Detail::to_string_with_hyphens_and_braces(string_case, &result[0], value_.data());
		break;

	default:
		break;
	}

	return result;
}

void Uuid::endian(
	const EndianType endian_type)
{
	switch (endian_type)
	{
	case EndianType::big:
		Endian::swap_i(value_);
		break;

	case EndianType::little_mixed:
	{
		auto bytes_ptr = value_.data();

		Endian::swap_i(*reinterpret_cast<std::uint32_t*>(bytes_ptr + 0));
		Endian::swap_i(*reinterpret_cast<std::uint16_t*>(bytes_ptr + 4));
		Endian::swap_i(*reinterpret_cast<std::uint16_t*>(bytes_ptr + 6));

		break;
	}

	default:
		throw "Invalid endianess type.";
	}
}

bool Uuid::read(
	StreamPtr stream_ptr,
	const EndianType endian_type)
{
	if (!stream_ptr->is_readable() || !Detail::is_endian_type_valid(endian_type))
	{
		return false;
	}

	if (stream_ptr->read(value_.data(), class_size) != class_size)
	{
		return false;
	}

	auto is_swap = false;

	switch (endian_type)
	{
	case EndianType::big:
		if (!Endian::is_big())
		{
			is_swap = true;
		}
		break;

	case EndianType::little_mixed:
		is_swap = true;
		break;

	default:
		break;
	}

	if (is_swap)
	{
		endian(endian_type);
	}

	return true;
}

bool Uuid::are_equal(
	const Uuid& lhs,
	const Uuid& rhs)
{
	return std::equal(lhs.value_.cbegin(), lhs.value_.cend(), rhs.value_.cbegin());
}


bool operator==(
	const Uuid& lhs,
	const Uuid& rhs)
{
	return Uuid::are_equal(lhs, rhs);
}

bool operator!=(
	const Uuid& lhs,
	const Uuid& rhs)
{
	return !(lhs == rhs);
}


} // spul
} // bibendovsky
