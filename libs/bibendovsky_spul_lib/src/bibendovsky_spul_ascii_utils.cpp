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
// ASCII utils.
//


#include "bibendovsky_spul_precompiled.h"
#include "bibendovsky_spul_encoding_utils.h"
#include "bibendovsky_spul_ascii_utils.h"


namespace bibendovsky
{
namespace spul
{


struct AsciiUtils::Detail
{
	using CharCaseInplaceFunc = void (*)(
		char& c);

	using CharCaseFunc = char (*)(
		const char c);


	static constexpr void to_lower_i(
		char& c)
	{
		if (c < 'A' || c > 'Z')
		{
			return;
		}

		c += 'a' - 'A';
	}

	static constexpr void to_upper_i(
		char& c)
	{
		if (c < 'a' || c > 'z')
		{
			return;
		}

		c += 'A' - 'a';
	}


	static void change_case_ascii_i(
		char* const string,
		const CharCaseInplaceFunc char_case_i_func)
	{
		for (auto i = 0; string[i] != '\0'; ++i)
		{
			char_case_i_func(string[i]);
		}
	}

	static void change_case_ascii_i(
		char* const string,
		const int string_length,
		const CharCaseInplaceFunc char_case_i_func)
	{
		for (auto i = 0; i < string_length; ++i)
		{
			char_case_i_func(string[i]);
		}
	}
}; // Detail


void AsciiUtils::to_lower_i(
	char& c)
{
	Detail::to_lower_i(c);
}

void AsciiUtils::to_lower_i(
	char* const string)
{
	if (!string)
	{
		return;
	}

	Detail::change_case_ascii_i(string, Detail::to_lower_i);
}

void AsciiUtils::to_lower_i(
	char* const string,
	const int string_length)
{
	if (!string || string_length <= 0)
	{
		return;
	}

	Detail::change_case_ascii_i(string, string_length, Detail::to_lower_i);
}

void AsciiUtils::to_lower_i(
	std::string& string)
{
	if (string.empty())
	{
		return;
	}

	Detail::change_case_ascii_i(&string[0], static_cast<int>(string.length()), Detail::to_lower_i);
}

char AsciiUtils::to_lower(
	const char c)
{
	auto result = c;
	Detail::to_lower_i(result);
	return result;
}

std::string AsciiUtils::to_lower(
	const char* const string)
{
	if (!string || *string == '\0')
	{
		return {};
	}

	auto result = std::string{string};
	Detail::change_case_ascii_i(&result[0], static_cast<int>(result.length()), Detail::to_lower_i);
	return result;
}

std::string AsciiUtils::to_lower(
	const char* const string,
	const int string_length)
{
	if (!string || string_length <= 0)
	{
		return {};
	}

	auto result = std::string{string, static_cast<std::size_t>(string_length)};
	Detail::change_case_ascii_i(&result[0], string_length, Detail::to_lower_i);
	return result;
}

std::string AsciiUtils::to_lower(
	const std::string& string)
{
	return to_lower(string.c_str(), static_cast<int>(string.length()));
}

void AsciiUtils::to_upper_i(
	char& c)
{
	Detail::to_upper_i(c);
}

void AsciiUtils::to_upper_i(
	char* const string)
{
	if (!string)
	{
		return;
	}

	Detail::change_case_ascii_i(string, Detail::to_upper_i);
}

void AsciiUtils::to_upper_i(
	char* const string,
	const int string_length)
{
	if (!string || string_length <= 0)
	{
		return;
	}

	Detail::change_case_ascii_i(string, string_length, Detail::to_upper_i);
}

void AsciiUtils::to_upper_i(
	std::string& string)
{
	if (string.empty())
	{
		return;
	}

	Detail::change_case_ascii_i(&string[0], static_cast<int>(string.length()), Detail::to_upper_i);
}

char AsciiUtils::to_upper(
	const char c)
{
	auto result = c;
	Detail::to_upper_i(result);
	return result;
}

std::string AsciiUtils::to_upper(
	const char* const string)
{
	if (!string)
	{
		return {};
	}

	auto result = std::string{string};
	Detail::change_case_ascii_i(&result[0], static_cast<int>(result.length()), Detail::to_upper_i);
	return result;
}

std::string AsciiUtils::to_upper(
	const char* const string,
	const int string_length)
{
	if (!string || string_length <= 0)
	{
		return {};
	}

	auto result = std::string{string, static_cast<std::size_t>(string_length)};
	Detail::change_case_ascii_i(&result[0], string_length, Detail::to_upper_i);
	return result;
}

std::string AsciiUtils::to_upper(
	const std::string& string)
{
	return to_upper(string.c_str(), static_cast<int>(string.length()));
}


} // spul
} // bibendovsky
