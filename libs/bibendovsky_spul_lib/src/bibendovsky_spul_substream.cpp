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
// A substream wrapper for a stream.
//


#include "bibendovsky_spul_precompiled.h"
#include "bibendovsky_spul_substream.h"
#include <algorithm>
#include <memory>
#include <utility>
#include "bibendovsky_spul_stream.h"


namespace bibendovsky
{
namespace spul
{


Substream::Substream()
	:
	Stream{},
	flags_{},
	stream_ptr_{},
	begin_position_{},
	current_position_{},
	end_position_{}
{
}

Substream::Substream(
	StreamPtr stream_ptr,
	const Position offset,
	const SyncPositionOnRead sync_position_on_read)
	:
	Substream{}
{
	static_cast<void>(open_internal(stream_ptr, offset, -1, sync_position_on_read));
}

Substream::Substream(
	StreamPtr stream_ptr,
	const Position offset,
	const Position size,
	const SyncPositionOnRead sync_position_on_read)
	:
	Substream{}
{
	static_cast<void>(open_internal(stream_ptr, offset, size, sync_position_on_read));
}

Substream::~Substream()
{
	Substream::close_internal();
}

bool Substream::open(
	StreamPtr stream,
	const Position offset,
	const SyncPositionOnRead sync_position_on_read)
{
	close_internal();
	return open_internal(stream, offset, -1, sync_position_on_read);
}

bool Substream::open(
	StreamPtr stream,
	const Position offset,
	const Position size,
	const SyncPositionOnRead sync_position_on_read)
{
	close_internal();
	return open_internal(stream, offset, size, sync_position_on_read);
}

bool Substream::do_is_open() const
{
	return flags_.has_any(Flags::is_open);
}

bool Substream::do_is_readable() const
{
	return flags_.has_any(Flags::is_open);
}

bool Substream::do_is_writable() const
{
	return false;
}

bool Substream::do_is_seekable() const
{
	return flags_.has_any(Flags::is_open);
}

bool Substream::do_is_failed() const
{
	return !flags_.has_any(Flags::is_open) || flags_.has_any(Flags::is_failed);
}

void Substream::do_close()
{
	Substream::close_internal();
}

int Substream::do_read(
	void* buffer,
	const int count)
{
	if (!flags_.has_any(Flags::is_open) ||
		flags_.has_any(Flags::is_failed) ||
		!buffer ||
		count < 0)
	{
		return -1;
	}

	if (count == 0 || current_position_ < begin_position_)
	{
		return 0;
	}

	const auto remain_size = end_position_ - current_position_;

	if (remain_size <= 0)
	{
		return 0;
	}

	const auto read_count = std::min(static_cast<Position>(count), remain_size);

	if (sync_position_on_read_ == SyncPositionOnRead::enable)
	{
		const auto set_result = stream_ptr_->set_position(current_position_);

		if (!set_result)
		{
			flags_.set(Flags::is_failed);
			return false;
		}
	}

	const auto read_result = stream_ptr_->read(buffer, static_cast<int>(read_count));

	if (read_result < 0)
	{
		flags_.set(Flags::is_failed);
		return -1;
	}

	current_position_ += read_result;

	return read_result;
}

int Substream::do_write(
	const void* buffer,
	const int buffer_size)
{
	static_cast<void>(buffer);
	static_cast<void>(buffer_size);

	return -1;
}

Substream::Position Substream::do_get_position()
{
	if (!flags_.has_any(Flags::is_open) || flags_.has_any(Flags::is_failed))
	{
		return -1;
	}

	return current_position_ - begin_position_;
}

Substream::Position Substream::do_set_position(
	const Position offset,
	const Origin origin)
{
	if (!flags_.has_any(Flags::is_open) || flags_.has_any(Flags::is_failed))
	{
		return -1;
	}

	auto new_position = Position{};

	switch (origin)
	{
	case Origin::begin:
		new_position = begin_position_ + offset;
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

	if (new_position < begin_position_)
	{
		new_position = begin_position_;
	}

	if (new_position != current_position_)
	{
		const auto set_result = stream_ptr_->set_position(new_position);

		if (!set_result)
		{
			flags_.set(Flags::is_failed);
			return -1;
		}

		current_position_ = new_position;
	}

	return new_position - begin_position_;
}

Substream::Position Substream::do_get_size()
{
	if (!flags_.has_any(Flags::is_open) || flags_.has_any(Flags::is_failed))
	{
		return -1;
	}

	return end_position_ - begin_position_;
}

void Substream::close_internal()
{
	flags_.clear();
	stream_ptr_ = nullptr;
	begin_position_ = 0;
	current_position_ = 0;
	end_position_ = 0;

	Stream::close_internal();
}

bool Substream::open_internal(
	StreamPtr stream_ptr,
	const Position offset,
	const Position size,
	const SyncPositionOnRead sync_position_on_read)
{
	if (!stream_ptr ||
		!stream_ptr->is_open() ||
		!stream_ptr->is_readable() ||
		stream_ptr->is_writable() ||
		!stream_ptr->is_seekable() ||
		offset < 0)
	{
		return false;
	}

	switch (sync_position_on_read)
	{
	case SyncPositionOnRead::disable:
	case SyncPositionOnRead::enable:
		break;

	default:
		return false;
	}

	const auto stream_size = stream_ptr->get_size();

	auto end_position = (size >= 0 ? offset + size : stream_size);

	if (end_position > stream_size)
	{
		end_position = stream_size;
	}

	flags_.set(Flags::is_open);
	stream_ptr_ = stream_ptr;
	begin_position_ = offset;
	current_position_ = offset;
	end_position_ = end_position;
	sync_position_on_read_ = sync_position_on_read;

	return true;
}


} // spul
} // bibendovsky
