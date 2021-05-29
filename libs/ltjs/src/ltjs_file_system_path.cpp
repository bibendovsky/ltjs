#include "ltjs_file_system_path.h"

#include <cassert>

#include <algorithm>
#include <memory>
#include <utility>

#include "ltjs_c_string.h"


namespace ltjs
{
namespace file_system
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

Path::Path() = default;

Path::Path(
	const char* rhs_path,
	Index rhs_size)
{
	assert(rhs_path);
	assert(rhs_size >= 0);

	assign(rhs_path, rhs_size);
}

Path::Path(
	const char* rhs_path)
{
	assert(rhs_path);

	assign(rhs_path);
}

Path::Path(
	const Path& rhs_path)
	:
	data_{rhs_path.data_}
{
}

Path::Path(
	Path&& rhs_path) noexcept
	:
	data_{std::move(rhs_path.data_)}
{
}

Path& Path::operator=(
	const char* rhs_path)
{
	assert(rhs_path);

	return assign(rhs_path);
}

Path& Path::operator=(
	const Path& rhs_path)
{
	if (this != std::addressof(rhs_path))
	{
		data_ = rhs_path.data_;
	}

	return *this;
}

Path& Path::operator=(
	Path&& rhs_path) noexcept
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

Index Path::get_size() const noexcept
{
	return static_cast<Index>(data_.size());
}

Path& Path::assign(
	const char* rhs_path,
	Index rhs_size)
{
	assert(rhs_path);
	assert(rhs_size >= 0);

	data_.assign(rhs_path, rhs_size);
	normalize_separators(0);

	return *this;
}

Path& Path::assign(
	const char* rhs_path)
{
	assert(rhs_path);

	const auto rhs_size = c_string::get_size(rhs_path);
	return assign(rhs_path, rhs_size);
}

Path& Path::assign(
	const Path& rhs_path)
{
	return assign(rhs_path.get_data(), rhs_path.get_size());
}

Path& Path::append(
	const char* rhs_path,
	Index rhs_size)
{
	assert(rhs_path);
	assert(rhs_size >= 0);

	if (rhs_size == 0)
	{
		return *this;
	}

	if (is_empty())
	{
		return assign(rhs_path, rhs_size);
	}

	add_capacity(rhs_size + 1);


	const auto has_left_separator =
		data_.back() == native_separator;

	const auto has_right_separator =
#if _WIN32
		rhs_path[0] == '/' || rhs_path[0] == '\\'
#else
		rhs_path[0] == '/'
#endif // _WIN32
	;

	const auto has_edge_separator =
		data_.back() == native_separator ||
#if _WIN32
		rhs_path[0] == '/' || rhs_path[0] == '\\'
#else
		rhs_path[0] == '/'
#endif // _WIN32
	;

	if (has_left_separator && has_right_separator)
	{
		rhs_path += 1;
		rhs_size -= 1;
	}
	else if (!has_left_separator && !has_right_separator)
	{
		data_.push_back(native_separator);
	}

	const auto old_size = get_size();
	data_.append(rhs_path, rhs_size);
	normalize_separators(old_size);

	return *this;
}

Path& Path::append(
	const char* rhs_path)
{
	assert(rhs_path);

	const auto rhs_size = c_string::get_size(rhs_path);

	return append(rhs_path, rhs_size);
}

Path& Path::append(
	const Path& rhs_path)
{
	return append(rhs_path.get_data(), rhs_path.get_size());
}

void Path::clear()
{
	data_.clear();
}

Path::Path(
	CapacityTag,
	Index capacity)
{
	assert(capacity >= 0);

	set_capacity(capacity);
}

Path::Path(
	SizeTag,
	Index size)
{
	assert(size >= 0);

	resize(size);
}

char* Path::get_data() noexcept
{
	// TODO
	// C++17 Use `data()`.
	return &data_[0];
}

void Path::normalize_separators(
	Index offset)
{
#if _WIN32
	std::replace(data_.begin() + offset, data_.end(), '/', '\\');
#endif // _WIN32
}

void Path::set_capacity(
	Index size)
{
	data_.reserve(static_cast<Data::size_type>(size));
}

void Path::add_capacity(
	Index size)
{
	set_capacity(get_size() + size);
}

void Path::resize(
	Index size)
{
	data_.resize(size);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

Path& operator/=(
	Path& lhs_path,
	const char* rhs_path)
{
	assert(rhs_path);

	return lhs_path.append(rhs_path);
}

Path& operator/=(
	Path& lhs_path,
	const Path& rhs_path)
{
	return lhs_path.append(rhs_path);
}

Path operator/(
	const char* lhs_path,
	const Path& rhs_path)
{
	assert(lhs_path);

	const auto lhs_size = c_string::get_size(lhs_path);
	const auto max_size = lhs_size + rhs_path.get_size() + 1;

	auto result = Path{Path::CapacityTag{}, max_size};
	result.assign(lhs_path, lhs_size);
	result.append(rhs_path);

	return result;
}

Path operator/(
	const Path& lhs,
	const char* rhs)
{
	assert(rhs);

	const auto rhs_size = c_string::get_size(rhs);
	const auto max_size = lhs.get_size() + rhs_size + 1;

	auto result = Path{Path::CapacityTag{}, max_size};
	result.assign(lhs);
	result.append(rhs, rhs_size);

	return result;
}

Path operator/(
	const Path& lhs_path,
	const Path& rhs_path)
{
	const auto max_size = lhs_path.get_size() + rhs_path.get_size() + 1;

	auto result = Path{Path::CapacityTag{}, max_size};
	result.assign(lhs_path);
	result.append(rhs_path);

	return result;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // file_system
} // ltjs
