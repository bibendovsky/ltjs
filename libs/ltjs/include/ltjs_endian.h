/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Endianness

#ifndef LTJS_ENDIAN_INCLUDED
#define LTJS_ENDIAN_INCLUDED

#include <cstdint>

namespace ltjs {

std::int16_t read_s16_le(const void* data) noexcept;
std::int32_t read_s32_le(const void* data) noexcept;
std::int64_t read_s64_le(const void* data) noexcept;

// -------------------------------------

std::uint16_t read_u16_le(const void* data) noexcept;
std::uint32_t read_u32_le(const void* data) noexcept;
std::uint64_t read_u64_le(const void* data) noexcept;

// -------------------------------------

float read_f32_le(const void* data) noexcept;
double read_f64_le(const void* data) noexcept;

// =====================================

void write_s16_le(std::int16_t value, void* buffer) noexcept;
void write_s32_le(std::int32_t value, void* buffer) noexcept;
void write_s64_le(std::int64_t value, void* buffer) noexcept;

// -------------------------------------

void write_u16_le(std::uint16_t value, void* buffer) noexcept;
void write_u32_le(std::uint32_t value, void* buffer) noexcept;
void write_u64_le(std::uint64_t value, void* buffer) noexcept;

// -------------------------------------

void write_f32_le(float value, void* buffer) noexcept;
void write_f64_le(double value, void* buffer) noexcept;

// =====================================

class BinaryReader
{
public:
	BinaryReader(const void* data, int data_size) noexcept;

	int get_data_left() const noexcept;
	bool can_read_n(int size) const noexcept;
	int get_position() const noexcept;
	const void* get_current_data() const noexcept;
	void skip(int size) noexcept;

	std::int8_t read_s8() noexcept;
	std::int16_t read_s16_le() noexcept;
	std::int32_t read_s32_le() noexcept;
	std::int64_t read_s64_le() noexcept;

	std::uint8_t read_u8() noexcept;
	std::uint16_t read_u16_le() noexcept;
	std::uint32_t read_u32_le() noexcept;
	std::uint64_t read_u64_le() noexcept;

	float read_f32_le() noexcept;
	double read_f64_le() noexcept;

private:
	const std::uint8_t* data_{};
	int data_size_{};
	int data_offset_{};
};

// =====================================

class BinaryWriter
{
public:
	BinaryWriter(void* data, int data_size) noexcept;

	bool can_write_n(int size) const noexcept;
	int get_position() const noexcept;
	void skip(int size) noexcept;

	void write_s8(std::int8_t value) noexcept;
	void write_s16_le(std::int16_t value) noexcept;
	void write_s32_le(std::int32_t value) noexcept;
	void write_s64_le(std::int64_t value) noexcept;

	void write_u8(std::uint8_t value) noexcept;
	void write_u16_le(std::uint16_t value) noexcept;
	void write_u32_le(std::uint32_t value) noexcept;
	void write_u64_le(std::uint64_t value) noexcept;

	void write_f32_le(float value) noexcept;
	void write_f64_le(double value) noexcept;

private:
	std::uint8_t* data_{};
	int data_size_{};
	int data_offset_{};
};

} // namespace ltjs

#endif // LTJS_ENDIAN_INCLUDED
