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


#ifndef BIBENDOVSKY_SPUL_UUID_INCLUDED
#define BIBENDOVSKY_SPUL_UUID_INCLUDED


#include <cstdint>
#include <array>
#include "bibendovsky_spul_stream.h"


namespace bibendovsky
{
namespace spul
{


//
// Universally unique identifier (UUID).
//
class Uuid
{
public:
	// A size of the class.
	static constexpr auto class_size = 16;


	// An array of bytes which represents the UUID value.
	using Array = std::array<std::uint8_t, class_size>;


	// String representation formats.
	enum class StringFormat
	{
		none,

		// "AAAAAAAABBBBCCCCDDDDEEEEEEEEEEEE"
		without_hyphens,

		// "AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE"
		with_hyphens,

		// "{AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE}"
		with_hyphens_and_braces,
	}; // StringFormat

	// String case format.
	enum class StringCase
	{
		none,
		lower,
		upper,
	}; // StringCase

	// Endianness type.
	enum class EndianType
	{
		none,
		big,
		little_mixed,
	}; // EndianType


	//
	// Construct a nil UUID.
	//
	Uuid();

	//
	// Construct UUID by specified part values.
	//
	// Parameters:
	//    - a - group 1.
	//    - b - group 2.
	//    - c - group 3.
	//    - d, e - group 4.
	//    - f, g, h, i, j, k - group 5.
	//
	Uuid(
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
		const std::uint8_t k);

	//
	// Construct UUID by specified part values.
	//
	// Parameters:
	//    - a - group 1.
	//    - b - group 2.
	//    - c - group 3.
	//    - d - group 4 and group 5 as array.
	//
	Uuid(
		const std::uint32_t a,
		const std::uint16_t b,
		const std::uint16_t c,
		const std::uint8_t (&d)[8]);

	//
	// Construct UUID by a byte array.
	//
	explicit Uuid(
		const std::uint8_t (&value)[class_size]);

	//
	// Construct UUID instance from a string.
	//
	// Prameters:
	//    - string - a string representation of UUID.
	//
	// Notes:
	//     - The supported formats:
	//       a) "AAAAAAAABBBBCCCCDDDDEEEEEEEEEEEE"
	//       b) "AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE"
	//       c) "{AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE}"
	//
	explicit Uuid(
		const char* const string);

	//
	// Tests if UUID is nil ("00000000-0000-0000-0000-000000000000").
	//
	// Returns:
	//    - "true" if UUID is nil.
	//    - "false" otherwise.
	//
	bool is_nil() const;

	//
	// Gets an array of bytes which forms UUID.
	//
	// Returns:
	//    - An array of bytes.
	//
	const Array& get_array() const;

	//
	// Converts UUID to a string.
	//
	// Parameters:
	//    - string_format - a representation format.
	//    - string_case - a case of characters.
	//
	// Returns:
	//    - A formatted string.
	//    - An empty string on error.
	//
	std::string to_string(
		const StringFormat string_format = StringFormat::with_hyphens,
		const StringCase string_case = StringCase::lower) const;

	//
	// Swaps the bytes according to the specified endian type.
	//
	// Parameters:
	//    - endian_type - endian type.
	//
	void endian(
		const EndianType endian_type);

	//
	// Reads UUID from a stream.
	//
	// Parameters:
	//    - stream - a stream to read UUID from.
	//    - endian_type - UUID's endian type.
	//
	bool read(
		StreamPtr stream_ptr,
		const EndianType endian_type);


	//
	// Compares two UUID for equality.
	//
	// Returns:
	//    - "true" if two values are equal.
	//    - "false" otherwise.
	//
	static bool are_equal(
		const Uuid& lhs,
		const Uuid& rhs);


private:
	class Detail;


	Array value_;
}; // Uuid


bool operator==(
	const Uuid& lhs,
	const spul::Uuid& rhs);

bool operator!=(
	const Uuid& lhs,
	const Uuid& rhs);


} // spul
} // bibendovsky


#endif // !BIBENDOVSKY_SPUL_UUID_INCLUDED
