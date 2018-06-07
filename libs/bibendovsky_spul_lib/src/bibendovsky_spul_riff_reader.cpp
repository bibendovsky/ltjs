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


#include "bibendovsky_spul_precompiled.h"
#include "bibendovsky_spul_riff_reader.h"
#include <cstdint>
#include "bibendovsky_spul_endian.h"
#include "bibendovsky_spul_riff_four_ccs.h"


namespace bibendovsky
{
namespace spul
{


RiffReader::Chunk::Chunk()
	:
	id_{},
	type_{},
	size_{},
	aligned_size_{},
	data_stream_{}
{
}

bool RiffReader::Chunk::is_empty() const
{
	return id_ == 0 || size_ == 0;
}

RiffReader::RiffReader()
	:
	is_open_{},
	stream_ptr_{},
	chunks_{}
{
	chunks_.reserve(default_subchunk_count);
}

RiffReader::RiffReader(
	Stream* stream_ptr,
	const FourCc& type)
	:
	RiffReader{}
{
	static_cast<void>(open(stream_ptr, type));
}

bool RiffReader::open(
	Stream* stream_ptr,
	const FourCc& type)
{
	if (!stream_ptr ||
		!stream_ptr->is_open() ||
		!stream_ptr->is_readable() ||
		stream_ptr->is_writable() ||
		!stream_ptr->is_seekable() ||
		type == 0)
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

bool RiffReader::descend(
	const FourCc& type)
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

	const auto previous_chunk_aligned_size = chunks_.back().chunk_.aligned_size_;

	chunks_.pop_back();

	if (chunks_.empty())
	{
		if (!stream_ptr_->set_position(0))
		{
			return false;
		}

		return true;
	}

	auto& chunk = chunks_.back();
	chunk.current_position_ += previous_chunk_aligned_size;

	const auto set_result = stream_ptr_->set_position(chunk.current_position_);

	if (!set_result)
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


	auto& chunk = chunks_.back();
	chunk.current_position_ = chunk.begin_position_;

	const auto set_result = stream_ptr_->set_position(chunk.current_position_);

	if (!set_result)
	{
		return false;
	}

	return true;
}

bool RiffReader::find_and_descend(
	const FourCc id,
	const FourCc type)
{
	if (!is_open_ || id == 0 || chunks_.empty())
	{
		return false;
	}

	while (true)
	{
		if (!descend_internal(type))
		{
			return false;
		}

		const auto& chunk = chunks_.back();

		if (chunk.chunk_.id_ == id &&
			(type != 0 ? chunk.chunk_.type_ == type : true))
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

	return chunks_.back().chunk_;
}

bool RiffReader::descend_internal(
	const FourCc& type)
{
	auto has_type = (type != 0);
	const auto is_root = chunks_.empty();

	const auto header_size = 8 + (has_type ? 4 : 0);

	if (!is_root)
	{
		const auto& chunk = chunks_.back();

		if ((chunk.current_position_ + header_size) > chunk.end_position_)
		{
			return false;
		}
	}

	std::uint32_t buffer[3];

	const auto read_result = stream_ptr_->read(buffer, header_size);

	if (read_result != header_size)
	{
		return false;
	}

	if (!is_root)
	{
		auto& chunk = chunks_.back();
		chunk.current_position_ += header_size;
	}

	const auto chunk_id = Endian::little(buffer[0]);

	if (chunk_id == 0 || (is_root && chunk_id != RiffFourCcs::riff))
	{
		return false;
	}

	if (!has_type && (chunk_id == RiffFourCcs::riff || chunk_id == RiffFourCcs::list))
	{
		has_type = true;

		if (stream_ptr_->read(buffer + 2, 4) != 4)
		{
			return false;
		}
	}

	auto chunk_size = Endian::little(buffer[1]);
	const auto chunk_type = (has_type ? Endian::little(buffer[2]) : 0);


	if (has_type)
	{
		chunk_size -= 4;
	}

	const auto adjusted_chunk_size = ((chunk_size + 1) / 2) * 2;
	const auto begin_position = stream_ptr_->get_position();
	auto end_position = begin_position + adjusted_chunk_size;
	auto is_adjusted = false;

	if (is_root)
	{
		const auto stream_size = stream_ptr_->get_size();

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
		const auto& chunk = chunks_.back();

		if (end_position > chunk.end_position_)
		{
			is_adjusted = true;
			end_position = chunk.end_position_;
		}
	}

	const auto data_size = is_adjusted ? end_position - begin_position : adjusted_chunk_size;

	if (data_size > 0xFFFFFFFF)
	{
		return false;
	}

	auto substream = Substream{stream_ptr_, begin_position, data_size};

	if (!substream.is_open())
	{
		return false;
	}

	auto chunk = ChunkInternal{};
	chunk.begin_position_ = begin_position;
	chunk.current_position_ = begin_position;
	chunk.end_position_ = begin_position + data_size;

	auto& chunk_i = chunk.chunk_;
	chunk_i.id_ = FourCc{chunk_id};
	chunk_i.type_ = FourCc{chunk_type};
	chunk_i.size_ = static_cast<std::uint32_t>(data_size);
	chunk_i.aligned_size_ = static_cast<std::uint32_t>(is_adjusted ? data_size : adjusted_chunk_size);
	chunk_i.data_stream_ = substream;

	chunks_.push_back(chunk);

	return true;
}


} // spul
} // bibendovsky
