/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// RIFF reader

#ifndef LTJS_RIFF_READER_INCLUDED
#define LTJS_RIFF_READER_INCLUDED

#include "ltjs_four_cc.h"
#include "ltjs_stream.h"
#include "ltjs_substream.h"
#include <cstdint>
#include <vector>

namespace ltjs {

class RiffReader
{
public:
	struct Chunk
	{
		FourCc id;                  // Chunk id.
		FourCc type;                // Chunk type (may not be filled).
		std::uint32_t size;         // Unaligned size of the chunk.
		std::uint32_t aligned_size; // Aligned size of the chunk.
		Substream data_stream;      // Data stream.

		Chunk();

		// Checks if id's all characters is NUL or the size is zero.
		bool is_empty() const;
	};

	RiffReader();
	explicit RiffReader(Stream* stream_ptr, FourCc type = FourCc{});

	bool open(Stream* stream_ptr, FourCc type = FourCc{});
	void close();
	bool is_open() const;
	bool descend(FourCc type = FourCc{});
	bool ascend();
	// Sets a current position in the curent chunk to the beginning.
	bool rewind();
	bool find_and_descend(FourCc id, FourCc type = FourCc{});
	Chunk get_current_chunk() const;

private:
	static constexpr int default_subchunk_count = 4;

	struct ChunkInternal
	{
		Chunk chunk;
		Stream::Position begin_position;
		Stream::Position current_position;
		Stream::Position end_position;
	};

	using Chunks = std::vector<ChunkInternal>;

	bool is_open_{};
	Stream* stream_ptr_{};
	Chunks chunks_{};

	bool descend_internal(FourCc type);
};

} // namespace ltjs

#endif // LTJS_RIFF_READER_INCLUDED
