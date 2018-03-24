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
// Base stream class.
//


#include "bibendovsky_spul_precompiled.h"
#include "bibendovsky_spul_stream.h"


namespace bibendovsky
{
namespace spul
{


Stream::~Stream()
{
	Stream::close_internal();
}

void Stream::close()
{
	do_close();
}

bool Stream::is_open() const
{
	return do_is_open();
}

bool Stream::is_readable() const
{
	return do_is_readable();
}

bool Stream::is_writable() const
{
	return do_is_writable();
}

bool Stream::is_seekable() const
{
	return do_is_seekable();
}

bool Stream::is_failed() const
{
	return do_is_failed();
}

int Stream::read(
	void* buffer,
	const int count)
{
	return do_read(buffer, count);
}

int Stream::write(
	const void* buffer,
	const int count)
{
	return do_write(buffer, count);
}

Stream::Position Stream::get_position()
{
	return do_get_position();
}

bool Stream::set_position(
	const Position position)
{
	return do_set_position(position);
}

Stream::Position Stream::set_position(
	const Position offset,
	const Origin origin)
{
	return do_set_position(offset, origin);
}

Stream::Position Stream::get_size()
{
	return do_get_size();
}

Stream::Position Stream::skip(
	const Position offset)
{
	return do_skip(offset);
}

bool Stream::do_set_position(
	const Position position)
{
	return do_set_position(position, Origin::begin) == position;
}

Stream::Position Stream::do_skip(
	const Stream::Position offset)
{
	return do_set_position(offset, Origin::current);
}

void Stream::close_internal()
{
}


} // spul
} // bibendovsky
