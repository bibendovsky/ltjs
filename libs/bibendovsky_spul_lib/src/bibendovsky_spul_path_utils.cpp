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
// Path utils.
//
// Notes:
//    - All string parameters are expected in UTF-8 encoding.
//    - It's assumed there are two allowed path separators: slash and backslash.
//


#include "bibendovsky_spul_precompiled.h"
#include "bibendovsky_spul_platform.h"
#include "bibendovsky_spul_path_utils.h"


namespace bibendovsky
{
namespace spul
{


char PathUtils::get_native_separator()
{
#ifdef BIBENDOVSKY_SPUL_IS_WIN32
	return '\\';
#else
	return '/';
#endif // BIBENDOVSKY_SPUL_IS_WIN32
}

bool PathUtils::is_separator(
	const char c)
{
	return c == '\\' || c == '/';
}


bool PathUtils::is_ends_with_separator(
	const char* const path_utf8)
{
	if (!path_utf8 || *path_utf8 == '\0')
	{
		return false;
	}

	const auto length = std::string::traits_type::length(path_utf8);

	return is_separator(path_utf8[length - 1]);
}

bool PathUtils::is_ends_with_separator(
	const std::string& path_utf8)
{
	if (path_utf8.empty())
	{
		return false;
	}

	return is_separator(path_utf8.back());
}

void PathUtils::normalize_i(
	char* const path_utf8)
{
	if (!path_utf8)
	{
		return;
	}

	for (auto i = 0; path_utf8[i] != '\0'; ++i)
	{
		if (is_separator(path_utf8[i]))
		{
			path_utf8[i] = get_native_separator();
		}
	}
}

void PathUtils::normalize_i(
	std::string& path_utf8)
{
	for (auto& c : path_utf8)
	{
		if (is_separator(c))
		{
			c = get_native_separator();
		}
	}
}

std::string PathUtils::normalize(
	const char* const path_utf8)
{
	if (!path_utf8 || *path_utf8 == '\0')
	{
		return {};
	}

	auto result = std::string{path_utf8};
	normalize_i(result);
	return result;
}

std::string PathUtils::normalize(
	const std::string& path_utf8)
{
	if (path_utf8.empty())
	{
		return {};
	}

	auto result = path_utf8;
	normalize_i(result);
	return result;
}

void PathUtils::append_i(
	std::string& path_utf8,
	const char* const part_utf8)
{
	if (!part_utf8 || *part_utf8 == '\0')
	{
		return;
	}

	if (!path_utf8.empty() && !is_ends_with_separator(path_utf8))
	{
		path_utf8 += get_native_separator();
	}

	path_utf8 += part_utf8;
}

void PathUtils::append_i(
	std::string& path_utf8,
	const std::string& part_utf8)
{
	if (part_utf8.empty())
	{
		return;
	}

	if (!path_utf8.empty() && !is_ends_with_separator(path_utf8))
	{
		path_utf8 += get_native_separator();
	}

	path_utf8 += part_utf8;
}

std::string PathUtils::append(
	const std::string& path_utf8,
	const char* const part_utf8)
{
	if (!part_utf8 || *part_utf8 == '\0')
	{
		return path_utf8;
	}

	if (path_utf8.empty())
	{
		return part_utf8;
	}

	if (is_ends_with_separator(path_utf8))
	{
		return path_utf8 + part_utf8;
	}
	else
	{
		return path_utf8 + get_native_separator() + part_utf8;
	}
}

std::string PathUtils::append(
	const std::string& path_utf8,
	const std::string& part_utf8)
{
	if (part_utf8.empty())
	{
		return path_utf8;
	}

	if (path_utf8.empty())
	{
		return part_utf8;
	}

	if (is_ends_with_separator(path_utf8))
	{
		return path_utf8 + part_utf8;
	}
	else
	{
		return path_utf8 + get_native_separator() + part_utf8;
	}
}

std::string PathUtils::get_parent_path(
	const std::string& path_utf8)
{
	if (path_utf8.empty())
	{
		return {};
	}


	const auto count = static_cast<int>(path_utf8.size());

	for (auto i = 0; i < count; ++i)
	{
		const auto c = path_utf8[count - i - 1];

		auto is_found = false;

		switch (c)
		{
		case '\\':
		case '/':
			is_found = true;
			break;

#ifdef BIBENDOVSKY_SPUL_IS_WIN32
		case ':':
			return {};
#endif // !BIBENDOVSKY_SPUL_IS_WIN32

		default:
			break;
		}

		if (is_found)
		{
			return path_utf8.substr(0, count - i - 1);
		}
	}

	return {};
}

std::string PathUtils::get_parent_path(
	const char* const path_utf8)
{
	if (!path_utf8 || *path_utf8 == '\0')
	{
		return {};
	}


	const auto count = static_cast<int>(std::string::traits_type::length(path_utf8));

	for (auto i = 0; i < count; ++i)
	{
		const auto c = path_utf8[count - i - 1];

		auto is_found = false;

		switch (c)
		{
		case '\\':
		case '/':
			is_found = true;
			break;

#ifdef BIBENDOVSKY_SPUL_IS_WIN32
		case ':':
			return {};
#endif // !BIBENDOVSKY_SPUL_IS_WIN32

		default:
			break;
		}

		if (is_found)
		{
			return std::string{path_utf8, static_cast<std::size_t>(count - i - 1)};
		}
	}

	return {};
}

const char* PathUtils::get_file_name(
	const char* const path_utf8)
{
	if (!path_utf8 || *path_utf8 == '\0')
	{
		return {};
	}


	const auto count = static_cast<int>(std::string::traits_type::length(path_utf8));

	for (auto i = 0; i < count; ++i)
	{
		const auto c = path_utf8[count - i - 1];

		auto is_found = false;

		switch (c)
		{
		case '\\':
		case '/':

#ifdef BIBENDOVSKY_SPUL_IS_WIN32
		case ':':
#endif // !BIBENDOVSKY_SPUL_IS_WIN32

			is_found = true;
			break;


		default:
			break;
		}

		if (is_found)
		{
			if (i != 0)
			{
				return &path_utf8[count - i];
			}
			else
			{
				return nullptr;
			}
		}
	}

	return path_utf8;
}

std::string PathUtils::get_file_name(
	const std::string& path_utf8)
{
	if (path_utf8.empty())
	{
		return {};
	}


	const auto count = static_cast<int>(path_utf8.size());

	for (auto i = 0; i < count; ++i)
	{
		const auto c = path_utf8[count - i - 1];

		auto is_found = false;

		switch (c)
		{
		case '\\':
		case '/':

#ifdef BIBENDOVSKY_SPUL_IS_WIN32
		case ':':
#endif // !BIBENDOVSKY_SPUL_IS_WIN32

			is_found = true;
			break;

		default:
			break;
		}

		if (is_found)
		{
			return path_utf8.substr(count - i);
		}
	}

	return path_utf8;
}

const char* PathUtils::get_file_extension(
	const char* const path_utf8)
{
	if (!path_utf8 || *path_utf8 == '\0')
	{
		return {};
	}


	const auto count = static_cast<int>(std::string::traits_type::length(path_utf8));

	for (auto i = 0; i < count; ++i)
	{
		const auto c = path_utf8[count - i - 1];

		auto is_found = false;

		switch (c)
		{
		case '\\':
		case '/':

#ifdef BIBENDOVSKY_SPUL_IS_WIN32
		case ':':
#endif // !BIBENDOVSKY_SPUL_IS_WIN32

			return {};


		case '.':
			is_found = true;
			break;

		default:
			break;
		}

		if (is_found)
		{
			return &path_utf8[count - i - 1];
		}
	}

	return {};
}

std::string PathUtils::get_file_extension(
	const std::string& path_utf8)
{
	if (path_utf8.empty())
	{
		return {};
	}


	const auto count = static_cast<int>(path_utf8.size());

	for (auto i = 0; i < count; ++i)
	{
		const auto c = path_utf8[count - i - 1];

		auto is_found = false;

		switch (c)
		{
		case '\\':
		case '/':

#ifdef BIBENDOVSKY_SPUL_IS_WIN32
		case ':':
#endif // !BIBENDOVSKY_SPUL_IS_WIN32

			return {};


		case '.':
			is_found = true;
			break;

		default:
			break;
		}

		if (is_found)
		{
			return path_utf8.substr(count - i - 1);
		}
	}

	return {};
}


} // spul
} // bibendovsky
