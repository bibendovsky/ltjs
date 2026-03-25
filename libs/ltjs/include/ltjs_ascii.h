/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// ASCII utility

#ifndef LTJS_ASCII_INCLUDED
#define LTJS_ASCII_INCLUDED

namespace ltjs {

/*
 * Checks if the ASCII character is a decimal digit.
 */
inline constexpr bool ascii_is_digit(char ch) noexcept
{
	return ch >= '0' && ch <= '9';
}

/*
 * Checks if the ASCII character is lowercase.
 */
inline constexpr bool ascii_is_lower(char ch) noexcept
{
	return ch >= 'a' && ch <= 'z';
}

/*
 * Checks if the ASCII character is uppercase.
 */
inline constexpr bool ascii_is_upper(char ch) noexcept
{
	return ch >= 'A' && ch <= 'Z';
}

// =====================================

/*
 * Converts the ASCII character to lowercase.
 */
inline constexpr char ascii_to_lower(char ch) noexcept
{
	if (ascii_is_upper(ch))
	{
		return static_cast<char>('a' + static_cast<unsigned char>(ch) - 'A');
	}
	else
	{
		return ch;
	}
}

/*
 * Converts the ASCII character to uppercase.
 */
inline constexpr char ascii_to_upper(char ch) noexcept
{
	if (ascii_is_lower(ch))
	{
		return static_cast<char>('A' + static_cast<unsigned char>(ch) - 'a');
	}
	else
	{
		return ch;
	};
}

} // namespace ltjs

#endif // LTJS_ASCII_INCLUDED

