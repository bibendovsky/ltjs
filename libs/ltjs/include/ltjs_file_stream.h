/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// File stream

#ifndef LTJS_FILE_STREAM_INCLUDED
#define LTJS_FILE_STREAM_INCLUDED

#include "ltjs_stream.h"

namespace ltjs {

enum class FileStreamOpenMode
{
	none = 0,
	read,               // Opens the existing file for reading.
	write,              // Opens the existing file for writing.
	write_with_truncate // Truncates the existing file or creates a new one for writing.
};

class FileStream : public Stream
{
public:
	FileStream() = default;
	FileStream(const char* path, FileStreamOpenMode open_mode) noexcept;
	FileStream(const FileStream& that) = delete;
	FileStream& operator=(const FileStream& that) = delete;
	FileStream(FileStream&& that) noexcept;
	FileStream& operator=(FileStream&& that) = delete;
	~FileStream() override;

	bool open(const char* path, FileStreamOpenMode open_mode) noexcept;

	bool is_open() const noexcept override;
	void close() noexcept override;
	int read(void* buffer, int count) noexcept override;
	int write(const void* buffer, int count) noexcept override;
	Position get_position() noexcept override;
	using Stream::set_position;
	Position set_position(Position offset, Origin origin) noexcept override;
	Position get_size() noexcept override;
	using Stream::skip;

private:
	void* handle_{};
	bool is_readable_{};

	static void close_handle(void* handle) noexcept;
	bool impl_is_open() const noexcept;
	void impl_close() noexcept;
	bool internal_open(const char* path, FileStreamOpenMode open_mode) noexcept;
	bool impl_open(const char* path, FileStreamOpenMode open_mode) noexcept;
};

} // namespace ltjs

#endif // LTJS_FILE_STREAM_INCLUDED
