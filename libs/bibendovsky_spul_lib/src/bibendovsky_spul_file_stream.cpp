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
// A file stream.
//


#include "bibendovsky_spul_precompiled.h"
#include "bibendovsky_spul_platform.h"
#include "bibendovsky_spul_file_stream.h"

#if defined(BIBENDOVSKY_SPUL_IS_POSIX)
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#endif // BIBENDOVSKY_SPUL_IS_POSIX

#include <limits>
#include <memory>
#include <utility>

#if defined(BIBENDOVSKY_SPUL_IS_WIN32)
#include <Windows.h>
#endif // BIBENDOVSKY_SPUL_IS_WIN32

#if defined(BIBENDOVSKY_SPUL_IS_WIN32)
#include "bibendovsky_spul_encoding_utils.h"
#endif // BIBENDOVSKY_SPUL_IS_WIN32


namespace bibendovsky
{
namespace spul
{


#if defined(BIBENDOVSKY_SPUL_IS_WIN32)
struct FileStream::Detail
{
	static Position win32_seek(
		void* handle,
		const Position position,
		const DWORD origin)
	{
		LARGE_INTEGER win32_offset;
		win32_offset.QuadPart = position;

		LARGE_INTEGER win32_position;

		const auto win32_seek_result = ::SetFilePointerEx(
			handle,
			win32_offset,
			&win32_position,
			origin);

		if (!win32_seek_result)
		{
			return -1;
		}

		return win32_position.QuadPart;
	}

	static Handle get_invalid_handle()
	{
		return INVALID_HANDLE_VALUE;
	}

	static void test_requirements()
	{
	}

	static Handle open(
		const std::string& file_name_utf8,
		const OpenMode open_mode)
	{
		const auto is_readable = ((open_mode & OpenMode::read) != 0);
		const auto is_writable = ((open_mode & OpenMode::write) != 0);
		const auto is_truncate = ((open_mode & OpenMode::truncate) != 0);

		auto win32_access = DWORD{};
		auto win32_disposition = DWORD{};
		auto win32_share = DWORD{FILE_SHARE_READ};

		if (is_readable && is_writable)
		{
			win32_access = GENERIC_READ | GENERIC_WRITE;
			win32_disposition = is_truncate ? CREATE_ALWAYS : OPEN_ALWAYS;
		}
		else if (is_readable)
		{
			win32_access = GENERIC_READ;
			win32_disposition = OPEN_EXISTING;
		}
		else if (is_writable)
		{
			win32_access = GENERIC_WRITE;
			win32_disposition = is_truncate ? CREATE_ALWAYS : OPEN_ALWAYS;
		}
		else
		{
			return get_invalid_handle();
		}

		const auto file_name_wide = EncodingUtils::utf8_to_wide(file_name_utf8);

		const auto handle = ::CreateFileW(
			file_name_wide.c_str(),
			win32_access,
			win32_share,
			nullptr,
			win32_disposition,
			FILE_ATTRIBUTE_NORMAL,
			nullptr);

		return handle;
	}

	static void close(
		const Handle handle)
	{
		static_cast<void>(::CloseHandle(handle));
	}

	static int read(
		const Handle handle,
		void* buffer,
		const int count)
	{
		DWORD win32_read_size;

		const auto win32_result = ::ReadFile(handle, buffer, count, &win32_read_size, nullptr);

		if (!win32_result)
		{
			return -1;
		}

		return static_cast<int>(win32_read_size);
	}

	static int write(
		const Handle handle,
		const void* buffer,
		const int count)
	{
		DWORD win32_write_size;

		const auto win32_result = ::WriteFile(handle, buffer, count, &win32_write_size, nullptr);

		if (!win32_result)
		{
			return -1;
		}

		return static_cast<int>(win32_write_size);
	}

	static Position set_position(
		const Handle handle,
		const Position offset,
		const Origin origin)
	{
		DWORD win32_origin;

		switch (origin)
		{
		case Origin::begin:
			win32_origin = FILE_BEGIN;
			break;

		case Origin::current:
			win32_origin = FILE_CURRENT;
			break;

		case Origin::end:
			win32_origin = FILE_END;
			break;

		default:
			return -1;
		}

		const auto seek_result = Detail::win32_seek(handle, offset, win32_origin);

		if (seek_result < 0)
		{
			return -1;
		}

		return seek_result;
	}

