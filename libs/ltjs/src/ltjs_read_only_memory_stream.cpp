/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Read-only stream with memory backend

#include "ltjs_read_only_memory_stream.h"
#include <cassert>
#include <algorithm>

namespace ltjs {

ReadOnlyMemoryStream::ReadOnlyMemoryStream(const void* data, int data_size) noexcept
{
	impl_open(data, data_size);
}

bool ReadOnlyMemoryStream::open(const void* data, int data_size) noexcept
{
	return impl_open(data, data_size);
}

bool ReadOnlyMemoryStream::is_open() const noexcept
{
	return impl_is_open();
}

void ReadOnlyMemoryStream::close() noexcept
{
	impl_close();
}

int ReadOnlyMemoryStream::read(void* buffer, int count) noexcept
{
	assert(buffer != nullptr && "Null buffer.");
	assert(count >= 0 && "Negative buffer max size.");
	assert(impl_is_open() && "Closed.");
	const int remain_size = data_size_ - data_offset_;
	if (remain_size <= 0)
	{
		return 0;
	}
	const int copy_size = std::min(count, remain_size);
	std::copy_n(data_ + data_offset_, copy_size, static_cast<std::byte*>(buffer));
	data_offset_ += copy_size;
	return copy_size;
}

int ReadOnlyMemoryStream::write([[maybe_unused]] const void* buffer, [[maybe_unused]] int size) noexcept
{
	assert(false && "Not supported.");
	return -1;
}

Stream::Position ReadOnlyMemoryStream::get_position() noexcept
{
	assert(impl_is_open() && "Closed.");
	return data_offset_;
}

Stream::Position ReadOnlyMemoryStream::set_position(Position offset, Origin origin) noexcept
{
	assert(impl_is_open() && "Closed.");
	Position new_position;
	switch (origin)
	{
		case Origin::begin:
			new_position = offset;
			break;
		case Origin::current:
			new_position = data_offset_ + offset;
			break;
		case Origin::end:
			new_position = data_size_ + offset;
			break;
		default:
			assert(false && "Unknown origin.");
			return -1;
	}
	if (new_position < 0)
	{
		assert(false && "Negative new position.");
		return -1;
	}
	if (new_position > data_size_)
	{
		new_position = data_size_;
	}
	data_offset_ = static_cast<int>(new_position);
	assert(data_offset_ >= 0 && data_offset_ <= data_size_ && "New position out of range.");
	return data_offset_;
}

Stream::Position ReadOnlyMemoryStream::get_size() noexcept
{
	assert(impl_is_open() && "Closed.");
	return data_size_;
}

bool ReadOnlyMemoryStream::impl_is_open() const noexcept
{
	return data_ != nullptr;
}

void ReadOnlyMemoryStream::impl_close() noexcept
{
	data_ = nullptr;
	data_size_ = 0;
	data_offset_ = 0;
}

bool ReadOnlyMemoryStream::impl_open(const void* data, int data_size) noexcept
{
	impl_close();
	assert(data != nullptr && "Null data.");
	assert(data_size >= 0 && "Negative data size.");
	data_ = static_cast<const std::byte*>(data);
	data_size_ = data_size;
	data_offset_ = 0;
	return true;
}

} // namespace ltjs
