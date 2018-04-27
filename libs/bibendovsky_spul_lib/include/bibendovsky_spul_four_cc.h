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
// FourCC (four-character code)
//


#ifndef BIBENDOVSKY_SPUL_FOUR_CC_INCLUDED
#define BIBENDOVSKY_SPUL_FOUR_CC_INCLUDED


#include <cstdint>
#include <string>


namespace bibendovsky
{
namespace spul
{


//
// FourCC (four-character code).
//
class FourCc
{
public:
	using Value = std::uint32_t;


	//
	// Constructs an empty FourCC.
	//
	constexpr FourCc()
		:
		value_{}
	{
	}

	//
	// Constructs FourCC by a numeric value.
	//
	// Parameters:
	//    - value - a numeric value.
	//
	constexpr explicit FourCc(
		const Value value)
		:
		value_{value}
	{
	}

	//
	// Constructs FourCC by four characters.
	//
	// Parameters:
	//    - char1 - a first character.
	//    - char2 - a second character.
	//    - char3 - a third character.
	//    - char4 - a fourth character.
	//
	constexpr FourCc(
		const char char1,
		const char char2,
		const char char3,
		const char char4)
		:
		value_{
			((static_cast<Value>(static_cast<unsigned char>(char1)) & 0xFF) << 0) |
			((static_cast<Value>(static_cast<unsigned char>(char2)) & 0xFF) << 8) |
			((static_cast<Value>(static_cast<unsigned char>(char3)) & 0xFF) << 16) |
			((static_cast<Value>(static_cast<unsigned char>(char4)) & 0xFF) << 24)}
	{
	}

	//
	// Constructs FourCC by an array of character.
	//
	// Parameters:
	//    - string - an array of characters.
	//
	// Notes:
	//    - Only first four characters will be used if array has more than four items.
	//    - Missing data will be replaced with zero if array has less than four items.
	//
	template<std::size_t TSize>
	constexpr explicit FourCc(
		const char (&string)[TSize])
		:
		FourCc{
			TSize > 0 ? string[0] : '\0',
			TSize > 1 ? string[1] : '\0',
			TSize > 2 ? string[2] : '\0',
			TSize > 3 ? string[3] : '\0'}
	{
	}

	//
	// Casts FourCC to a numeric value.
	//
	// Returns:
	//    - a numeric value.
	//
	constexpr operator Value() const
	{
		return value_;
	}

	//
	// Converts FourCC to a string.
	//
	// Returns:
	//    - A string representation of FourCC.
	//
	std::string to_string() const
	{
		const char string_buffer[4] =
		{
			static_cast<char>((value_ >> 0) & 0xFF),
			static_cast<char>((value_ >> 8) & 0xFF),
			static_cast<char>((value_ >> 16) & 0xFF),
			static_cast<char>((value_ >> 24) & 0xFF),
		};

		return std::string{string_buffer, 4};
	}

	//
	// Compares two FourCC for equality.
	//
	// Returns:
	//    - "true" if two values are equal.
	//    - "false" otherwise.
	//
	static constexpr bool are_equal(
		const FourCc& lhs,
		const FourCc& rhs)
	{
		return lhs.value_ == rhs.value_;
	}


private:
	Value value_;
}; // FourCc


constexpr bool operator==(
	const FourCc& lhs,
	const FourCc& rhs)
{
	return FourCc::are_equal(lhs, rhs);
}

constexpr bool operator!=(
	const FourCc& lhs,
	const FourCc& rhs)
{
	return !(lhs == rhs);
}


} // spul
} // bibendovsky


#endif // !BIBENDOVSKY_SPUL_FOUR_CC_INCLUDED
