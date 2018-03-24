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


#include "bibendovsky_spul_precompiled.h"
#include "bibendovsky_spul_platform.h"
#include "bibendovsky_spul_wchar_utils.h"
#include <locale>

#if defined(BIBENDOVSKY_SPUL_IS_WIN32)
#include <Windows.h>
#endif // BIBENDOVSKY_SPUL_IS_WIN32


namespace bibendovsky
{
namespace spul
{


struct WCharUtils::Detail
{
	using StringCaseInplaceFunc = void (*)(
		wchar_t* const string,
		const int string_length,
		bool& is_succeed);


	static const std::locale& get_locale()
	{
		static std::locale locale{""};
		return locale;
	}

	static const std::ctype<wchar_t>& get_wide_facet()
	{
		static const auto& wide_facet = std::use_facet<std::ctype<wchar_t>>(get_locale());
		return wide_facet;
	}

#if defined(BIBENDOVSKY_SPUL_IS_WIN32)
	static void to_lower_i(
		std::wstring& string,
		bool& is_succeed)
	{
		const auto string_length = static_cast<int>(string.length());

		const auto result = ::LCMapStringW(
			LOCALE_USER_DEFAULT,
			LCMAP_LOWERCASE,
			string.c_str(),
			string_length,
			&string[0],
			string_length);

		if (result == string_length)
		{
			is_succeed = true;
		}
		else
		{
			is_succeed = false;
			string.clear();
		}
	}

	static void to_upper_i(
		std::wstring& string,
		bool& is_succeed)
	{
		const auto string_length = static_cast<int>(string.length());

		const auto result = ::LCMapStringW(
			LOCALE_USER_DEFAULT,
			LCMAP_UPPERCASE,
			string.c_str(),
			string_length,
			&string[0],
			string_length);

		if (result == string_length)
		{
			is_succeed = true;
		}
		else
		{
			is_succeed = false;
			string.clear();
		}
	}
#endif // BIBENDOVSKY_SPUL_IS_WIN32

#if defined(BIBENDOVSKY_SPUL_IS_POSIX)
	static void to_lower_i(
		std::wstring& string,
		bool& is_succeed)
	{
		try
		{
			static_cast<void>(get_wide_facet().tolower(&string[0], &string[string.size()]));
			is_succeed = true;
		}
		catch (...)
		{
			is_succeed = false;
			string.clear();
		}
	}

	static void to_upper_i(
		std::wstring& string,
		bool& is_succeed)
	{
		try
		{
			static_cast<void>(get_wide_facet().toupper(&string[0], &string[string.size()]));
			is_succeed = true;
		}
		catch (...)
		{
			is_succeed = false;
			string.clear();
		}
	}
#endif // BIBENDOVSKY_SPUL_IS_POSIX
}; // Detail


void WCharUtils::to_lower_i(
	std::wstring& string)
{
	bool is_succeed;
	to_lower_i(string, is_succeed);
}

void WCharUtils::to_lower_i(
	std::wstring& string,
	bool& is_succeed)
{
	Detail::to_lower_i(string, is_succeed);
}

std::wstring WCharUtils::to_lower(
	const std::wstring& string)
{
	bool is_succeed;
	return to_lower(string, is_succeed);
}

std::wstring WCharUtils::to_lower(
	const std::wstring& string,
	bool& is_succeed)
{
	if (string.empty())
	{
		return {};
	}

	auto result = string;
	Detail::to_lower_i(result, is_succeed);

	if (!is_succeed)
	{
		return {};
	}

	return result;
}

void WCharUtils::to_upper_i(
	std::wstring& string)
{
	bool is_succeed;
	Detail::to_upper_i(string, is_succeed);
}

void WCharUtils::to_upper_i(
	std::wstring& string,
	bool& is_succeed)
{
	Detail::to_upper_i(string, is_succeed);
}

std::wstring WCharUtils::to_upper(
	const std::wstring& string)
{
	bool is_succeed;
	return to_upper(string, is_succeed);
}

std::wstring WCharUtils::to_upper(
	const std::wstring& string,
	bool& is_succeed)
{
	if (string.empty())
	{
		return {};
	}

	auto result = string;
	Detail::to_upper_i(result, is_succeed);

	if (!is_succeed)
	{
		return {};
	}

	return result;
}


} // spul
} // bibendovsky
