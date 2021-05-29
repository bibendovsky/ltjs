#include "ltjs_file.h"

#include "SDL.h"

#include "ltjs_sdl_uresources.h"


namespace ltjs
{
namespace file
{


Index get_size(
	const char* path)
{
	const auto sdl_rwops = SdlRwOpsUResource{::SDL_RWFromFile(path, "rb")};

	if (!sdl_rwops)
	{
		return 0;
	}

	const auto sdl_file_size = sdl_rwops->seek(sdl_rwops.get(), 0, RW_SEEK_END);

	if (sdl_file_size <= 0)
	{
		return 0;
	}

	return static_cast<Index>(sdl_file_size);
}

Index load(
	const char* path,
	void* buffer,
	Index buffer_size) noexcept
{
	if (!buffer || buffer_size <= 0)
	{
		return 0;
	}

	const auto sdl_rwops = SdlRwOpsUResource{::SDL_RWFromFile(path, "rb")};

	if (!sdl_rwops)
	{
		return 0;
	}

	const auto sdl_file_size = sdl_rwops->seek(sdl_rwops.get(), 0, RW_SEEK_END);

	if (sdl_file_size <= 0 || sdl_file_size > buffer_size)
	{
		return 0;
	}

	sdl_rwops->seek(sdl_rwops.get(), 0, RW_SEEK_SET);

	const auto sdl_read_size = sdl_rwops->read(
		sdl_rwops.get(),
		buffer,
		1,
		static_cast<std::size_t>(sdl_file_size)
	);

	if (sdl_read_size != static_cast<std::size_t>(sdl_file_size))
	{
		return 0;
	}

	return static_cast<Index>(sdl_file_size);
}

bool save(
	const char* path,
	const void* buffer,
	Index buffer_size) noexcept
{
	if (!buffer || buffer_size <= 0)
	{
		return false;
	}

	const auto sdl_rwops = SdlRwOpsUResource{::SDL_RWFromFile(path, "wb")};

	if (!sdl_rwops)
	{
		return false;
	}

	const auto sdl_written_size = sdl_rwops->write(
		sdl_rwops.get(),
		buffer,
		1,
		static_cast<std::size_t>(buffer_size)
	);

	if (static_cast<Index>(sdl_written_size) != buffer_size)
	{
		return false;
	}

	return true;
}


} // file
} // ltjs
