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
// A WAVEFORMAT-family structures.
//


#ifndef BIBENDOVSKY_SPUL_WAVE_FORMAT_INCLUDED
#define BIBENDOVSKY_SPUL_WAVE_FORMAT_INCLUDED


#include <cstdint>
#include "bibendovsky_spul_wave_format_tag.h"


namespace bibendovsky
{
namespace spul
{


#pragma pack(push, 1)


//
// A WAVEFORMAT structure.
//
struct WaveFormat
{
	static const auto packed_size = 14;


	WaveFormatTag tag_;
	std::uint16_t channel_count_;
	std::uint32_t sample_rate_;
	std::uint32_t avg_bytes_per_sec_;
	std::uint16_t block_align_;
}; // WaveFormat

using WaveFormatPtr = WaveFormat*;


//
// A PCMWAVEFORMAT structure.
//
struct PcmWaveFormat :
	public WaveFormat
{
	static const auto packed_size = 16;


	std::uint16_t bit_depth_;
}; // PcmWaveFormat

using PcmWaveFormatPtr = PcmWaveFormat*;


//
// A WAVEFORMATEX structure.
//
struct WaveFormatEx :
	public PcmWaveFormat
{
	static const auto packed_size = 18;


	std::uint16_t extra_size_;
}; // WaveFormatEx

using WaveFormatExPtr = WaveFormatEx*;


#pragma pack(pop)


} // spul
} // bibendovsky


#endif // !BIBENDOVSKY_SPUL_WAVE_FORMAT_INCLUDED