	static Position get_size(
		const Handle handle)
	{
		LARGE_INTEGER size;

		const auto win32_result = ::GetFileSizeEx(handle, &size);

		if (!win32_result)
		{
			return -1;
		}

		return static_cast<Position>(size.QuadPart);
	}
}; // FileStream::Detail
#endif // BIBENDOVSKY_SPUL_IS_WIN32

#if defined(BIBENDOVSKY_SPUL_IS_POSIX)
struct FileStream::Detail
{
	static int posix_descriptor(
		void* const handle)
	{
		return static_cast<int>(reinterpret_cast<std::intptr_t>(handle));
	}

	static void* posix_handle(
		const int descriptor)
	{
		return reinterpret_cast<void*>(static_cast<std::intptr_t>(descriptor));
	}


	static Handle get_invalid_handle()
	{
		return reinterpret_cast<Handle>(static_cast<std::intptr_t>(-1));
	}

	static void test_requirements()
	{
		static_assert(sizeof(off_t) == sizeof(std::int64_t), "Expected 64-bit file positioning.");
	}

	static Handle open(
		const std::string& file_name_utf8,
		const OpenMode open_mode)
	{
		const auto is_readable = ((open_mode & OpenMode::read) != 0);
		const auto is_writable = ((open_mode & OpenMode::write) != 0);
		const auto is_truncate = ((open_mode & OpenMode::truncate) != 0);

		auto posix_mode = 0;

		if (is_readable && is_writable)
		{
			posix_mode = O_RDWR | O_CREAT | (is_truncate ? O_TRUNC : 0);
		}
		else if (is_readable)
		{
			posix_mode = O_RDONLY;
		}
		else if (is_writable)
		{
			posix_mode = O_WRONLY | O_CREAT | (is_truncate ? O_TRUNC : 0);
		}

		// rw-rw-rw / 0666
		const auto permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

		const auto descriptor = ::open(file_name_utf8.c_str(), posix_mode, permissions);

		if (descriptor < 0)
		{
			return get_invalid_handle();
		}

		return posix_handle(descriptor);
	}

	static void close(
		const Handle handle)
	{
		static_cast<void>(::close(Detail::posix_descriptor(handle)));
	}

	static int read(
		const Handle handle,
		void* buffer,
		const int count)
	{
		const auto posix_result = ::read(posix_descriptor(handle), buffer, count);

		if (posix_result < 0)
		{
			return -1;
		}

		return static_cast<int>(posix_result);
	}

	static int write(
		const Handle handle,
		const void* buffer,
		const int count)
	{
		const auto posix_result = ::write(posix_descriptor(handle), buffer, count);

		if (posix_result < 0)
		{
			return -1;
		}

		return static_cast<int>(posix_result);
	}

	static Position posix_set_position(
		const Handle handle,
		const Position offset,
		const int origin)
	{
		const auto posix_result = ::lseek(posix_descriptor(handle), offset, origin);

		if (posix_result < 0)
		{
			return -1;
		}

		return posix_result;
	}

	static Position set_position(
		const Handle handle,
		const Position offset,
		const Origin origin)
	{
		int posix_origin;

		switch (origin)
		{
		case Origin::begin:
			posix_origin = SEEK_SET;
			break;

		case Origin::current:
			posix_origin = SEEK_CUR;
			break;

		case Origin::end:
			posix_origin = SEEK_END;
			break;

		default:
			return -1;
		}

		return posix_set_position(handle, offset, posix_origin);
	}

