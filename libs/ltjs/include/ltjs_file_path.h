/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// File path utility

#ifndef LTJS_FILE_PATH_INCLUDED
#define LTJS_FILE_PATH_INCLUDED

#include <string>
#include <string_view>

namespace ltjs {

class FilePath
{
public:
	static char get_native_separator() noexcept;
	static bool has_any_separator(std::string_view path) noexcept;
	static void normalize_inplace(std::string& path) noexcept;
	static std::string normalize(std::string_view path);
	static std::string append(std::string_view a, std::string_view b);
	static std::string get_file_name(std::string_view path);
	static std::string get_parent_path(std::string_view path);

private:
	static std::string_view get_separators() noexcept;
};

} // namespace ltjs

#endif // LTJS_FILE_PATH_INCLUDED
