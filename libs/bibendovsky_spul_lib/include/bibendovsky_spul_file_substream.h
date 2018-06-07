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


#ifndef BIBENDOVSKY_SPUL_FILE_SUBSTREAM_INCLUDED
#define BIBENDOVSKY_SPUL_FILE_SUBSTREAM_INCLUDED


#include "bibendovsky_spul_enum_flags.h"
#include "bibendovsky_spul_file_stream.h"
#include "bibendovsky_spul_substream.h"


namespace bibendovsky
{
namespace spul
{


//
// A substream wrapper for a file stream.
//
class FileSubstream :
	public Stream
{
public:
	//
	// Constructs an uninitialized substream.
	//
	FileSubstream();

	//
	// Constructs a substream.
	//
	// Parameters:
	//    - file_name_utf8 - a file name in UTF-8.
	//    - offset - a beginning position in the file.
	//
	// Notes:
	//    - A size of the substream is substruction of the file size and the offset.
	//
	FileSubstream(
		const char* const file_name_utf8,
		const Position offset);

	//
	// Constructs a substream.
	//
	// Parameters:
	//    - file_name_utf8 - a file name in UTF-8.
	//    - offset - a beginning position in the file.
	//
	// Notes:
	//    - A size of the substream is substruction of the file size and the offset.
	//
	FileSubstream(
		const std::string& file_name_utf8,
		const Position offset);

	//
	// Constructs a substream.
	//
	// Parameters:
	//    - file_name_utf8 - a file name in UTF-8.
	//    - offset - a beginning position in the file.
	//    - size - a size of the substream.
	//      Pass a negative value to use remaining size of the file.
	//
	FileSubstream(
		const char* const file_name_utf8,
		const Position offset,
		const Position size);

	//
	// Constructs a substream.
	//
	// Parameters:
	//    - file_name_utf8 - a file name in UTF-8.
	//    - offset - a beginning position in the file.
	//    - size - a size of the substream.
	//      Pass a negative value to use remaining size of the file.
	//
	FileSubstream(
		const std::string& file_name_utf8,
		const Position offset,
		const Position size);

	FileSubstream(
		const FileSubstream& that) = default;

	FileSubstream& operator=(
		const FileSubstream& that) = default;

	FileSubstream(
		FileSubstream&& that) noexcept;

	FileSubstream& operator=(
		FileSubstream&& that) = delete;

	~FileSubstream() override;


	//
	// Initializes a substream.
	//
	// Parameters:
	//    - file_name_utf8 - a file name in UTF-8.
	//    - offset - a beginning position in the file.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" otherwise.
	//
	// Notes:
	//    - A size of the substream is a substruction of the file size and the provided offset.
	//
	bool open(
		const char* const file_name_utf8,
		const Position offset);

	//
	// Initializes a substream.
	//
	// Parameters:
	//    - file_name_utf8 - a file name in UTF-8.
	//    - offset - a beginning position in the file.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" otherwise.
	//
	// Notes:
	//    - A size of the substream is a substruction of the file size and the provided offset.
	//
	bool open(
		const std::string& file_name_utf8,
		const Position offset);

	//
	// Initializes a substream.
	//
	// Parameters:
	//    - file_name_utf8 - a file name in UTF-8.
	//    - offset - a beginning position in the file.
	//    - size - a size of the substream.
	//      Pass a negative value to use remaining size of the file.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" otherwise.
	//
	bool open(
		const char* const file_name_utf8,
		const Position offset,
		const Position size);

	//
	// Initializes a substream.
	//
	// Parameters:
	//    - file_name_utf8 - a file name in UTF-8.
	//    - offset - a beginning position in the file.
	//    - size - a size of the substream.
	//      Pass a negative value to use remaining size of the file.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" otherwise.
	//
	bool open(
		const std::string& file_name_utf8,
		const Position offset,
		const Position size);


protected:
	void close_internal();


private:
	FileStream file_stream_;
	Substream substream_;


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
		const char* const file_name_utf8,
		const Position offset,
		const Position size);
}; // FileSubstream


using FileSubstreamPtr = FileSubstream*;


} // spul
} // bibendovsky


#endif // !BIBENDOVSKY_SPUL_FILE_SUBSTREAM_INCLUDED
