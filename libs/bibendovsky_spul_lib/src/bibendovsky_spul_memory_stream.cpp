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
// A stream with memory storage.
//


#include "bibendovsky_spul_precompiled.h"
#include "bibendovsky_spul_memory_stream.h"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <utility>


namespace bibendovsky
{
namespace spul
{


MemoryStream::MemoryStream()
	:
	Stream{},
	flags_{},
	internal_buffer_{},
	buffer_ptr_{},
	current_position_{},
	end_position_{},
	max_size_{}
{
}

MemoryStream::MemoryStream(
	const int initial_buffer_size,
	const OpenMode open_mode)
	:
	MemoryStream{}
{
	static_cast<void>(open_internal(true, nullptr, initial_buffer_size, open_mode | OpenMode::write));
}

MemoryStream::MemoryStream(
	void* const buffer,
	const int buffer_size)
	:
	MemoryStream{}
{
	static_cast<void>(open_internal(false, buffer, buffer_size, OpenMode::read_write));
}

MemoryStream::MemoryStream(
	const void* const buffer,
	const int buffer_size)
	:
	MemoryStream{}
{
	static_cast<void>(open_internal(false, const_cast<void*>(buffer), buffer_size, OpenMode::read));
}

MemoryStream::MemoryStream(
	void* const buffer,
	const int buffer_size,
	const OpenMode open_mode)
	:
	MemoryStream{}
{
	static_cast<void>(open_internal(false, buffer, buffer_size, open_mode));
}

MemoryStream::MemoryStream(
	const void* const buffer,
	const int buffer_size,
	const OpenMode open_mode)
	:
	MemoryStream{}
{
	static_cast<void>(open_internal(false, const_cast<void*>(buffer), buffer_size, open_mode));
}

MemoryStream::MemoryStream(
	MemoryStream&& that) noexcept
	:
	Stream{std::move(that)},
	flags_{std::move(that.flags_)},
	internal_buffer_{std::move(that.internal_buffer_)},
	buffer_ptr_{std::move(that.buffer_ptr_)},
	current_position_{std::move(that.current_position_)},
	end_position_{std::move(that.end_position_)},
	max_size_{std::move(that.max_size_)}
{
	that.flags_.clear();
}

MemoryStream& MemoryStream::operator=(
	MemoryStream&& that)
{
	if (std::addressof(that) != this)
	{
		static_cast<void>(Stream::operator=(std::move(that)));

		flags_ = std::move(that.flags_);
		internal_buffer_ = std::move(that.internal_buffer_);
		buffer_ptr_ = std::move(that.buffer_ptr_);
		current_position_ = std::move(that.current_position_);
		end_position_ = std::move(that.end_position_);
		max_size_ = std::move(that.max_size_);
	}

	return *this;
}

MemoryStream::~MemoryStream()
{
	MemoryStream::close_internal();
}


bool MemoryStream::open(
	const int buffer_size,
	const OpenMode open_mode)
{
	close_internal();
	return open_internal(true, nullptr, buffer_size, open_mode);
}

bool MemoryStream::open(
	void* buffer,
	const int buffer_size)
{
	return open(buffer, buffer_size, OpenMode::read_write);
}

bool MemoryStream::open(
	const void* buffer,
	const int buffer_size)
{
	return open(const_cast<void*>(buffer), buffer_size, OpenMode::read);
}

bool MemoryStream::open(
	void* buffer,
	const int buffer_size,
	const OpenMode open_mode)
{
	close_internal();
	return open_internal(false, buffer, buffer_size, open_mode);
}

bool MemoryStream::do_is_open() const
{
	return flags_.has_any(Flags::is_open);
}

bool MemoryStream::do_is_readable() const
{
	return flags_.has_all(Flags::is_open | Flags::is_readable);
}

bool MemoryStream::do_is_writable() const
{
	return flags_.has_all(Flags::is_open | Flags::is_writable);
}

bool MemoryStream::do_is_seekable() const
{
	return flags_.has_any(Flags::is_open);
}

bool MemoryStream::do_is_failed() const
{
	return !flags_.has_any(Flags::is_open);
}

void MemoryStream::do_close()
{
	MemoryStream::close_internal();
}

int MemoryStream::do_read(
	void* buffer,
	const int count)
{
	if (!flags_.has_all(Flags::is_open | Flags::is_readable) || !buffer || count < 0 || current_position_ < 0)
	{
		return -1;
	}

	if (count == 0)
	{
		return 0;
	}

	const auto remain_size = max_size_ - current_position_;

	if (remain_size <= 0)
	{
		return 0;
	}

	const auto read_count = std::min(static_cast<Position>(count), remain_size);

	if (read_count <= 0)
	{
		return 0;
	}

	std::char_traits<char>::copy(
		static_cast<char*>(buffer),
		buffer_ptr_ + current_position_,
		static_cast<std::size_t>(read_count));

	current_position_ += read_count;

	return static_cast<int>(read_count);
}

int MemoryStream::do_write(
	const void* buffer,
	const int count)
{
	if (!flags_.has_all(Flags::is_open | Flags::is_writable) || !buffer || count < 0 || current_position_ < 0)
	{
		return -1;
	}

	if (count == 0)
	{
		return 0;
	}

	const auto remain_size = max_size_ - current_position_;

	if (flags_.has_any(Flags::is_internal_buffer))
	{
		if (remain_size < count)
		{
			const auto new_size = current_position_ + count;
			internal_buffer_.resize(static_cast<InternalBuffer::size_type>(new_size));
			max_size_ = static_cast<int>(new_size);
			buffer_ptr_ = internal_buffer_.data();
		}
	}
	else
	{
		if (remain_size <= 0)
		{
			return -1;
		}
	}

	std::uninitialized_copy_n(
		static_cast<const char*>(buffer),
		count,
		buffer_ptr_ + current_position_);

	current_position_ += count;

	if (current_position_ > end_position_)
	{
		end_position_ = static_cast<int>(current_position_);
	}

	return count;
}

MemoryStream::Position MemoryStream::do_get_position()
{
	return flags_.has_any(Flags::is_open) ? current_position_ : -1;
}

MemoryStream::Position MemoryStream::do_set_position(
	const Position offset,
	const Origin origin)
{
	if (!flags_.has_any(Flags::is_open))
	{
		return -1;
	}

	auto new_position = Position{};

	switch (origin)
	{
	case Origin::begin:
		new_position = offset;
		break;

	case Origin::current:
		new_position = current_position_ + offset;
		break;

	case Origin::end:
		new_position = end_position_ + offset;
		break;

	default:
		return -1;
	}

	if (new_position < 0)
	{
		return -1;
	}

	current_position_ = new_position;

	return new_position;
}

void MemoryStream::close_internal()
{
	flags_.clear();
	internal_buffer_ = {};
	buffer_ptr_ = nullptr;
	current_position_ = 0;
	end_position_ = 0;
	max_size_ = 0;

	Stream::close_internal();
}

MemoryStream::Position MemoryStream::do_get_size()
{
	if (!flags_.has_any(Flags::is_open))
	{
		return -1;
	}

	return max_size_;
}

bool MemoryStream::open_internal(
	bool is_internal_buffer,
	void* buffer,
	const int buffer_size,
	const OpenMode open_mode)
{
	const auto is_readable = ((open_mode & OpenMode::read) != 0);
	const auto is_writable = ((open_mode & OpenMode::write) != 0);

	if (!is_readable && !is_writable)
	{
		return false;
	}

	if (buffer_size < 0)
	{
		return false;
	}

	if (is_internal_buffer)
	{
		internal_buffer_.resize(buffer_size);
		buffer_ptr_ = internal_buffer_.data();
	}
	else
	{
		max_size_ = (buffer && buffer_size > 0 ? buffer_size : 0);
		buffer_ptr_ = static_cast<char*>(buffer);
	}

	flags_.set(Flags::is_open);
	flags_.set(is_readable ? Flags::is_readable : Flags::none);
	flags_.set(is_writable ? Flags::is_writable : Flags::none);
	flags_.set(is_internal_buffer ? Flags::is_internal_buffer : Flags::none);

	return true;
}


} // spul
} // bibendovsky
