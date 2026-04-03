/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// WAVEFORMAT-family facility

#include "ltjs_wave_format.h"

namespace ltjs {

bool operator==(const WaveFormat& a, const WaveFormat& b) noexcept
{
	return
		a.tag == b.tag &&
		a.channel_count == b.channel_count &&
		a.sample_rate == b.sample_rate &&
		a.avg_bytes_per_sec == b.avg_bytes_per_sec &&
		a.block_align == b.block_align;
}

bool operator==(const PcmWaveFormat& a, const PcmWaveFormat& b) noexcept
{
	return
		static_cast<const WaveFormat&>(a) == static_cast<const WaveFormat&>(b) &&
		a.bit_depth == b.bit_depth;
}

bool operator==(const WaveFormatEx& a, const WaveFormatEx& b) noexcept
{
	return
		static_cast<const PcmWaveFormat&>(a) == static_cast<const PcmWaveFormat&>(b) &&
		a.extra_size == b.extra_size;
}

bool operator!=(const WaveFormat& a, const WaveFormat& b) noexcept
{
	return !(a == b);
}

bool operator!=(const PcmWaveFormat& a, const PcmWaveFormat& b) noexcept
{
	return !(a == b);
}

bool operator!=(const WaveFormatEx& a, const WaveFormatEx& b) noexcept
{
	return !(a == b);
}

} // namespace ltjs