	static Position get_size(
		const Handle handle)
	{
		struct stat posix_stat;

		const auto posix_result = ::fstat(posix_descriptor(handle), &posix_stat);

		if (posix_result != 0)
		{
			return -1;
		}

		return static_cast<Position>(posix_stat.st_size);
	}
}; // FileStream::Detail
#endif // BIBENDOVSKY_SPUL_IS_POSIX


FileStream::FileStream()
	:
	Stream{},
	flags_{},
	handle_{Detail::get_invalid_handle()}
{
	Detail::test_requirements();
}

FileStream::FileStream(
	const char* const file_name_utf8,
	const OpenMode open_mode)
	:
	FileStream{}
{
	static_cast<void>(open_internal(file_name_utf8, open_mode));
}

FileStream::FileStream(
	const std::string& file_name_utf8,
	const OpenMode open_mode)
	:
	FileStream{}
{
	static_cast<void>(open_internal(file_name_utf8.c_str(), open_mode));
}

FileStream::FileStream(
	FileStream&& that) noexcept
	:
	Stream{std::move(that)},
	handle_{std::move(that.handle_)}
{
	that.handle_ = Detail::get_invalid_handle();
}

FileStream::~FileStream()
{
	FileStream::close_internal();
}

bool FileStream::open(
	const char* const file_name_utf8,
	const OpenMode open_mode)
{
	close_internal();
	return open_internal(file_name_utf8, open_mode);
}

bool FileStream::open(
	const std::string& file_name_utf8,
	const OpenMode open_mode)
{
	return open(file_name_utf8.c_str(), open_mode);
}

bool FileStream::do_is_open() const
{
	return flags_.has_any(Flags::is_open);
}

bool FileStream::do_is_readable() const
{
	return flags_.has_all(Flags::is_open | Flags::is_readable);
}

bool FileStream::do_is_writable() const
{
	return flags_.has_all(Flags::is_open | Flags::is_writable);
}

bool FileStream::do_is_seekable() const
{
	return flags_.has_all(Flags::is_open | Flags::is_seekable);
}

bool FileStream::do_is_failed() const
{
	return flags_.has_any(Flags::is_failed);
}

void FileStream::do_close()
{
	FileStream::close_internal();
}

int FileStream::do_read(
	void* buffer,
	const int count)
{
	if (!flags_.has_all(Flags::is_open | Flags::is_readable) ||
		flags_.has_any(Flags::is_failed) ||
		!buffer ||
		count < 0)
	{
		return -1;
	}

	if (count == 0)
	{
		return 0;
	}

	const auto result = Detail::read(handle_, buffer, count);

	if (result < 0)
	{
		flags_.set(Flags::is_failed);
	}

	return result;
}

int FileStream::do_write(
	const void* buffer,
	const int count)
{
	if (!flags_.has_all(Flags::is_open | Flags::is_writable) ||
		flags_.has_any(Flags::is_failed) || 
		!buffer ||
		count < 0)
	{
		return -1;
	}

	if (count == 0)
	{
		return 0;
	}

	const auto write_result = Detail::write(handle_, buffer, count);

	if (write_result < 0)
	{
		flags_.set(Flags::is_failed);
	}

	return write_result;
}

FileStream::Position FileStream::do_get_position()
{
	if (!flags_.has_all(Flags::is_open | Flags::is_seekable) || flags_.has_any(Flags::is_failed))
	{
		return -1;
	}

	return Detail::set_position(handle_, 0, Origin::current);
}

FileStream::Position FileStream::do_set_position(
	const Position offset,
	const Origin origin)
{
	if (!flags_.has_all(Flags::is_open | Flags::is_seekable) || flags_.has_any(Flags::is_failed))
	{
		return -1;
	}

	const auto set_position_result = Detail::set_position(handle_, offset, origin);

	if (set_position_result < 0)
	{
		flags_.set(Flags::is_failed);
	}

	return set_position_result;
}

void FileStream::close_internal()
{
	flags_.clear();

	if (handle_ != Detail::get_invalid_handle())
	{
		Detail::close(handle_);
		handle_ = Detail::get_invalid_handle();
	}

	Stream::close_internal();
}

FileStream::Position FileStream::do_get_size()
{
	if (!flags_.has_all(Flags::is_open | Flags::is_seekable) || flags_.has_any(Flags::is_failed))
	{
		return -1;
	}

	return Detail::get_size(handle_);
}

bool FileStream::open_internal(
	const char* const file_name_utf8,
	const OpenMode open_mode)
{
	if (!file_name_utf8)
	{
		return false;
	}

	const auto is_readable = ((open_mode & OpenMode::read) != 0);
	const auto is_writable = ((open_mode & OpenMode::write) != 0);
	const auto is_truncate = ((open_mode & OpenMode::truncate) != 0);
	const auto is_at_the_end = ((open_mode & OpenMode::at_the_end) != 0);

	if (!is_readable && !is_writable)
	{
		return false;
	}

	if (!is_writable && is_truncate)
	{
		return false;
	}

	handle_ = Detail::open(file_name_utf8, open_mode);

	if (handle_ == Detail::get_invalid_handle())
	{
		return false;
	}

	auto is_seekable = false;

	const auto end_position = Detail::set_position(handle_, 0, Origin::end);

	if (end_position >= 0)
	{
		if (!is_at_the_end)
		{
			const auto begin_position = Detail::set_position(handle_, 0, Origin::begin);

			if (begin_position != 0)
			{
				close_internal();
				return false;
			}
		}

		is_seekable = true;
	}

	flags_.set(Flags::is_open);
	flags_.set(is_readable ? Flags::is_readable : Flags::none);
	flags_.set(is_writable ? Flags::is_writable : Flags::none);
	flags_.set(is_seekable ? Flags::is_seekable : Flags::none);

	return true;
}


} // spul
} // bibendovsky
