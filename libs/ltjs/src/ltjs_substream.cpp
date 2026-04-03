/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// A substream wrapper for a stream

#include "ltjs_substream.h"
#include "ltjs_stream.h"
#include <algorithm>
#include <memory>
#include <utility>

namespace ltjs {

Substream::Substream(
	Stream* stream_ptr,
	Position offset,
	SyncPositionOnRead sync_position_on_read) noexcept
{
	open_internal(stream_ptr, offset, -1, sync_position_on_read);
}

Substream::Substream(
	Stream* stream_ptr,
	Position offset,
	Position size,
	SyncPositionOnRead sync_position_on_read) noexcept
{
	open_internal(stream_ptr, offset, size, sync_position_on_read);
}

Substream::~Substream()
{
	impl_close();
}

bool Substream::open(
	Stream* stream,
	Position offset,
	SyncPositionOnRead sync_position_on_read) noexcept
{
	impl_close();
	return open_internal(stream, offset, -1, sync_position_on_read);
}

bool Substream::open(
	Stream* stream,
	Position offset,
	Position size,
	SyncPositionOnRead sync_position_on_read) noexcept
{
	impl_close();
	return open_internal(stream, offset, size, sync_position_on_read);
}

bool Substream::is_open() const noexcept
{
	return impl_is_open();
}

void Substream::close() noexcept
{
	impl_close();
}

int Substream::read(void* buffer, int count) noexcept
{
	if (!impl_is_open() || !buffer || count < 0)
	{
		return -1;
	}
	if (count == 0 || current_position_ < begin_position_)
	{
		return 0;
	}
	const Position remain_size = end_position_ - current_position_;
	if (remain_size <= 0)
	{
		return 0;
	}
	const Position read_count = std::min(static_cast<Position>(count), remain_size);
	if (sync_position_on_read_ == SyncPositionOnRead::enable)
	{
		if (const bool set_result = stream_ptr_->set_position(current_position_);
			!set_result)
		{
			return false;
		}
	}
	const int read_result = stream_ptr_->read(buffer, static_cast<int>(read_count));
	if (read_result < 0)
	{
		return -1;
	}
	current_position_ += read_result;
	return read_result;
}

int Substream::write([[maybe_unused]] const void* buffer, [[maybe_unused]] int buffer_size) noexcept
{
	return -1;
}

Substream::Position Substream::get_position() noexcept
{
	if (!impl_is_open())
	{
		return -1;
	}
	return current_position_ - begin_position_;
}

Substream::Position Substream::set_position(Position offset, Origin origin) noexcept
{
	if (!impl_is_open())
	{
		return -1;
	}
	Position new_position = 0;
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
		if (const bool set_result = stream_ptr_->set_position(new_position);
			!set_result)
		{
			return -1;
		}
		current_position_ = new_position;
	}
	return new_position - begin_position_;
}

Substream::Position Substream::get_size() noexcept
{
	if (!impl_is_open())
	{
		return -1;
	}
	return end_position_ - begin_position_;
}

bool Substream::impl_is_open() const noexcept
{
	return stream_ptr_ != nullptr;
}

void Substream::impl_close() noexcept
{
	stream_ptr_ = nullptr;
	begin_position_ = 0;
	current_position_ = 0;
	end_position_ = 0;
}

bool Substream::open_internal(
	Stream* stream_ptr,
	Position offset,
	Position size,
	SyncPositionOnRead sync_position_on_read) noexcept
{
	if (stream_ptr == nullptr || !stream_ptr->is_open() || offset < 0)
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
	const Position stream_size = stream_ptr->get_size();
	Position end_position = (size >= 0 ? offset + size : stream_size);
	if (end_position > stream_size)
	{
		end_position = stream_size;
	}
	stream_ptr_ = stream_ptr;
	begin_position_ = offset;
	current_position_ = offset;
	end_position_ = end_position;
	sync_position_on_read_ = sync_position_on_read;
	return true;
}

} // namespace ltjs
