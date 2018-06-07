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
// A substream wrapper for a stream.
//


#ifndef BIBENDOVSKY_SPUL_SUBSTREAM_INCLUDED
#define BIBENDOVSKY_SPUL_SUBSTREAM_INCLUDED


#include "bibendovsky_spul_enum_flags.h"
#include "bibendovsky_spul_stream.h"


namespace bibendovsky
{
namespace spul
{


//
// A substream wrapper for a stream.
//
class Substream :
	public Stream
{
public:
	enum class SyncPositionOnRead
	{
		none,
		enable,
		disable,
	}; // SyncPositionOnRead


	//
	// Constructs an uninitialized substream.
	//
	Substream();

	//
	// Constructs a substream.
	//
	// Parameters:
	//    - stream - an underlying stream instance.
	//    - offset - a beginning position in the underlying stream.
	//    - sync_position_on_read - controls synchronization of a position of the underlying stream on read.
	//
	// Notes:
	//    - A size of the substream will be substruction of the underlying stream size and the offset.
	//
	Substream(
		StreamPtr stream_ptr,
		const Position offset,
		const SyncPositionOnRead sync_position_on_read = SyncPositionOnRead::enable);

	//
	// Constructs a substream.
	//
	// Parameters:
	//    - stream - an underlying stream instance.
	//    - offset - a beginning position in the underlying stream.
	//    - size - a size of the substream.
	//      Pass a negative value to use remaining size of the underlying stream.
	//    - sync_position_on_read - controls synchronization of a position of the underlying stream on read.
	//
	Substream(
		StreamPtr stream_ptr,
		const Position offset,
		const Position size,
		const SyncPositionOnRead sync_position_on_read = SyncPositionOnRead::enable);

	Substream(
		const Substream& that) = default;

	Substream& operator=(
		const Substream& that) = default;

	~Substream() override;


	//
	// Initializes a substream.
	//
	// Parameters:
	//    - stream - an underlying stream instance.
	//    - offset - a beginning position in the underlying stream.
	//    - sync_position_on_read - controls synchronization of a position of the underlying stream on read.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" otherwise.
	//
	// Notes:
	//    - A size of the substream is a substruction of the underlying stream size and the provided offset.
	//
	bool open(
		StreamPtr stream,
		const Position offset,
		const SyncPositionOnRead sync_position_on_read = SyncPositionOnRead::enable);

	//
	// Initializes a substream.
	//
	// Parameters:
	//    - stream - an underlying stream instance.
	//    - offset - a beginning position in the underlying stream.
	//    - size - a size of the substream.
	//      Pass a negative value to use remaining size of the underlying stream.
	//    - sync_position_on_read - controls synchronization of a position of the underlying stream on read.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" otherwise.
	//
	bool open(
		StreamPtr stream,
		const Position offset,
		const Position size,
		const SyncPositionOnRead sync_position_on_read = SyncPositionOnRead::enable);


protected:
	void close_internal();


private:
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
			is_open = 0B0001,
			is_failed = 0B0010,
		}; // enum
	}; // Flags


	Flags flags_;
	StreamPtr stream_ptr_; // An underlying stream.
	Position begin_position_; // A beginning position in the underlying stream.
	Position current_position_; // Internal current position.
	Position end_position_; // Internal end position.
	SyncPositionOnRead sync_position_on_read_; // Controls synchronization of a position of the underlying stream on read.


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
		StreamPtr stream_ptr,
		const Position offset,
		const Position size,
		const SyncPositionOnRead sync_position_on_read);
}; // Substream


using SubstreamPtr = Substream*;


} // spul
} // bibendovsky


#endif // !BIBENDOVSKY_SPUL_SUBSTREAM_INCLUDED
