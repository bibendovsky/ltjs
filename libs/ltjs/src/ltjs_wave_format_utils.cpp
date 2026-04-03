/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// WaveFormatEx utility

#include "ltjs_wave_format_utils.h"
#include "ltjs_endian.h"
#include <cassert>

namespace ltjs {

bool WaveformatUtils::write(const WaveFormat& format, Stream& out_stream) noexcept
{
	constexpr int octet_count = WaveFormat::io_size;
	unsigned char octets[octet_count];
	BinaryWriter binary_writer{octets, octet_count};
	assert(binary_writer.can_write_n(octet_count));
	binary_writer.write_u16_le(static_cast<std::uint16_t>(format.tag));
	binary_writer.write_u16_le(format.channel_count);
	binary_writer.write_u32_le(format.sample_rate);
	binary_writer.write_u32_le(format.avg_bytes_per_sec);
	binary_writer.write_u16_le(format.block_align);
	assert(binary_writer.get_position() == octet_count);
	if (out_stream.write(octets, octet_count) != octet_count)
	{
		return false;
	}
	return true;
}

bool WaveformatUtils::write(const PcmWaveFormat& format, Stream& out_stream) noexcept
{
	constexpr int octet_count = PcmWaveFormat::io_size;
	unsigned char octets[octet_count];
	BinaryWriter binary_writer{octets, octet_count};
	assert(binary_writer.can_write_n(octet_count));
	binary_writer.write_u16_le(static_cast<std::uint16_t>(format.tag));
	binary_writer.write_u16_le(format.channel_count);
	binary_writer.write_u32_le(format.sample_rate);
	binary_writer.write_u32_le(format.avg_bytes_per_sec);
	binary_writer.write_u16_le(format.block_align);
	binary_writer.write_u16_le(format.bit_depth);
	assert(binary_writer.get_position() == octet_count);
	if (out_stream.write(octets, octet_count) != octet_count)
	{
		return false;
	}
	return true;
}

bool WaveformatUtils::write(const WaveFormatEx& format, Stream& out_stream) noexcept
{
	constexpr int octet_count = WaveFormatEx::io_size;
	unsigned char octets[octet_count];
	BinaryWriter binary_writer{octets, octet_count};
	assert(binary_writer.can_write_n(octet_count));
	binary_writer.write_u16_le(static_cast<std::uint16_t>(format.tag));
	binary_writer.write_u16_le(format.channel_count);
	binary_writer.write_u32_le(format.sample_rate);
	binary_writer.write_u32_le(format.avg_bytes_per_sec);
	binary_writer.write_u16_le(format.block_align);
	binary_writer.write_u16_le(format.bit_depth);
	binary_writer.write_u16_le(format.extra_size);
	assert(binary_writer.get_position() == octet_count);
	if (out_stream.write(octets, octet_count) != octet_count)
	{
		return false;
	}
	return true;
}

} // namespace ltjs
