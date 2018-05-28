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


#include "bibendovsky_spul_precompiled.h"
#include "bibendovsky_spul_wave_format.h"


namespace bibendovsky
{
namespace spul
{


bool operator==(
	const WaveFormat& lhs,
	const WaveFormat& rhs)
{
	return
		lhs.tag_ == rhs.tag_ &&
		lhs.channel_count_ == rhs.channel_count_ &&
		lhs.sample_rate_ == rhs.sample_rate_ &&
		lhs.avg_bytes_per_sec_ == rhs.avg_bytes_per_sec_ &&
		lhs.block_align_ == rhs.block_align_;
}

bool operator==(
	const PcmWaveFormat& lhs,
	const PcmWaveFormat& rhs)
{
	return
		static_cast<const WaveFormat&>(lhs) == static_cast<const WaveFormat&>(rhs) &&
		lhs.bit_depth_ == rhs.bit_depth_;
}

bool operator==(
	const WaveFormatEx& lhs,
	const WaveFormatEx& rhs)
{
	return
		static_cast<const PcmWaveFormat&>(lhs) == static_cast<const PcmWaveFormat&>(rhs) &&
		lhs.extra_size_ == rhs.extra_size_;
}

bool operator!=(
	const WaveFormat& lhs,
	const WaveFormat& rhs)
{
	return !(lhs == rhs);
}

bool operator!=(
	const PcmWaveFormat& lhs,
	const PcmWaveFormat& rhs)
{
	return !(lhs == rhs);
}

bool operator!=(
	const WaveFormatEx& lhs,
	const WaveFormatEx& rhs)
{
	return !(lhs == rhs);
}


} // spul
} // bibendovsky
