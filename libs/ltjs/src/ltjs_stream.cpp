/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Base I/O stream class

#include "ltjs_stream.h"
#include <cstddef>

namespace ltjs {

auto Stream::get_position() noexcept -> Position
{
	return skip(0);
}

bool Stream::read_exactly(void* buffer, int size) noexcept
{
	std::byte* bytes = static_cast<std::byte*>(buffer);
	while (size > 0)
	{
		const int read_size = read(bytes, size);
		if (read_size <= 0)
		{
			return false;
		}
		bytes += read_size;
		size -= read_size;
	}
	return true;
}

bool Stream::write_exactly(const void* buffer, int size) noexcept
{
	const std::byte* bytes = static_cast<const std::byte*>(buffer);
	while (size > 0)
	{
		const int written_size = write(bytes, size);
		if (written_size <= 0)
		{
			return false;
		}
		bytes += written_size;
		size -= written_size;
	}
	return true;
}

bool Stream::set_position(Position position) noexcept
{
	return set_position(position, Origin::begin) >= 0;
}

Stream::Position Stream::skip(Position offset) noexcept
{
	return set_position(offset, Origin::current);
}

} // namespace ltjs
