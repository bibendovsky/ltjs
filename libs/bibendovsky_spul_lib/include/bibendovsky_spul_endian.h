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
// Byte order manipulation.
//
// Default: little-endian
//
// Define BIBENDOVSKY_SPUL_LITTLE_ENDIAN or BIBENDOVSKY_SPUL_BIG_ENDIAN to set the endianness.
//


#ifndef BIBENDOVSKY_SPUL_ENDIAN_INCLUDED
#define BIBENDOVSKY_SPUL_ENDIAN_INCLUDED


#include "bibendovsky_spul_platform.h"


#if defined(BIBENDOVSKY_SPUL_LITTLE_ENDIAN) && defined(BIBENDOVSKY_SPUL_BIG_ENDIAN)
#error Multiple endianness defined.
#endif // BIBENDOVSKY_SPUL_LITTLE_ENDIAN && BIBENDOVSKY_SPUL_BIG_ENDIAN


#include <cstdint>
#include <algorithm>
#include <type_traits>


namespace bibendovsky
{
namespace spul
{


class Endian
{
public:
	enum class Id
	{
		none,
		big,
		little,
	}; // Id


#if defined(BIBENDOVSKY_SPUL_BIG_ENDIAN)
	static constexpr auto id = Id::big;
#elif defined(BIBENDOVSKY_SPUL_LITTLE_ENDIAN)
	static constexpr auto id = Id::little;
#else
	static constexpr auto id = Id::none;
#endif // BIBENDOVSKY_SPUL_BIG_ENDIAN


	//
	// Checks if byte order is not set.
	//
	// Returns:
	//    "true" if not detected.
	//    "false" is otherwise.
	//
	static constexpr bool is_none()
	{
		return id == Id::none;
	}

	//
	// Checks if big-endian byte order is set.
	//
	// Returns:
	//    "true" if big-endian byte order.
	//    "false" is otherwise.
	//
	static constexpr bool is_big()
	{
		return id == Id::big;
	}

	//
	// Checks if little-endian byte order is set.
	//
	// Returns:
	//    "true" if little-endian byte order.
	//    "false" is otherwise.
	//
	static constexpr bool is_little()
	{
		return id == Id::little;
	}

	//
	// Swaps bytes on little-endian platform.
	//
	// Parameters:
	//    - value - a value to swap.
	//
	// Returns:
	//    A swapped value on little-endian platform.
	//    Same value as input value on big-endian platform.
	//
	template<typename T>
	static T big(
		T value)
	{
#if defined(BIBENDOVSKY_SPUL_BIG_ENDIAN)
		return value;
#elif defined(BIBENDOVSKY_SPUL_LITTLE_ENDIAN)
		return Swap<T>::swap(value);
#else
		static_assert(false, "Endianess not defined.");
#endif // BIBENDOVSKY_SPUL_BIG_ENDIAN
	}

	//
	// Swaps bytes inplace on little-endian platform.
	//
	// Parameters:
	//    - value - a value to swap.
	//
	template<typename T>
	static void big_i(
		T& value)
	{
#if defined(BIBENDOVSKY_SPUL_BIG_ENDIAN)
		static_cast<void>(value);
#elif defined(BIBENDOVSKY_SPUL_LITTLE_ENDIAN)
		Swap<T>::swap_i(value);
#else
		static_assert(false, "Endianess not defined.");
#endif // BIBENDOVSKY_SPUL_BIG_ENDIAN
	}

	//
	// Swaps bytes on big-endian platform.
	//
	// Parameters:
	//    - value - a value to swap.
	//
	// Returns:
	//    A swapped value on big-endian platform.
	//    Same value as input value on little-endian platform.
	//
	template<typename T>
	static T little(
		T value)
	{
#if defined(BIBENDOVSKY_SPUL_BIG_ENDIAN)
		return Swap<T>::swap(value);
#elif defined(BIBENDOVSKY_SPUL_LITTLE_ENDIAN)
		return value;
#else
		static_assert(false, "Endianess not defined.");
#endif // BIBENDOVSKY_SPUL_BIG_ENDIAN
	}

