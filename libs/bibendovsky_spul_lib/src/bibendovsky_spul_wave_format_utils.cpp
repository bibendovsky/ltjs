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
// WaveFormatEx utils.
//


#include "bibendovsky_spul_precompiled.h"
#include "bibendovsky_spul_wave_format_utils.h"
#include "bibendovsky_spul_endian.h"


namespace bibendovsky
{
namespace spul
{


bool WaveformatUtils::read(
	StreamPtr stream_ptr,
	WaveFormat& format)
{
	if (stream_ptr->read(&format, WaveFormat::packed_size) != WaveFormat::packed_size)
	{
		return false;
	}

	if (!Endian::is_little())
	{
		Endian::swap_i(format.tag_);
		Endian::swap_i(format.channel_count_);
		Endian::swap_i(format.sample_rate_);
		Endian::swap_i(format.avg_bytes_per_sec_);
		Endian::swap_i(format.block_align_);
	}

	return true;
}

bool WaveformatUtils::read(
	StreamPtr stream_ptr,
	PcmWaveFormat& format)
{
	if (stream_ptr->read(&format, PcmWaveFormat::packed_size) != PcmWaveFormat::packed_size)
	{
		return false;
	}

	if (!Endian::is_little())
	{
		Endian::swap_i(format.tag_);
		Endian::swap_i(format.channel_count_);
		Endian::swap_i(format.sample_rate_);
		Endian::swap_i(format.avg_bytes_per_sec_);
		Endian::swap_i(format.block_align_);
		Endian::swap_i(format.bit_depth_);
	}

	return true;
}

bool WaveformatUtils::read(
	StreamPtr stream_ptr,
	WaveFormatEx& format)
{
	if (stream_ptr->read(&format, WaveFormatEx::packed_size) != WaveFormatEx::packed_size)
	{
		return false;
	}

	if (!Endian::is_little())
	{
		Endian::swap_i(format.tag_);
		Endian::swap_i(format.channel_count_);
		Endian::swap_i(format.sample_rate_);
		Endian::swap_i(format.avg_bytes_per_sec_);
		Endian::swap_i(format.block_align_);
		Endian::swap_i(format.bit_depth_);
		Endian::swap_i(format.extra_size_);
	}

	return true;
}


} // spul
} // bibendovsky
