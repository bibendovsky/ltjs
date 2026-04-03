/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Endianness

#include "ltjs_endian.h"
#include <cassert>
#include <bit>
#include <limits>

namespace ltjs {

namespace {

std::uint16_t impl_read_u16_le(const std::uint8_t* octets) noexcept
{
	return static_cast<std::uint16_t>(
		 static_cast<std::uint_fast16_t>(octets[0])       |
		(static_cast<std::uint_fast16_t>(octets[1]) << 8));
}

std::uint32_t impl_read_u32_le(const std::uint8_t* octets) noexcept
{
	return static_cast<std::uint32_t>(
		 static_cast<std::uint_fast32_t>(octets[0])        |
		(static_cast<std::uint_fast32_t>(octets[1]) <<  8) |
		(static_cast<std::uint_fast32_t>(octets[2]) << 16) |
		(static_cast<std::uint_fast32_t>(octets[3]) << 24));
}

std::uint64_t impl_read_u64_le(const std::uint8_t* octets) noexcept
{
	return static_cast<std::uint64_t>(
		 static_cast<std::uint_fast64_t>(octets[0])        |
		(static_cast<std::uint_fast64_t>(octets[1]) <<  8) |
		(static_cast<std::uint_fast64_t>(octets[2]) << 16) |
		(static_cast<std::uint_fast64_t>(octets[3]) << 24) |
		(static_cast<std::uint_fast64_t>(octets[4]) << 32) |
		(static_cast<std::uint_fast64_t>(octets[5]) << 40) |
		(static_cast<std::uint_fast64_t>(octets[6]) << 48) |
		(static_cast<std::uint_fast64_t>(octets[7]) << 56));
}

float impl_read_f32_le(const std::uint8_t* octets) noexcept
{
	return std::bit_cast<float>(impl_read_u32_le(octets));
}

double impl_read_f64_le(const std::uint8_t* octets) noexcept
{
	return std::bit_cast<double>(impl_read_u64_le(octets));
}

} // namespace

// -------------------------------------

std::int16_t read_s16_le(const void* data) noexcept
{
	return static_cast<std::int16_t>(read_u16_le(data));
}

std::int32_t read_s32_le(const void* data) noexcept
{
	return static_cast<std::int32_t>(read_u32_le(data));
}

std::int64_t read_s64_le(const void* data) noexcept
{
	return static_cast<std::int64_t>(read_u64_le(data));
}

// -------------------------------------

std::uint16_t read_u16_le(const void* data) noexcept
{
	return impl_read_u16_le(static_cast<const std::uint8_t*>(data));
}

std::uint32_t read_u32_le(const void* data) noexcept
{
	return impl_read_u32_le(static_cast<const std::uint8_t*>(data));
}

std::uint64_t read_u64_le(const void* data) noexcept
{
	return impl_read_u64_le(static_cast<const std::uint8_t*>(data));
}

// -------------------------------------

float read_f32_le(const void* data) noexcept
{
	return impl_read_f32_le(static_cast<const std::uint8_t*>(data));
}

double read_f64_le(const void* data) noexcept
{
	return impl_read_f64_le(static_cast<const std::uint8_t*>(data));
}

// =====================================

namespace {

void impl_write_u16_le(std::uint16_t value, std::uint8_t* octets) noexcept
{
	octets[0] = static_cast<std::uint8_t>( static_cast<std::uint_fast16_t>(value)       & 0xFF);
	octets[1] = static_cast<std::uint8_t>((static_cast<std::uint_fast16_t>(value) >> 8) & 0xFF);
}

void impl_write_u32_le(std::uint32_t value, std::uint8_t* octets) noexcept
{
	octets[0] = static_cast<std::uint8_t>( static_cast<std::uint_fast32_t>(value)        & 0xFF);
	octets[1] = static_cast<std::uint8_t>((static_cast<std::uint_fast32_t>(value) >>  8) & 0xFF);
	octets[2] = static_cast<std::uint8_t>((static_cast<std::uint_fast32_t>(value) >> 16) & 0xFF);
	octets[3] = static_cast<std::uint8_t>((static_cast<std::uint_fast32_t>(value) >> 24) & 0xFF);
}

void impl_write_u64_le(std::uint64_t value, std::uint8_t* octets) noexcept
{
	octets[0] = static_cast<std::uint8_t>( static_cast<std::uint_fast64_t>(value)        & 0xFF);
	octets[1] = static_cast<std::uint8_t>((static_cast<std::uint_fast64_t>(value) >>  8) & 0xFF);
	octets[2] = static_cast<std::uint8_t>((static_cast<std::uint_fast64_t>(value) >> 16) & 0xFF);
	octets[3] = static_cast<std::uint8_t>((static_cast<std::uint_fast64_t>(value) >> 24) & 0xFF);
	octets[4] = static_cast<std::uint8_t>((static_cast<std::uint_fast64_t>(value) >> 32) & 0xFF);
	octets[5] = static_cast<std::uint8_t>((static_cast<std::uint_fast64_t>(value) >> 40) & 0xFF);
	octets[6] = static_cast<std::uint8_t>((static_cast<std::uint_fast64_t>(value) >> 48) & 0xFF);
	octets[7] = static_cast<std::uint8_t>((static_cast<std::uint_fast64_t>(value) >> 56) & 0xFF);
}

void impl_write_f32_le(float value, std::uint8_t* octets) noexcept
{
	impl_write_u32_le(std::bit_cast<std::uint32_t>(value), octets);
}

void impl_write_f64_le(double value, std::uint8_t* octets) noexcept
{
	impl_write_u64_le(std::bit_cast<std::uint64_t>(value), octets);
}

} // namespace

// -------------------------------------

void write_s16_le(std::int16_t value, void* buffer) noexcept
{
	write_u16_le(static_cast<std::uint16_t>(value), buffer);
}

void write_s32_le(std::int32_t value, void* buffer) noexcept
{
	write_u32_le(static_cast<std::uint32_t>(value), buffer);
}

void write_s64_le(std::int64_t value, void* buffer) noexcept
{
	write_u64_le(static_cast<std::uint64_t>(value), buffer);
}

// -------------------------------------

void write_u16_le(std::uint16_t value, void* buffer) noexcept
{
	impl_write_u16_le(value, static_cast<std::uint8_t*>(buffer));
}

void write_u32_le(std::uint32_t value, void* buffer) noexcept
{
	impl_write_u32_le(value, static_cast<std::uint8_t*>(buffer));
}

void write_u64_le(std::uint64_t value, void* buffer) noexcept
{
	impl_write_u64_le(value, static_cast<std::uint8_t*>(buffer));
}

// -------------------------------------

void write_f32_le(float value, void* buffer) noexcept
{
	impl_write_f32_le(value, static_cast<std::uint8_t*>(buffer));
}

void write_f64_le(double value, void* buffer) noexcept
{
	impl_write_f64_le(value, static_cast<std::uint8_t*>(buffer));
}

// =====================================

BinaryReader::BinaryReader(const void* data, int data_size) noexcept
	:
	data_{static_cast<const std::uint8_t*>(data)},
	data_size_{data_size}
{
	assert(data != nullptr);
	assert(data_size >= 0);
}

int BinaryReader::get_data_left() const noexcept
{
	return data_size_ - data_offset_;
}

bool BinaryReader::can_read_n(int size) const noexcept
{
	assert(size >= 0);
	return get_data_left() >= size;
}

int BinaryReader::get_position() const noexcept
{
	return data_offset_;
}

const void* BinaryReader::get_current_data() const noexcept
{
	return data_ + data_offset_;
}

void BinaryReader::skip(int size) noexcept
{
	assert(can_read_n(size));
	data_offset_ += size;
}

std::int8_t BinaryReader::read_s8() noexcept
{
	return static_cast<std::int8_t>(read_u8());
}

std::int16_t BinaryReader::read_s16_le() noexcept
{
	return static_cast<std::int16_t>(read_u16_le());
}

std::int32_t BinaryReader::read_s32_le() noexcept
{
	return static_cast<std::int32_t>(read_u32_le());
}

std::int64_t BinaryReader::read_s64_le() noexcept
{
	return static_cast<std::int64_t>(read_u64_le());
}

std::uint8_t BinaryReader::read_u8() noexcept
{
	assert(can_read_n(1));
	return data_[data_offset_++];
}

std::uint16_t BinaryReader::read_u16_le() noexcept
{
	constexpr int n = 2;
	assert(can_read_n(n));
	const std::uint16_t value = impl_read_u16_le(data_ + data_offset_);
	data_offset_ += n;
	return value;
}

std::uint32_t BinaryReader::read_u32_le() noexcept
{
	constexpr int n = 4;
	assert(can_read_n(n));
	const std::uint32_t value = impl_read_u32_le(data_ + data_offset_);
	data_offset_ += n;
	return value;
}

std::uint64_t BinaryReader::read_u64_le() noexcept
{
	constexpr int n = 8;
	assert(can_read_n(n));
	const std::uint64_t value = impl_read_u64_le(data_ + data_offset_);
	data_offset_ += n;
	return value;
}

float BinaryReader::read_f32_le() noexcept
{
	constexpr int n = 4;
	assert(can_read_n(n));
	const float value = impl_read_f32_le(data_ + data_offset_);
	data_offset_ += n;
	return value;
}

double BinaryReader::read_f64_le() noexcept
{
	constexpr int n = 8;
	assert(can_read_n(n));
	const double value = impl_read_f64_le(data_ + data_offset_);
	data_offset_ += n;
	return value;
}

// =====================================

BinaryWriter::BinaryWriter(void* data, int data_size) noexcept
:
	data_{static_cast<std::uint8_t*>(data)},
	data_size_{data_size}
{
	assert(data != nullptr);
	assert(data_size >= 0);
}

bool BinaryWriter::can_write_n(int size) const noexcept
{
	assert(size >= 0);
	return data_size_ - data_offset_ >= size;
}

int BinaryWriter::get_position() const noexcept
{
	return data_offset_;
}

void BinaryWriter::skip(int size) noexcept
{
	assert(can_write_n(size));
	data_offset_ += size;
}

void BinaryWriter::write_s8(std::int8_t value) noexcept
{
	write_u8(static_cast<std::uint8_t>(value));
}

void BinaryWriter::write_s16_le(std::int16_t value) noexcept
{
	assert(can_write_n(2));
	write_u16_le(static_cast<std::uint16_t>(value));
}

void BinaryWriter::write_s32_le(std::int32_t value) noexcept
{
	assert(can_write_n(4));
	write_u32_le(static_cast<std::uint32_t>(value));
}

void BinaryWriter::write_s64_le(std::int64_t value) noexcept
{
	assert(can_write_n(8));
	write_u64_le(static_cast<std::uint64_t>(value));
}

void BinaryWriter::write_u8(std::uint8_t value) noexcept
{
	assert(can_write_n(1));
	data_[data_offset_++] = value;
}

void BinaryWriter::write_u16_le(std::uint16_t value) noexcept
{
	constexpr int n = 2;
	assert(can_write_n(n));
	impl_write_u16_le(value, data_ + data_offset_);
	data_offset_ += n;
}

void BinaryWriter::write_u32_le(std::uint32_t value) noexcept
{
	constexpr int n = 4;
	assert(can_write_n(n));
	impl_write_u32_le(value, data_ + data_offset_);
	data_offset_ += n;
}

void BinaryWriter::write_u64_le(std::uint64_t value) noexcept
{
	constexpr int n = 8;
	assert(can_write_n(n));
	impl_write_u64_le(value, data_ + data_offset_);
	data_offset_ += n;
}

void BinaryWriter::write_f32_le(float value) noexcept
{
	constexpr int n = 4;
	assert(can_write_n(n));
	impl_write_f32_le(value, data_ + data_offset_);
	data_offset_ += n;
}

void BinaryWriter::write_f64_le(double value) noexcept
{
	constexpr int n = 8;
	assert(can_write_n(n));
	impl_write_f64_le(value, data_ + data_offset_);
	data_offset_ += n;
}

} // namespace ltjs
