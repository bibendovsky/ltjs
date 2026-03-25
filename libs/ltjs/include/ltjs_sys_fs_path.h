/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// System: File system: Path

#ifndef LTJS_SYS_FS_PATH_INCLUDED
#define LTJS_SYS_FS_PATH_INCLUDED

#include <string>
#include <string_view>

namespace ltjs::sys::fs {

inline constexpr char native_separator =
#if _WIN32
		'\\'
#else
		'/'
#endif // _WIN32
	;

// =====================================

class Path
{
public:
	Path() = default;
	Path(std::string_view path);
	Path(const Path& rhs_path);
	Path(Path&& rhs_path) noexcept;
	Path& operator=(std::string_view rhs_path);
	Path& operator=(const Path& rhs_path);
	Path& operator=(Path&& rhs_path) noexcept;

	bool is_empty() const noexcept;
	const char* get_data() const noexcept;
	int get_size() const noexcept;
	std::string_view to_string_view() const noexcept;

	Path& assign(std::string_view rhs_path);
	Path& assign(const Path& rhs_path);

	Path& append(std::string_view rhs_path);
	Path& append(const Path& rhs_path);

	void clear();

private:
	struct CapacityTag {};
	struct SizeTag {};

	using Data = std::string;

	Data data_{};

	friend Path operator/(std::string_view lhs_path, const Path& rhs_path);
	friend Path operator/(const Path& lhs_path, std::string_view rhs_path);
	friend Path operator/(const Path& lhs_path, const Path& rhs_path);

	Path(CapacityTag, int capacity);
	Path(SizeTag, int size);

	char* get_data() noexcept;
	void normalize_separators(int offset);
	void set_capacity(int size);
	void add_capacity(int size);
	void resize(int size);
};

// =====================================

Path& operator/=(Path& lhs, std::string_view rhs_path);
Path& operator/=(Path& lhs, const Path& rhs_path);

Path operator/(std::string_view lhs_path, const Path& rhs_path);
Path operator/(const Path& lhs_path, std::string_view rhs_path);
Path operator/(const Path& lhs_path, const Path& rhs_path);

} // namespace ltjs::sys::fs

#endif // LTJS_SYS_FS_PATH_INCLUDED
