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


struct WaveformatUtils::Detail
{
	template<typename T>
	static bool generic_write(
		const T& format,
		StreamPtr stream_ptr)
	{
		if (!stream_ptr || !stream_ptr->is_writable())
		{
			return false;
		}

		if (Endian::is_little())
		{
			if (stream_ptr->write(&format, T::class_size) != T::class_size)
			{
				return false;
			}

			return true;
		}

		auto format_be = format;

		endian(format_be);

		if (stream_ptr->write(&format_be, T::class_size) != T::class_size)
		{
			return false;
		}

		return true;
	}
}; // WaveformatUtils::Detail


void WaveformatUtils::endian(
	WaveFormat& format)
{
	Endian::swap_i(format.tag_);
	Endian::swap_i(format.channel_count_);
	Endian::swap_i(format.sample_rate_);
	Endian::swap_i(format.avg_bytes_per_sec_);
	Endian::swap_i(format.block_align_);
}

void WaveformatUtils::endian(
	PcmWaveFormat& format)
{
	Endian::swap_i(format.tag_);
	Endian::swap_i(format.channel_count_);
	Endian::swap_i(format.sample_rate_);
	Endian::swap_i(format.avg_bytes_per_sec_);
	Endian::swap_i(format.block_align_);
	Endian::swap_i(format.bit_depth_);
}

void WaveformatUtils::endian(
	WaveFormatEx& format)
{
	Endian::swap_i(format.tag_);
	Endian::swap_i(format.channel_count_);
	Endian::swap_i(format.sample_rate_);
	Endian::swap_i(format.avg_bytes_per_sec_);
	Endian::swap_i(format.block_align_);
	Endian::swap_i(format.bit_depth_);
	Endian::swap_i(format.extra_size_);
}

bool WaveformatUtils::read(
	StreamPtr stream_ptr,
	WaveFormat& format)
{
	if (stream_ptr->read(&format, WaveFormat::class_size) != WaveFormat::class_size)
	{
		return false;
	}

	if (!Endian::is_little())
	{
		endian(format);
	}

	return true;
}

bool WaveformatUtils::read(
	StreamPtr stream_ptr,
	PcmWaveFormat& format)
{
	if (stream_ptr->read(&format, PcmWaveFormat::class_size) != PcmWaveFormat::class_size)
	{
		return false;
	}

	if (!Endian::is_little())
{
		endian(format);
	}

	return true;
}

bool WaveformatUtils::read(
	StreamPtr stream_ptr,
	WaveFormatEx& format)
{
	if (stream_ptr->read(&format, WaveFormatEx::class_size) != WaveFormatEx::class_size)
	{
		return false;
	}

	if (!Endian::is_little())
	{
		endian(format);
	}

	return true;
}

bool WaveformatUtils::write(
	const WaveFormat& format,
	StreamPtr stream_ptr)
{
	return Detail::generic_write<WaveFormat>(format, stream_ptr);
}

bool WaveformatUtils::write(
	const PcmWaveFormat& format,
	StreamPtr stream_ptr)
{
	return Detail::generic_write<PcmWaveFormat>(format, stream_ptr);
}

bool WaveformatUtils::write(
	const WaveFormatEx& format,
	StreamPtr stream_ptr)
{
	return Detail::generic_write<WaveFormatEx>(format, stream_ptr);
}


} // spul
} // bibendovsky