	//
	// Swaps bytes inplace on big-endian platform.
	//
	// Parameters:
	//    - value - a value to swap.
	//
	template<typename T>
	static void little_i(
		T& value)
	{
#if defined(BIBENDOVSKY_SPUL_BIG_ENDIAN)
		Swap<T>::swap_i(value);
#elif defined(BIBENDOVSKY_SPUL_LITTLE_ENDIAN)
		static_cast<void>(value);
#else
		static_assert(false, "Endianess not defined.");
#endif // BIBENDOVSKY_SPUL_BIG_ENDIAN
	}

	//
	// Swaps bytes inplace.
	//
	// Parameters:
	//    - value - a value to swap.
	//
	template<typename T>
	static void swap_i(
		T& value)
	{
		Swap<T>::swap_i(value);
	}

	//
	// Swaps bytes.
	//
	// Parameters:
	//    - value - a value to swap.
	//
	// Returns:
	//    A swapped value.
	//
	template<typename T>
	static T swap(
		T value)
	{
		return Swap<T>::swap(value);
	}


private:
	Endian() = delete;


	//
	// Generic swap class.
	//
	template<typename T, std::size_t TSize = sizeof(T)>
	struct Swap
	{
		static void swap_i(
			T& value)
		{
			auto& result = reinterpret_cast<std::uint8_t (&)[sizeof(T)]>(value);
			std::reverse(std::begin(result), std::end(result));
		}

		static T swap(
			T& value)
		{
			swap_i(value);
			return value;
		}
	};

	//
	// Specialized swap class for 1-byte values.
	//
	template<typename T>
	struct Swap<T, 1>
	{
		static void swap_i(
			T& value)
		{
			static_cast<void>(value);
		}

		static T swap(
			const T& value)
		{
			return value;
		}
	};

	//
	// Specialized swap class for 2-byte values.
	//
	template<typename T>
	struct Swap<T, 2>
	{
		static void swap_i(
			T& value)
		{
			auto& result = *reinterpret_cast<uint16_t*>(&value);
			result = ((result & 0x00FF) << 8) | ((result & 0xFF00) >> 8);
		}

		static T swap(
			const T& value)
		{
			auto result = *reinterpret_cast<const uint16_t*>(&value);
			result = ((result & 0x00FF) << 8) | ((result & 0xFF00) >> 8);
			return *reinterpret_cast<const T*>(&result);
		}
	};

	//
	// Specialized swap class for 4-byte values.
	//
	template<typename T>
	struct Swap<T, 4>
	{
		static void swap_i(
			T& value)
		{
			auto& result = *reinterpret_cast<uint32_t*>(&value);
			result = ((result & 0x0000FFFF) << 16) | ((result & 0xFFFF0000) >> 16);
			result = ((result & 0x00FF00FF) << 8) | ((result & 0xFF00FF00) >> 8);
		}

		static T swap(
			const T& value)
		{
			auto result = *reinterpret_cast<const uint32_t*>(&value);
			result = ((result & 0x0000FFFF) << 16) | ((result & 0xFFFF0000) >> 16);
			result = ((result & 0x00FF00FF) << 8) | ((result & 0xFF00FF00) >> 8);
			return *reinterpret_cast<const T*>(&result);
		}
	};

	//
	// Specialized swap class for 8-byte values.
	//
	template<typename T>
	struct Swap<T, 8>
	{
		static void swap_i(
			T& value)
		{
			auto& result = *reinterpret_cast<uint64_t*>(&value);
			result = ((result & 0x00000000FFFFFFFF) << 32) | ((result & 0xFFFFFFFF00000000) >> 32);
			result = ((result & 0x0000FFFF0000FFFF) << 16) | ((result & 0xFFFF0000FFFF0000) >> 16);
			result = ((result & 0x00FF00FF00FF00FF) << 8) | ((result & 0xFF00FF00FF00FF00) >> 8);
		}

		static T swap(
			const T& value)
		{
			auto result = *reinterpret_cast<const uint64_t*>(&value);
			result = ((result & 0x00000000FFFFFFFF) << 32) | ((result & 0xFFFFFFFF00000000) >> 32);
			result = ((result & 0x0000FFFF0000FFFF) << 16) | ((result & 0xFFFF0000FFFF0000) >> 16);
			result = ((result & 0x00FF00FF00FF00FF) << 8) | ((result & 0xFF00FF00FF00FF00) >> 8);
			return result;
		}
	};
}; // Endian


} // spul
} // bibendovsky


#endif // !BIBENDOVSKY_SPUL_ENDIAN_INCLUDED
