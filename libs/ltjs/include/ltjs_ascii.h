#ifndef LTJS_ASCII_INCLUDED
#define LTJS_ASCII_INCLUDED


#include <cassert>

#include <type_traits>

#include "ltjs_index_type.h"


namespace ltjs
{
namespace ascii
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

template<
	typename TChar
>
inline constexpr bool is_decimal_digit(
	TChar ch) noexcept
{
	return ch >= '0' && ch <= '9';
}


template<
	typename TChar
>
inline constexpr bool is_lower(
	TChar ch) noexcept
{
	return ch >= 'a' && ch <= 'z';
}

template<
	typename TChar
>
inline constexpr bool is_upper(
	TChar ch) noexcept
{
	return ch >= 'A' && ch <= 'Z';
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

template<
	typename TChar
>
inline constexpr TChar to_lower(
	TChar ch) noexcept
{
	return
		is_upper(ch) ?
		static_cast<TChar>(static_cast<std::make_unsigned_t<TChar>>('a' + ch - 'A')) :
		ch
	;
}

template<
	typename TChar
>
inline constexpr TChar to_upper(
	TChar ch) noexcept
{
	return
		is_lower(ch) ?
		static_cast<TChar>(static_cast<std::make_unsigned_t<TChar>>('A' + ch - 'a')) :
		ch
	;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

template<
	typename TChar
>
inline void to_lower(
	TChar* string,
	Index string_size) noexcept
{
	for (auto i = Index{}; i < string_size; ++i)
	{
		string[i] = to_lower(string[i]);
	}
}

template<
	typename TChar
>
inline void to_upper(
	TChar* string,
	Index string_size) noexcept
{
	for (auto i = Index{}; i < string_size; ++i)
	{
		string[i] = to_upper(string[i]);
	}
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // ascii
} // ltjs


#endif // !LTJS_ASCII_INCLUDED

