/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// System: File system: Path

#include "ltjs_sys_fs_path.h"
#include <algorithm>
#include <memory>
#include <utility>

namespace ltjs::sys::fs {

Path::Path(std::string_view rhs_path)
{
	assign(rhs_path);
}

Path::Path(const Path& rhs_path)
	:
	data_{rhs_path.data_}
{}

Path::Path(Path&& rhs_path) noexcept
	:
	data_{std::move(rhs_path.data_)}
{}

Path& Path::operator=(std::string_view rhs_path)
{
	return assign(rhs_path);
}

Path& Path::operator=(const Path& rhs_path)
{
	data_ = rhs_path.data_;
	return *this;
}

Path& Path::operator=(Path&& rhs_path) noexcept
{
	data_ = std::move(rhs_path.data_);
	return *this;
}

bool Path::is_empty() const noexcept
{
	return data_.empty();
}

const char* Path::get_data() const noexcept
{
	return data_.data();
}

std::string_view Path::to_string_view() const noexcept
{
	return data_;
}

int Path::get_size() const noexcept
{
	return static_cast<int>(data_.size());
}

Path& Path::assign(std::string_view rhs_path)
{
	data_.assign(rhs_path);
	normalize_separators(0);
	return *this;
}

Path& Path::assign(const Path& rhs_path)
{
	return assign(rhs_path.to_string_view());
}

Path& Path::append(std::string_view rhs_path)
{
	if (rhs_path.empty())
	{
		return *this;
	}
	if (is_empty())
	{
		return assign(rhs_path);
	}
	add_capacity(static_cast<int>(rhs_path.size() + 1));
	const bool has_left_separator = (data_.back() == native_separator);
	const bool has_right_separator = (
#if _WIN32
		rhs_path[0] == '/' || rhs_path[0] == '\\'
#else
		rhs_path[0] == '/'
#endif // _WIN32
	);
	if (has_left_separator && has_right_separator)
	{
		rhs_path.remove_prefix(1);
	}
	else if (!has_left_separator && !has_right_separator)
	{
		data_.push_back(native_separator);
	}
	const int old_size = get_size();
	data_ += rhs_path;
	normalize_separators(old_size);
	return *this;
}

Path& Path::append(const Path& rhs_path)
{
	return append(rhs_path.to_string_view());
}

void Path::clear()
{
	data_.clear();
}

Path::Path(CapacityTag, int capacity)
{
	set_capacity(capacity);
}

Path::Path(SizeTag, int size)
{
	resize(size);
}

char* Path::get_data() noexcept
{
	return data_.data();
}

void Path::normalize_separators(int offset)
{
#if _WIN32
	std::replace(data_.begin() + offset, data_.end(), '/', '\\');
#endif // _WIN32
}

void Path::set_capacity(int size)
{
	data_.reserve(static_cast<std::size_t>(size));
}

void Path::add_capacity(int size)
{
	set_capacity(get_size() + size);
}

void Path::resize(int size)
{
	data_.resize(static_cast<std::size_t>(size));
}

// =====================================

Path& operator/=(Path& lhs_path, std::string_view rhs_path)
{
	return lhs_path.append(rhs_path);
}

Path& operator/=(Path& lhs_path, const Path& rhs_path)
{
	return lhs_path.append(rhs_path);
}

Path operator/(std::string_view lhs_path, const Path& rhs_path)
{
	const int max_size = static_cast<int>(lhs_path.size()) + rhs_path.get_size() + 1;
	Path result{Path::CapacityTag{}, max_size};
	result.assign(lhs_path);
	result.append(rhs_path);
	return result;
}

Path operator/(const Path& lhs, std::string_view rhs)
{
	const int max_size = lhs.get_size() + static_cast<int>(rhs.size()) + 1;
	Path result{Path::CapacityTag{}, max_size};
	result.assign(lhs);
	result.append(rhs);
	return result;
}

Path operator/(const Path& lhs_path, const Path& rhs_path)
{
	const int max_size = lhs_path.get_size() + rhs_path.get_size() + 1;
	Path result{Path::CapacityTag{}, max_size};
	result.assign(lhs_path);
	result.append(rhs_path);
	return result;
}

} // namespace ltjs::sys::fs
