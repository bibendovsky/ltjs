/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// File stream

#include "ltjs_file_stream.h"
#include <cassert>
#include <cstddef>
#include "SDL3/SDL_iostream.h"

namespace ltjs {

FileStream::FileStream(const char* path, FileStreamOpenMode open_mode) noexcept
{
	internal_open(path, open_mode);
}

FileStream::FileStream(FileStream&& that) noexcept
	:
	handle_{that.handle_},
	is_readable_{that.is_readable_}
{
	that.handle_ = nullptr;
	that.is_readable_ = false;
}

FileStream::~FileStream()
{
	close_handle(handle_);
}

bool FileStream::open(const char* path, FileStreamOpenMode open_mode) noexcept
{
	return impl_open(path, open_mode);
}

bool FileStream::is_open() const noexcept
{
	return impl_is_open();
}

void FileStream::close() noexcept
{
	impl_close();
}

int FileStream::read(void* buffer, int count) noexcept
{
	assert(buffer != nullptr && "Null buffer.");
	assert(count >= 0 && "Negative buffer max size.");
	assert(impl_is_open() && "Closed.");
	if (!is_readable_)
	{
		assert(false && "Non-readable.");
		return -1;
	}
	const std::size_t sdl_to_read_size = static_cast<std::size_t>(count);
	const std::size_t sdl_read_size = SDL_ReadIO(static_cast<SDL_IOStream*>(handle_), buffer, sdl_to_read_size);
	return static_cast<int>(sdl_read_size);
}

int FileStream::write(const void* buffer, int count) noexcept
{
	assert(buffer != nullptr && "Null buffer.");
	assert(count >= 0 && "Negative buffer size.");
	assert(impl_is_open() && "Closed.");
	const std::size_t sdl_to_write_size = static_cast<std::size_t>(count);
	const std::size_t sdl_written_size = SDL_WriteIO(static_cast<SDL_IOStream*>(handle_), buffer, sdl_to_write_size);
	return static_cast<int>(sdl_written_size);
}

FileStream::Position FileStream::get_position() noexcept
{
	assert(impl_is_open());
	return SDL_TellIO(static_cast<SDL_IOStream*>(handle_));
}

FileStream::Position FileStream::set_position(Position offset, Origin origin) noexcept
{
	assert(impl_is_open());
	SDL_IOWhence sdl_whence;
	switch (origin)
	{
		case Origin::begin:
			sdl_whence = SDL_IO_SEEK_SET;
			break;
		case Origin::current:
			sdl_whence = SDL_IO_SEEK_CUR;
			break;
		case Origin::end:
			sdl_whence = SDL_IO_SEEK_END;
			break;
		default:
			assert(false && "Unknown origin.");
			return -1;
	}
	return SDL_SeekIO(static_cast<SDL_IOStream*>(handle_), offset, sdl_whence);
}

FileStream::Position FileStream::get_size() noexcept
{
	assert(impl_is_open());
	return SDL_GetIOSize(static_cast<SDL_IOStream*>(handle_));
}

void FileStream::close_handle(void* handle) noexcept
{
	[[maybe_unused]] const bool is_closed = SDL_CloseIO(static_cast<SDL_IOStream*>(handle));
	assert(is_closed);
}

bool FileStream::impl_is_open() const noexcept
{
	return handle_ != nullptr;
}

void FileStream::impl_close() noexcept
{
	close_handle(handle_);
	handle_ = nullptr;
	is_readable_ = false;
}

bool FileStream::internal_open(const char* path, FileStreamOpenMode open_mode) noexcept
{
	assert(path != nullptr && "Null path.");
	const char* sdl_mode;
	const char* sdl_mode_2 = nullptr;
	bool is_readable = false;
	switch (open_mode)
	{
		case FileStreamOpenMode::read:
			is_readable = true;
			sdl_mode = "rb";
			break;
		case FileStreamOpenMode::write:
			sdl_mode = "r+b";
			sdl_mode_2 = "wxb";
			break;
		case FileStreamOpenMode::write_with_truncate:
			sdl_mode = "wb";
			break;
		default:
			assert(false && "Unknown open mode.");
			return false;
	}
	handle_ = SDL_IOFromFile(path, sdl_mode);
	if (handle_ == nullptr && sdl_mode_2 != nullptr)
	{
		handle_ = SDL_IOFromFile(path, sdl_mode_2);
	}
	if (handle_ == nullptr)
	{
		return false;
	}
	is_readable_ = is_readable;
	return true;
}

bool FileStream::impl_open(const char* path, FileStreamOpenMode open_mode) noexcept
{
	impl_close();
	return internal_open(path, open_mode);
}

} // namespace ltjs
