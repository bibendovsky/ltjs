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
	template<typename TDstString>
	static TDstString utf8_to_utf16_or_wide(
		const std::string& utf8_string,
		bool& is_succeed)
	{
		using TDstChar = typename TDstString::value_type;

		static_assert(sizeof(TDstChar) == 2, "Unsupported destination character type.");

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

		const auto dst_size = ::MultiByteToWideChar(
			CP_UTF8,
			MB_ERR_INVALID_CHARS,
			utf8_string.c_str(),
			utf8_size,
			nullptr,
			0);

		if (dst_size == 0)
		{
			return {};
		}

		auto dst_string = TDstString{};
		dst_string.resize(dst_size);

		const auto win32_result = ::MultiByteToWideChar(
			CP_UTF8,
			MB_ERR_INVALID_CHARS,
			utf8_string.c_str(),
			utf8_size,
			reinterpret_cast<LPWSTR>(&dst_string[0]),
			dst_size);

		if (win32_result != dst_size)
		{
			return {};
		}

		is_succeed = true;

		return dst_string;
	}

	template<typename TSrcString>
	static std::string utf16_or_wide_to_utf8(
		const TSrcString& src_string,
		bool& is_succeed)
	{
		using TSrcChar = typename TSrcString::value_type;

		static_assert(sizeof(TSrcChar) == 2, "Unsupported source character type.");

		if (src_string.empty())
		{
			is_succeed = true;
			return {};
		}


		is_succeed = false;

		if (src_string.size() > max_length)
		{
			return {};
		}

		const auto src_size = static_cast<int>(src_string.size());
		const auto src_ptr = &src_string[0];

		const auto utf8_size = ::WideCharToMultiByte(
			CP_UTF8,
			0,
			reinterpret_cast<LPCWSTR>(src_ptr),
			src_size,
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
			reinterpret_cast<LPCWSTR>(src_ptr),
			src_size,
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


	static std::wstring utf8_to_wide(
		const std::string& utf8_string,
		bool& is_succeed)
	{
		return utf8_to_utf16_or_wide<std::wstring>(utf8_string, is_succeed);
	}

	static std::string wide_to_utf8(
		const std::wstring& wide_string,
		bool& is_succeed)
	{
		return utf16_or_wide_to_utf8(wide_string, is_succeed);
	}

	static std::u16string utf8_to_utf16(
		const std::string& utf8_string,
		bool& is_succeed)
	{
		return utf8_to_utf16_or_wide<std::u16string>(utf8_string, is_succeed);
	}

	static std::string utf16_to_utf8(
		const std::u16string& utf16_string,
		bool& is_succeed)
	{
		return utf16_or_wide_to_utf8(utf16_string, is_succeed);
	}
#endif // BIBENDOVSKY_SPUL_IS_WIN32

#if defined(BIBENDOVSKY_SPUL_IS_POSIX)
	template<typename TChar>
	using Utf8Utf16ToWideConverter = std::wstring_convert<std::codecvt_utf8<TChar>, TChar>;


	template<typename TDstString>
	static TDstString utf8_to_utf16_or_wide(
		const std::string& utf8_string,
		bool& is_succeed)
	{
		using TDstChar = typename TDstString::value_type;

		static_assert(
			sizeof(TDstChar) == sizeof(wchar_t) || sizeof(TDstChar) == sizeof(char16_t),
			"Unsupported destination character type.");

		is_succeed = true;

		if (utf8_string.empty())
		{
			return {};
		}

		try
		{
			return Utf8Utf16ToWideConverter<TDstChar>{}.from_bytes(utf8_string);
		}
		catch (...)
		{
			is_succeed = false;
			return {};
		}
	}

	template<typename TSrcString>
	static std::string utf16_or_wide_to_utf8(
		const TSrcString& utf16_string,
		bool& is_succeed)
	{
		using TSrcChar = typename TSrcString::value_type;

		static_assert(
			sizeof(TSrcChar) == sizeof(wchar_t) || sizeof(TSrcChar) == sizeof(char16_t),
			"Unsupported source character type.");

		is_succeed = true;

		if (utf16_string.empty())
		{
			return {};
		}

		try
		{
			return Utf8Utf16ToWideConverter<TSrcChar>{}.to_bytes(utf16_string);
		}
		catch (...)
		{
			is_succeed = false;
			return {};
		}
	}


	static std::wstring utf8_to_wide(
		const std::string& utf8_string,
		bool& is_succeed)
	{
		return utf8_to_utf16_or_wide<std::wstring>(utf8_string, is_succeed);
	}

	static std::string wide_to_utf8(
		const std::wstring& utf16_string,
		bool& is_succeed)
	{
		return utf16_or_wide_to_utf8(utf16_string, is_succeed);
	}

	static std::u16string utf8_to_utf16(
		const std::string& utf8_string,
		bool& is_succeed)
	{
		return utf8_to_utf16_or_wide<std::u16string>(utf8_string, is_succeed);
	}

	static std::string utf16_to_utf8(
		const std::u16string& utf16_string,
		bool& is_succeed)
	{
		return utf16_or_wide_to_utf8(utf16_string, is_succeed);
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

std::u16string EncodingUtils::utf8_to_utf16(
	const std::string& string_utf8)
{
	bool is_succeed;
	return Detail::utf8_to_utf16(string_utf8, is_succeed);
}

std::u16string EncodingUtils::utf8_to_utf16(
	const std::string& string_utf8,
	bool& is_succeed)
{
	return Detail::utf8_to_utf16(string_utf8, is_succeed);
}

std::string EncodingUtils::utf16_to_utf8(
	const std::u16string& string_utf16)
{
	bool is_succeed;
	return Detail::utf16_to_utf8(string_utf16, is_succeed);
}

std::string EncodingUtils::utf16_to_utf8(
	const std::u16string& string_utf16,
	bool& is_succeed)
{
	return Detail::utf16_to_utf8(string_utf16, is_succeed);
}


} // spul
} // bibendovsky
