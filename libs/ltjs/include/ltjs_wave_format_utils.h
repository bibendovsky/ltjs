/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// WaveFormatEx utility

#ifndef LTJS_WAVE_FORMAT_UTILS_INCLUDED
#define LTJS_WAVE_FORMAT_UTILS_INCLUDED

#include "ltjs_stream.h"
#include "ltjs_wave_format.h"

namespace ltjs {

class WaveformatUtils
{
public:
	static bool write(const WaveFormat& format, Stream& out_stream) noexcept;
	static bool write(const PcmWaveFormat& format, Stream& out_stream) noexcept;
	static bool write(const WaveFormatEx& format, Stream& out_stream) noexcept;
};

} // namespace ltjs

#endif // LTJS_WAVE_FORMAT_UTILS_INCLUDED
