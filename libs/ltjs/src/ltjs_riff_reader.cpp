/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// RIFF reader

#include "ltjs_riff_reader.h"
#include "ltjs_endian.h"
#include "ltjs_riff_four_ccs.h"
#include <cstdint>

namespace ltjs {

RiffReader::Chunk::Chunk()
	:
	id{},
	type{},
	size{},
	aligned_size{},
	data_stream{}
{}

bool RiffReader::Chunk::is_empty() const
{
	return id == FourCc{} || size == 0;
}

RiffReader::RiffReader()
{
	chunks_.reserve(default_subchunk_count);
}

RiffReader::RiffReader(Stream* stream_ptr, FourCc type)
	:
	RiffReader{}
{
	open(stream_ptr, type);
}

bool RiffReader::open(Stream* stream_ptr, FourCc type)
{
	if (stream_ptr == nullptr || !stream_ptr->is_open() || type == FourCc{})
	{
		return false;
	}
	stream_ptr_ = stream_ptr;
	if (!descend_internal(type))
	{
		close();
		return false;
	}
	is_open_ = true;
	return true;
}

void RiffReader::close()
{
	is_open_ = false;
	stream_ptr_ = nullptr;
	chunks_.clear();
}

bool RiffReader::is_open() const
{
	return is_open_;
}

bool RiffReader::descend(FourCc type)
{
	if (!is_open_)
	{
		return false;
	}
	if (!descend_internal(type))
	{
		return false;
	}
	return true;
}

bool RiffReader::ascend()
{
	if (!is_open_ || chunks_.empty())
	{
		return false;
	}
	const std::uint32_t previous_chunk_aligned_size = chunks_.back().chunk.aligned_size;
	chunks_.pop_back();
	if (chunks_.empty())
	{
		if (!stream_ptr_->set_position(0))
		{
			return false;
		}
		return true;
	}
	ChunkInternal& chunk = chunks_.back();
	chunk.current_position += previous_chunk_aligned_size;
	if (!stream_ptr_->set_position(chunk.current_position))
	{
		return false;
	}
	return true;
}

bool RiffReader::rewind()
{
	if (!is_open_ || chunks_.empty())
	{
		return false;
	}
	ChunkInternal& chunk = chunks_.back();
	chunk.current_position = chunk.begin_position;
	if (!stream_ptr_->set_position(chunk.current_position))
	{
		return false;
	}
	return true;
}

bool RiffReader::find_and_descend(FourCc id, FourCc type)
{
	if (!is_open_ || id == FourCc{} || chunks_.empty())
	{
		return false;
	}
	while (true)
	{
		if (!descend_internal(type))
		{
			return false;
		}
		const ChunkInternal& chunk = chunks_.back();
		if (chunk.chunk.id == id &&
			(type != FourCc{} ? chunk.chunk.type == type : true))
		{
			return true;
		}
		if (!ascend())
		{
			return false;
		}
	}
	return false;
}

RiffReader::Chunk RiffReader::get_current_chunk() const
{
	if (chunks_.empty())
	{
		return Chunk{};
	}
	return chunks_.back().chunk;
}

bool RiffReader::descend_internal(FourCc type)
{
	bool has_type = (type != FourCc{});
	const bool is_root = chunks_.empty();
	const int header_size = 8 + (has_type ? 4 : 0);
	if (!is_root)
	{
		const ChunkInternal& chunk = chunks_.back();
		if (chunk.current_position + header_size > chunk.end_position)
		{
			return false;
		}
	}
	std::uint8_t buffer[12];
	if (stream_ptr_->read(buffer, header_size) != header_size)
	{
		return false;
	}
	if (!is_root)
	{
		ChunkInternal& chunk = chunks_.back();
		chunk.current_position += header_size;
	}
	FourCc chunk_id;
	if (!FourCc::from_octets_n(buffer, 4, chunk_id))
	{
		return false;
	}
	if (chunk_id == FourCc{} || (is_root && chunk_id != RiffFourCcs::riff))
	{
		return false;
	}
	if (!has_type && (chunk_id == RiffFourCcs::riff || chunk_id == RiffFourCcs::list))
	{
		has_type = true;
		if (stream_ptr_->read(buffer + 8, 4) != 4)
		{
			return false;
		}
	}
	std::uint32_t chunk_size = read_u32_le(buffer + 4);
	FourCc chunk_type{};
	if (has_type)
	{
		if (!FourCc::from_octets_n(buffer + 8, 4, chunk_type))
		{
			return false;
		}
		chunk_size -= 4;
	}
	const std::uint32_t adjusted_chunk_size = ((chunk_size + 1) / 2) * 2;
	const Stream::Position begin_position = stream_ptr_->get_position();
	Stream::Position end_position = begin_position + adjusted_chunk_size;
	bool is_adjusted = false;
	if (is_root)
	{
		const Stream::Position stream_size = stream_ptr_->get_size();
		if (stream_size < 0)
		{
			return false;
		}
		if (end_position > stream_size)
		{
			is_adjusted = true;
			end_position = stream_size;
		}
	}
	else
	{
		const ChunkInternal& chunk = chunks_.back();
		if (end_position > chunk.end_position)
		{
			is_adjusted = true;
			end_position = chunk.end_position;
		}
	}
	const Stream::Position data_size = is_adjusted ? end_position - begin_position : adjusted_chunk_size;
	if (data_size > 0xFFFFFFFFU)
	{
		return false;
	}
	Substream substream{stream_ptr_, begin_position, data_size};
	if (!substream.is_open())
	{
		return false;
	}
	ChunkInternal chunk{};
	chunk.begin_position = begin_position;
	chunk.current_position = begin_position;
	chunk.end_position = begin_position + data_size;
	Chunk& chunk_i = chunk.chunk;
	chunk_i.id = chunk_id;
	chunk_i.type = chunk_type;
	chunk_i.size = static_cast<std::uint32_t>(data_size);
	chunk_i.aligned_size = static_cast<std::uint32_t>(is_adjusted ? data_size : adjusted_chunk_size);
	chunk_i.data_stream = substream;
	chunks_.push_back(chunk);
	return true;
}

} // namespace ltjs
