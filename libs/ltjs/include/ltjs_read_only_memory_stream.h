/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Read-only stream with memory backend

#ifndef LTJS_READ_ONLY_MEMORY_STREAM_INCLUDED
#define LTJS_READ_ONLY_MEMORY_STREAM_INCLUDED

#include "ltjs_stream.h"
#include <cstddef>

namespace ltjs {

class ReadOnlyMemoryStream : public Stream
{
public:
	ReadOnlyMemoryStream() = default;
	ReadOnlyMemoryStream(const void* data, int data_size) noexcept;
	ReadOnlyMemoryStream(const ReadOnlyMemoryStream& that) = default;
	ReadOnlyMemoryStream& operator=(const ReadOnlyMemoryStream& that) = default;
	~ReadOnlyMemoryStream() override = default;

	bool open(const void* data, int data_size) noexcept;

	bool is_open() const noexcept override;
	void close() noexcept override;
	int read(void* buffer, int count) noexcept override;
	int write(const void* buffer, int count) noexcept override;
	Position get_position() noexcept override;
	Position set_position(Position offset, Origin origin) noexcept override;
	Position get_size() noexcept override;

private:
	const std::byte* data_{};
	int data_size_{};
	int data_offset_{};

	bool impl_is_open() const noexcept;
	void impl_close() noexcept;
	bool impl_open(const void* data, int data_size) noexcept;
};

} // namespace ltjs

#endif // LTJS_READ_ONLY_MEMORY_STREAM_INCLUDED
