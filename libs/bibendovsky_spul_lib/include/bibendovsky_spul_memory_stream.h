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
// A stream with memory storage.
//


#ifndef BIBENDOVSKY_SPUL_MEMORY_STREAM_INCLUDED
#define BIBENDOVSKY_SPUL_MEMORY_STREAM_INCLUDED


#include <vector>
#include "bibendovsky_spul_enum_flags.h"
#include "bibendovsky_spul_stream.h"


namespace bibendovsky
{
namespace spul
{


//
// A stream with memory storage.
//
class MemoryStream :
	public Stream
{
public:
	//
	// Constructs an uninitialized stream.
	//
	MemoryStream();

	//
	// Constructs a stream with an internal storage.
	//
	// Parameters:
	//    - initial_buffer_size - an initial size of the storage.
	//    - open_mode - an open mode.
	//
	// Notes:
	//    Default open mode: read and write.
	//
	explicit MemoryStream(
		const int initial_buffer_size,
		const OpenMode open_mode = OpenMode::read_write);

	//
	// Constructs a modifiable stream with an external storage.
	//
	// Parameters:
	//    - buffer - storage buffer.
	//    - buffer_size - a maximum size of the storage.
	//
	MemoryStream(
		void* const buffer,
		const int buffer_size);

	//
	// Constructs a read-only stream with an external storage.
	//
	// Parameters:
	//    - buffer - storage buffer.
	//    - buffer_size - a size of the storage.
	//
	MemoryStream(
		const void* const buffer,
		const int buffer_size);

	//
	// Constructs a stream with an external storage and with a specified mode.
	//
	// Parameters:
	//    - buffer - storage buffer.
	//    - buffer_size - a size of the storage.
	//    - open_mode - desired open mode.
	//
	MemoryStream(
		void* const buffer,
		const int buffer_size,
		const OpenMode open_mode);

	//
	// Constructs a stream with an external storage and with a specified mode.
	//
	// Parameters:
	//    - buffer - storage buffer.
	//    - buffer_size - a size of the storage.
	//    - open_mode - desired open mode.
	//
	MemoryStream(
		const void* const buffer,
		const int buffer_size,
		const OpenMode open_mode);

	MemoryStream(
		const MemoryStream& that) = default;

	MemoryStream& operator=(
		const MemoryStream& that) = default;

	MemoryStream(
		MemoryStream&& that) noexcept;

	MemoryStream& operator=(
		MemoryStream&& that);

	~MemoryStream() override;


	//
	// Initializes a stream with an internal storage.
	//
	// Parameters:
	//    - initial_buffer_size - an initial size of the storage.
	//    - open_mode - an open mode.
	//
	// Returns:
	//    "true" on sucess.
	//    "false" otherwise.
	//
	bool open(
		const int buffer_size,
		const OpenMode open_mode = OpenMode::read_write);

	//
	// Initializes a modifiable stream with an external storage.
	//
	// Parameters:
	//    - buffer - storage buffer.
	//    - buffer_size - a maximum size of the storage.
	//
	// Returns:
	//    "true" on sucess.
	//    "false" otherwise.
	//
	bool open(
		void* buffer,
		const int buffer_size);

	//
	// Initializes a read-only stream with an external storage.
	//
	// Parameters:
	//    - buffer - storage buffer.
	//    - buffer_size - a size of the storage.
	//
	// Returns:
	//    "true" on sucess.
	//    "false" otherwise.
	//
	bool open(
		const void* buffer,
		const int buffer_size);

	//
	// Initializes a stream with an external storage and with a specified mode.
	//
	// Parameters:
	//    - buffer - storage buffer.
	//    - buffer_size - a size of the storage.
	//    - open_mode - a desired open mode.
	//
	// Returns:
	//    "true" on sucess.
	//    "false" otherwise.
	//
	bool open(
		void* buffer,
		const int buffer_size,
		const OpenMode open_mode);


protected:
	void close_internal();


private:
	using InternalBuffer = std::vector<char>;
	using BufferPtr = char*;


	struct Flags :
		EnumFlags
	{
		Flags(
			const Value flags = none)
			:
			EnumFlags{flags}
		{
		}

		enum : Value
		{
			is_open = 0B0001,
			is_readable = 0B0010,
			is_writable = 0B0100,
			is_internal_buffer = 0B1000,
		}; // enum
	}; // Flags


	Flags flags_;
	InternalBuffer internal_buffer_; // An internal storage buffer.
	BufferPtr buffer_ptr_; // A pointer to the storage.
	Position current_position_; // Internal current position.
	int end_position_; // Internal end position.
	int max_size_; // Maximum size of the storage.


	bool do_is_open() const override;

	bool do_is_readable() const override;

	bool do_is_writable() const override;

	bool do_is_seekable() const override;

	bool do_is_failed() const override;

	void do_close() override;

	int do_read(
		void* buffer,
		const int buffer_size) override;

	int do_write(
		const void* buffer,
		const int buffer_size) override;

	Position do_get_position() override;

	Position do_set_position(
		const Position offset,
		const Origin origin) override;

	Position do_get_size() override;

	bool open_internal(
		bool is_internal_buffer,
		void* buffer,
		const int buffer_size,
		const OpenMode open_mode);
}; // MemoryStream


using MemoryStreamPtr = MemoryStream*;


} // spul
} // bibendovsky


#endif // !BIBENDOVSKY_SPUL_MEMORY_STREAM_INCLUDED
