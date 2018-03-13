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


#ifndef BIBENDOVSKY_SPUL_STREAM_INCLUDED
#define BIBENDOVSKY_SPUL_STREAM_INCLUDED


#include <cstdint>
#include "bibendovsky_spul_enum_flags.h"


namespace bibendovsky
{
namespace spul
{


//
// Base stream class.
//
// Notes:
//    Do not use virtual methods in constructor/destructor.
//
class Stream
{
public:
	using Position = std::int64_t;


	// Position origin.
	enum class Origin
	{
		none = 0,

		// Advances a position from the beginning.
		begin = 1,

		// Advances a position from the current one.
		current = 2,

		// Advances a position from the end.
		end = 3,
	}; // Origin

	// Open mode.
	struct OpenMode :
		EnumFlags
	{
		OpenMode(
			const Value flags = {})
			:
			EnumFlags{flags}
		{
		}

		enum : Value
		{
			none = 0B0000,

			// Read mode.
			// Data should be exists.
			read = 0B0001,

			// Write mode.
			// Does not truncate an existing data implicitly.
			write = 0B0010,

			// Read and write.
			read_write = read | write,

			// Truncates a content of a stream.
			// Should be used in combination with "write" mode.
			truncate = 0B0100,

			// Moves the file pointer to the end after opening a file.
			// A stream should be seekable.
			at_the_end = 0B1000,
		}; // enum
	}; // OpenMode


	virtual ~Stream();


	//
	// Closes the stream.
	//
	void close();

	//
	// Checks if the stream is open.
	//
	// Returns:
	//    - "true" if the stream is open.
	//    - "false" otherwise.
	//
	bool is_open() const;

	//
	// Checks if the stream supports reading.
	//
	// Returns:
	//    - "true" if the stream supports reading.
	//    - "false" otherwise.
	//
	bool is_readable() const;

	//
	// Checks if the stream supports writing.
	//
	// Returns:
	//    - "true" if the stream supports writing.
	//    - "false" otherwise.
	//
	bool is_writable() const;

	//
	// Checks if the stream supports seeking.
	//
	// Returns:
	//    - "true" if the stream supports seeking.
	//    - "false" otherwise.
	//
	bool is_seekable() const;

	//
	// Checks if the stream encountered a fatal error.
	//
	// Returns:
	//    - "true" if the stream encountered a fatal error.
	//    - "false" otherwise.
	//
	// Notes:
	//    - To clear this flag you have to re-open the stream.
	//
	bool is_failed() const;

	//
	// Reads a sequence of bytes and advances the position by the number of bytes read.
	//
	// Parameters:
	//    - buffer - a buffer to store bytes read.
	//    - count - a maximum number of bytes to read.
	//
	// Returns:
	//    - Actual number of bytes read.
	//    - A negative value on error.
	//
	// Notes:
	//    - On device I/O error the method sets "is_failed" to "true".
	//
	int read(
		void* buffer,
		const int count);

	//
	// Writes a sequence of bytes and advances the position by the number of bytes written.
	//
	// Parameters:
	//    - buffer - a buffer of bytes to write.
	//    - count - a maximum number of bytes to write.
	//
	// Returns:
	//    - Actual number of bytes written into the stream.
	//    - A negative value on error.
	//
	// Notes:
	//    - On device I/O error the method sets "is_failed" to "true".
	//
	int write(
		const void* buffer,
		const int count);

	//
	// Gets a current position in the stream.
	//
	// Returns:
	//    - A current position in the stream.
	//    - A negative value on error.
	//
	// Notes:
	//    - The stream must support seeking.
	//
	Position get_position();

	//
	// Sets a current position.
	//
	// Parameters:
	//    - position - a new position to set.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" on error.
	//
	// Notes:
	//    - The stream must support seeking.
	//    - On device I/O error the method sets "is_failed" to "true".
	//
	bool set_position(
		const Position position);

	//
	// Sets a current position according to an origin.
	//
	// Parameters:
	//    - offset - an offset at origin to set.
	//    - origin - origin identifier.
	//
	// Returns:
	//    - A new position.
	//    - A negative value on error.
	//
	// Notes:
	//    - The stream must support seeking.
	//    - Effective position should not be negative.
	//    - On device I/O error the method sets "is_failed" to "true".
	//
	Position set_position(
		const Position offset,
		const Origin origin);

	//
	// Gets a size of the stream.
	//
	// Returns:
	//    - A size of the stream.
	//    - A negative value on error.
	//
	// Notes:
	//    - The stream must support seeking.
	//
	Position get_size();

	//
	// Advances a position.
	//
	// Parameters:
	//    - offset - advance distance (may be negative).
	//
	// Returns:
	//    - A new position.
	//    - A negative value on error.
	//
	// Notes:
	//    - The stream must support seeking.
	//    - On device I/O error the method sets "is_failed" to "true".
	//
	Position skip(
		const Position offset);


protected:
	//
	// Uninitializes the stream.
	//
	// Notes:
	//    Implement this method with your version in derived class.
	//
	void close_internal();


private:
	virtual bool do_is_open() const = 0;

	virtual bool do_is_readable() const = 0;

	virtual bool do_is_writable() const = 0;

	virtual bool do_is_seekable() const = 0;

	virtual bool do_is_failed() const = 0;

	virtual void do_close() = 0;

	virtual int do_read(
		void* buffer,
		const int count) = 0;

	virtual int do_write(
		const void* buffer,
		const int count) = 0;

	virtual Position do_get_position() = 0;

	virtual bool do_set_position(
		const Position position);

	virtual Position do_set_position(
		const Position offset,
		const Origin origin) = 0;

	virtual Position do_get_size() = 0;

	virtual Position do_skip(
		const Position offset);
}; // Stream


using StreamPtr = Stream*;


} // spul
} // bibendovsky


#endif // !BIBENDOVSKY_SPUL_STREAM_INCLUDED
