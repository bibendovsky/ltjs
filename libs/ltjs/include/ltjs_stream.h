/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Base I/O stream class

#ifndef LTJS_STREAM_INCLUDED
#define LTJS_STREAM_INCLUDED

#include <cstdint>

namespace ltjs {

class Stream
{
public:
	using Position = std::int64_t;

	enum class Origin
	{
		none    = 0,
		begin   = 1, // Advances a position from the beginning.
		current = 2, // Advances a position from the current one.
		end     = 3  // Advances a position from the end.
	};

	Stream() = default;
	virtual ~Stream() = default;

	virtual void close() noexcept = 0;
	virtual bool is_open() const noexcept = 0;
	virtual int read(void* buffer, int max_size) noexcept = 0;
	virtual int write(const void* buffer, int size) noexcept = 0;
	virtual Position get_position() noexcept;
	virtual Position set_position(Position offset, Origin origin) noexcept = 0;
	virtual Position get_size() noexcept = 0;

	bool read_exactly(void* buffer, int size) noexcept;
	bool write_exactly(const void* buffer, int size) noexcept;
	bool set_position(Position position) noexcept;
	Position skip(const Position offset) noexcept;
};

} // namespace ltjs

#endif // LTJS_STREAM_INCLUDED
