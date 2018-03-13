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
// RIFF reader.
//


#ifndef BIBENDOVSKY_SPUL_RIFF_READER_INCLUDED
#define BIBENDOVSKY_SPUL_RIFF_READER_INCLUDED


#include <cstdint>
#include <vector>
#include "bibendovsky_spul_four_cc.h"
#include "bibendovsky_spul_stream.h"
#include "bibendovsky_spul_substream.h"


namespace bibendovsky
{
namespace spul
{


class RiffReader
{
public:
	//
	// A chunk.
	//
	struct Chunk
	{
		FourCc id_; // Chunk id.
		FourCc type_; // Chunk type (may not be filled).
		std::uint32_t size_; // Unaligned size of the chunk.
		std::uint32_t aligned_size_; // Aligned size of the chunk.
		Substream data_stream_; // Data stream.


		Chunk();

		//
		// Test a chunk for empty.
		//
		// Returns:
		//    "true" if chunk is empty (zero id or zero size).
		//    "false" otherwise.
		bool is_empty() const;
	}; // Chunk


	//
	// Creates an uninitialized instance.
	//
	// Parameters:
	//    - stream_ptr -  a pointer to the stream with RIFF data.
	//    - type - expected type of the chunk (optional).
	//
	explicit RiffReader();

	//
	// Creates and initializes an instance.
	//
	explicit RiffReader(
		Stream* stream_ptr,
		const FourCc& type = {});

	//
	// Initializes the instance.
	//
	// Parameters:
	//    - stream_ptr -  a pointer to the stream with RIFF data.
	//    - type - expected type of the chunk (optional).
	//
	// Returns:
	//    "true" on success.
	//    "false" otherwise.
	//
	bool initialize(
		Stream* stream_ptr,
		const FourCc& type = {});

	//
	// Uninitializes the instance.
	//
	void uninitialize();

	//
	// Gets an initialization status of the instance.
	//
	// Returns:
	//    "true" if instance is initialized.
	//    "false" otherwise.
	//
	bool is_initialized() const;

	//
	// Descends into a chunk.
	//
	// Parameters:
	//    - type - expected type of the chunk (optional).
	//
	// Returns:
	//    "true" on success.
	//    "false" otherwise.
	//
	bool descend(
		const FourCc& type = {});

	//
	// Ascends out of the current chunk.
	//
	// Returns:
	//    "true" on success.
	//    "false" otherwise.
	//
	bool ascend();

	//
	// Sets a current position in the curent chunk to the beginning.
	//
	// Returns:
	//    "true" on success.
	//    "false" otherwise.
	//
	bool rewind();

	//
	// Searches for a chunk and descends into it.
	//
	// Parameters:
	//    - id - an id of a chunk to find.
	//    - type - expected type of the chunk (optional).
	//
	// Returns:
	//    "true" on success.
	//    "false" otherwise.
	//
	bool find_and_descend(
		const FourCc id,
		const FourCc type = {});

	//
	// Gets a current chunk.
	//
	// Returns:
	//    A current chunk.
	//    Empty chunk otherwise.
	//
	Chunk get_current_chunk() const;


private:
	static constexpr int default_subchunk_count = 4;


	struct ChunkInternal
	{
		Chunk chunk_;
		Stream::Position begin_position_;
		Stream::Position current_position_;
		Stream::Position end_position_;
	}; // ChunkInternal


	using Chunks = std::vector<ChunkInternal>;


	bool is_initialized_;
	StreamPtr stream_ptr_;
	Chunks chunks_;


	bool descend_internal(
		const FourCc& type);
}; // RiffReader


} // spul
} // bibendovsky


#endif // !BIBENDOVSKY_SPUL_RIFF_READER_INCLUDED
