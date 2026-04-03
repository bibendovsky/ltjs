/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// WAVEFORMAT-family facility

#ifndef LTJS_WAVE_FORMAT_INCLUDED
#define LTJS_WAVE_FORMAT_INCLUDED

#include <cstdint>

namespace ltjs {

enum class WaveFormatTag : std::uint16_t
{
	unknown    = 0,
	pcm        = 0x0001,
	ima_adpcm  = 0x0011,
	mp3        = 0x0055,
	extensible = 0xFFFF
};

// =====================================

#pragma pack(push, 1)

struct WaveFormat
{
	inline static constexpr int io_size = 14;

	WaveFormatTag tag;
	std::uint16_t channel_count;
	std::uint32_t sample_rate;
	std::uint32_t avg_bytes_per_sec;
	std::uint16_t block_align;
};

struct PcmWaveFormat : public WaveFormat
{
	inline static constexpr int io_size = 16;

	std::uint16_t bit_depth;
};

struct WaveFormatEx : public PcmWaveFormat
{
	inline static constexpr int io_size = 18;

	std::uint16_t extra_size;
};

#pragma pack(pop)

// =====================================

bool operator==(const WaveFormat& a, const WaveFormat& b) noexcept;
bool operator==(const PcmWaveFormat& a, const PcmWaveFormat& b) noexcept;
bool operator==(const WaveFormatEx& a, const WaveFormatEx& b) noexcept;

bool operator!=(const WaveFormat& a, const WaveFormat& b) noexcept;
bool operator!=(const PcmWaveFormat& a, const PcmWaveFormat& b) noexcept;
bool operator!=(const WaveFormatEx& a, const WaveFormatEx& b) noexcept;

} // namespace ltjs

#endif // LTJS_WAVE_FORMAT_INCLUDED
