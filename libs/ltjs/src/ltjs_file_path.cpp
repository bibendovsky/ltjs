/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// File path utility

#include "ltjs_file_path.h"

namespace {

#ifdef _WIN32
constexpr bool ltjs_win32 = true;
#else
constexpr bool ltjs_win32 = false;
#endif

} // namespace

// =====================================

namespace ltjs {

char FilePath::get_native_separator() noexcept
{
	if constexpr (ltjs_win32)
	{
		return '\\';
	}
	else
	{
		return '/';
	}
}

bool FilePath::has_any_separator(std::string_view path) noexcept
{
	return path.find_first_of(get_separators()) != std::string::npos;
}

void FilePath::normalize_inplace(std::string& path) noexcept
{
	for (char& ch : path)
	{
		if (ch == '/' || ch == '\\')
		{
			ch = get_native_separator();
		}
	}
}

std::string FilePath::normalize(std::string_view path)
{
	std::string new_path{path};
	normalize_inplace(new_path);
	return new_path;
}

std::string FilePath::append(std::string_view a, std::string_view b)
{
	if (a.empty())
	{
		return std::string{b};
	}
	if (b.empty())
	{
		return std::string{a};
	}
	std::string path{};
	path.reserve(a.size() + b.size() + 1);
	path = a;
	const bool has_left_separator = (a.back() == '/' || a.back() == '\\');
	const bool has_right_separator = (b.front() == '/' || b.front() == '\\');
	if (has_left_separator && has_right_separator)
	{
		b.remove_prefix(1);
	}
	else if (!has_left_separator && !has_right_separator)
	{
		path += get_native_separator();
	}
	path += b;
	normalize_inplace(path);
	return path;
}

std::string FilePath::get_file_name(std::string_view path)
{
	const std::size_t separator_pos = path.find_last_of(get_separators());
	if (separator_pos == std::string_view::npos)
	{
		return std::string{};
	}
	return std::string{path.substr(separator_pos + 1)};
}

std::string FilePath::get_parent_path(std::string_view path)
{
	const std::size_t separator_pos = path.find_last_of(get_separators());
	if (separator_pos == std::string_view::npos || separator_pos == 0)
	{
		return std::string{};
	}
	std::string new_path{path.substr(0, separator_pos)};
	normalize_inplace(new_path);
	return new_path;
}

std::string_view FilePath::get_separators() noexcept
{
	constinit static const std::string_view separators{"/\\:"};
	return separators;
}

} // namespace ltjs
