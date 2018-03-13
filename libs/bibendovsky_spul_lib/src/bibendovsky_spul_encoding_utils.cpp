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
// Encoding utils.
//


#include "bibendovsky_spul_precompiled.h"
#include "bibendovsky_spul_platform.h"
#include "bibendovsky_spul_encoding_utils.h"
#include <limits>
#include <string>

#if defined(BIBENDOVSKY_SPUL_IS_POSIX)
#include <codecvt>
#include <locale>
#endif // BIBENDOVSKY_SPUL_IS_POSIX

#if defined(BIBENDOVSKY_SPUL_IS_WIN32)
#include <Windows.h>
#endif // BIBENDOVSKY_SPUL_IS_WIN32


namespace bibendovsky
{
namespace spul
{


struct EncodingUtils::Detail
{
	static const auto max_length = std::numeric_limits<int>::max();


#if defined(BIBENDOVSKY_SPUL_IS_WIN32)
	static std::wstring utf8_to_wide(
		const std::string& utf8_string,
		bool& is_succeed)
	{
		if (utf8_string.empty())
		{
			is_succeed = true;
			return {};
		}


		is_succeed = false;

		if (utf8_string.size() > max_length)
		{
			return {};
		}

		const auto utf8_size = static_cast<int>(utf8_string.size());

		const auto wide_size = ::MultiByteToWideChar(
			CP_UTF8,
			MB_ERR_INVALID_CHARS,
			utf8_string.c_str(),
			utf8_size,
			nullptr,
			0);

		if (wide_size == 0)
		{
			return {};
		}

		auto wide_string = std::wstring{};
		wide_string.resize(wide_size);

		const auto win32_result = ::MultiByteToWideChar(
			CP_UTF8,
			MB_ERR_INVALID_CHARS,
			utf8_string.c_str(),
			utf8_size,
			&wide_string[0],
			wide_size);

		if (win32_result != wide_size)
		{
			return {};
		}

		is_succeed = true;

		return wide_string;
	}

	static std::string wide_to_utf8(
		const std::wstring& wide_string,
		bool& is_succeed)
	{
		if (wide_string.empty())
		{
			is_succeed = true;
			return {};
		}


		is_succeed = false;

		if (wide_string.size() > max_length)
		{
			return {};
		}

		const auto wide_size = static_cast<int>(wide_string.size());

		const auto utf8_size = ::WideCharToMultiByte(
			CP_UTF8,
			0,
			wide_string.c_str(),
			wide_size,
			nullptr,
			0,
			nullptr,
			nullptr);

		if (utf8_size == 0)
		{
			return {};
		}

		auto utf8_string = std::string{};
		utf8_string.resize(utf8_size);

		const auto win32_result = ::WideCharToMultiByte(
			CP_UTF8,
			0,
			wide_string.c_str(),
			wide_size,
			&utf8_string[0],
			utf8_size,
			nullptr,
			nullptr);

		if (win32_result != utf8_size)
		{
			return {};
		}

		is_succeed = true;

		return utf8_string;
	}
#endif // BIBENDOVSKY_SPUL_IS_WIN32

#if defined(BIBENDOVSKY_SPUL_IS_POSIX)
	using Utf8WideConverter = std::wstring_convert<std::codecvt_utf8<wchar_t>>;


	static std::wstring utf8_to_wide(
		const std::string& utf8_string,
		bool& is_succeed)
	{
		if (utf8_string.empty())
		{
			is_succeed = true;
			return {};
		}

		try
		{
			const auto& result = Utf8WideConverter{}.from_bytes(utf8_string);
			is_succeed = true;
			return result;
		}
		catch (...)
		{
			is_succeed = false;
			return {};
		}
	}

	static std::string wide_to_utf8(
		const std::wstring& wide_string,
		bool& is_succeed)
	{
		if (wide_string.empty())
		{
			is_succeed = true;
			return {};
		}

		try
		{
			const auto& result = Utf8WideConverter{}.to_bytes(wide_string);
			is_succeed = true;
			return result;
		}
		catch (...)
		{
			is_succeed = false;
			return {};
		}
	}
#endif // BIBENDOVSKY_SPUL_IS_POSIX
}; // Detail


std::wstring EncodingUtils::utf8_to_wide(
	const std::string& utf8_string)
{
	bool is_succeed;
	return utf8_to_wide(utf8_string, is_succeed);
}

std::wstring EncodingUtils::utf8_to_wide(
	const std::string& utf8_string,
	bool& is_succeed)
{
	return Detail::utf8_to_wide(utf8_string, is_succeed);
}

std::string EncodingUtils::wide_to_utf8(
	const std::wstring& wide_string)
{
	bool is_succeed;
	return wide_to_utf8(wide_string, is_succeed);
}

std::string EncodingUtils::wide_to_utf8(
	const std::wstring& wide_string,
	bool& is_succeed)
{
	return Detail::wide_to_utf8(wide_string, is_succeed);
}


} // spul
} // bibendovsky
