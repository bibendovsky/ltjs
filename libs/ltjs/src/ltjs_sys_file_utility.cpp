/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// System: File utility

#include "ltjs_sys_file_utility.h"
#include <climits>
#include <cstddef>
#include "SDL3/SDL_iostream.h"

namespace ltjs::sys {

int get_file_size(const char* path) noexcept
{
	int file_size = 0;
	if (SDL_IOStream* const sdl_io_stream = SDL_IOFromFile(path, "rb");
		sdl_io_stream != nullptr)
	{
		if (const Sint64 sdl_io_size = SDL_GetIOSize(sdl_io_stream);
			sdl_io_size > 0 && sdl_io_size < INT_MAX)
		{
			file_size = static_cast<int>(sdl_io_size);
		}
		SDL_CloseIO(sdl_io_stream);
	}
	return file_size;
}

int load_file(const char* path, void* buffer, int buffer_size) noexcept
{
	int loaded_size = 0;
	if (SDL_IOStream* const sdl_io_stream = SDL_IOFromFile(path, "rb");
		sdl_io_stream != nullptr)
	{
		if (const Sint64 sdl_io_size = SDL_GetIOSize(sdl_io_stream);
			sdl_io_size > 0 && sdl_io_size <= buffer_size)
		{
			const std::size_t sdl_to_read_size = static_cast<std::size_t>(sdl_io_size);
			if (SDL_ReadIO(sdl_io_stream, buffer, sdl_to_read_size) == sdl_to_read_size)
			{
				loaded_size = static_cast<int>(sdl_to_read_size);
			}
		}
		SDL_CloseIO(sdl_io_stream);
	}
	return loaded_size;
}

bool save_file(const char* path, const void* buffer, int buffer_size) noexcept
{
	bool is_saved = false;
	if (SDL_IOStream* const sdl_io_stream = SDL_IOFromFile(path, "wb");
		sdl_io_stream != nullptr)
	{
		const std::size_t sdl_to_write_size = static_cast<std::size_t>(buffer_size);
		const bool is_written = (SDL_WriteIO(sdl_io_stream, buffer, sdl_to_write_size) == sdl_to_write_size);
		const bool is_closed = SDL_CloseIO(sdl_io_stream);
		is_saved = is_written && is_closed;
	}
	return is_saved;
}

} // namespace ltjs::sys
