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
// A substream wrapper for a file stream.
//


#include "bibendovsky_spul_precompiled.h"
#include "bibendovsky_spul_file_substream.h"
#include <algorithm>
#include <memory>
#include <utility>


namespace bibendovsky
{
namespace spul
{


FileSubstream::FileSubstream()
	:
	Stream{},
	file_stream_{},
	substream_{}
{
}

FileSubstream::FileSubstream(
	const char* const file_name_utf8,
	const Position offset)
	:
	FileSubstream{}
{
	static_cast<void>(open_internal(file_name_utf8, offset, -1));
}

FileSubstream::FileSubstream(
	const std::string& file_name_utf8,
	const Position offset)
	:
	FileSubstream{}
{
	static_cast<void>(open_internal(file_name_utf8.c_str(), offset, -1));
}

FileSubstream::FileSubstream(
	const char* const file_name_utf8,
	const Position offset,
	const Position size)
	:
	FileSubstream{}
{
	static_cast<void>(open_internal(file_name_utf8, offset, size));
}

FileSubstream::FileSubstream(
	const std::string& file_name_utf8,
	const Position offset,
	const Position size)
	:
	FileSubstream{}
{
	static_cast<void>(open_internal(file_name_utf8.c_str(), offset, size));
}

FileSubstream::FileSubstream(
	FileSubstream&& that) noexcept
	:
	Stream{std::move(that)},
	file_stream_{std::move(file_stream_)},
	substream_{std::move(substream_)}
{
}

FileSubstream::~FileSubstream()
{
	FileSubstream::close_internal();
}

bool FileSubstream::open(
	const char* const file_name_utf8,
	const Position offset)
{
	close_internal();
	return open_internal(file_name_utf8, offset, -1);
}

bool FileSubstream::open(
	const std::string& file_name_utf8,
	const Position offset)
{
	close_internal();
	return open_internal(file_name_utf8.c_str(), offset, -1);
}

bool FileSubstream::open(
	const char* const file_name_utf8,
	const Position offset,
	const Position size)
{
	close_internal();
	return open_internal(file_name_utf8, offset, size);
}

bool FileSubstream::open(
	const std::string& file_name_utf8,
	const Position offset,
	const Position size)
{
	close_internal();
	return open_internal(file_name_utf8.c_str(), offset, size);
}

bool FileSubstream::do_is_open() const
{
	return substream_.is_open();
}

bool FileSubstream::do_is_readable() const
{
	return substream_.is_readable();
}

bool FileSubstream::do_is_writable() const
{
	return false;
}

bool FileSubstream::do_is_seekable() const
{
	return substream_.is_seekable();
}

bool FileSubstream::do_is_failed() const
{
	return substream_.is_failed();
}

void FileSubstream::do_close()
{
	FileSubstream::close_internal();
}

int FileSubstream::do_read(
	void* buffer,
	const int count)
{
	return substream_.read(buffer, count);
}

int FileSubstream::do_write(
	const void* buffer,
	const int buffer_size)
{
	static_cast<void>(buffer);
	static_cast<void>(buffer_size);

	return -1;
}

FileSubstream::Position FileSubstream::do_get_position()
{
	return substream_.get_position();
}

FileSubstream::Position FileSubstream::do_set_position(
	const Position offset,
	const Origin origin)
{
	return substream_.set_position(offset, origin);
}

FileSubstream::Position FileSubstream::do_get_size()
{
	return substream_.get_size();
}

void FileSubstream::close_internal()
{
	substream_.close();
	file_stream_.close();

	Stream::close_internal();
}

bool FileSubstream::open_internal(
	const char* const file_name_utf8,
	const Position offset,
	const Position size)
{
	if (!file_stream_.open(file_name_utf8, OpenMode::read))
	{
		return false;
	}

	if (!substream_.open(&file_stream_, offset, size, Substream::SyncPositionOnRead::disable))
	{
		close_internal();
		return false;
	}

	return true;
}


} // spul
} // bibendovsky
