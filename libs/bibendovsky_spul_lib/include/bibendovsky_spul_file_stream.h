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


#ifndef BIBENDOVSKY_SPUL_FILE_STREAM_INCLUDED
#define BIBENDOVSKY_SPUL_FILE_STREAM_INCLUDED


#include <string>
#include "bibendovsky_spul_stream.h"


namespace bibendovsky
{
namespace spul
{


//
// A file stream.
//
class FileStream :
	public Stream
{
public:
	//
	// Constructs an uninitialized stream.
	//
	FileStream();

	//
	// Constructs a file stream.
	//
	// Parameters:
	//    - file_name_utf8 - a file name in UTF-8.
	//    - open_mode - an open mode.
	//
	FileStream(
		const char* const file_name_utf8,
		const OpenMode open_mode);

	//
	// Constructs a file stream.
	//
	// Parameters:
	//    - file_name_utf8 - a file name in UTF-8.
	//    - open_mode - an open mode.
	//
	FileStream(
		const std::string& file_name_utf8,
		const OpenMode open_mode);

	FileStream(
		const FileStream& that) = delete;

	FileStream& operator=(
		const FileStream& that) = delete;

	FileStream(
		FileStream&& that) noexcept;

	FileStream& operator=(
		FileStream&& that) = delete;

	~FileStream() override;


	//
	// Initializes a file stream.
	//
	// Parameters:
	//    - file_name_utf8 - a file name in UTF-8.
	//    - open_mode - an open mode.
	//
	// Returns:
	//    - "true" on sucess.
	//    - "false" otherwise.
	//
	bool open(
		const char* const file_name_utf8,
		const OpenMode open_mode);

	//
	// Initializes a file stream.
	//
	// Parameters:
	//    - file_name_utf8 - a file name in UTF-8.
	//    - open_mode - an open mode.
	//
	// Returns:
	//    - "true" on sucess.
	//    - "false" otherwise.
	//
	bool open(
		const std::string& file_name_utf8,
		const OpenMode open_mode);


protected:
	void close_internal();


private:
	struct Detail;

	using Handle = void*;

	struct Flags :
		public EnumFlags
	{
		Flags(
			const Value flags = none)
			:
			EnumFlags{flags}
		{
		}

		enum : Value
		{
			is_open = 0B0000'0001,
			is_failed = 0B0000'0010,
			is_readable = 0B0000'0100,
			is_writable = 0B0000'1000,
			is_seekable = 0B0001'0000,
		}; // enum
	}; // Flags


	Flags flags_;
	Handle handle_;


	bool do_is_open() const override;

	bool do_is_readable() const override;

	bool do_is_writable() const override;

	bool do_is_seekable() const override;

	bool do_is_failed() const override;

	void do_close() override;

	int do_read(
		void* buffer,
		const int count) override;

	int do_write(
		const void* buffer,
		const int count) override;

	Position do_get_position() override;

	Position do_set_position(
		const Position offset,
		const Origin origin) override;

	Position do_get_size() override;

	bool open_internal(
		const char* const file_name,
		const OpenMode open_mode);
}; // FileStream


using FileStreamPtr = FileStream*;


} // spul
} // bibendovsky


#endif // !BIBENDOVSKY_SPUL_FILE_STREAM_INCLUDED
