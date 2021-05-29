#ifndef LTJS_CHAR_CONV_INCLUDED
#define LTJS_CHAR_CONV_INCLUDED


#include <cstdint>

#include "ltjs_index_type.h"


namespace ltjs
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

enum ToCharsFormat :
	unsigned int
{
	to_chars_format_default = 0,

	to_chars_format_uppercase = 1U << 0,
}; // ToCharsFormat


Index to_chars(
	std::int32_t value,
	int base,
	const ToCharsFormat& format,
	char* dst_buffer,
	Index dst_buffer_size) noexcept;

Index to_chars(
	std::uint32_t value,
	int base,
	const ToCharsFormat& format,
	char* dst_buffer,
	Index dst_buffer_size) noexcept;

Index to_chars(
	std::uint64_t value,
	int base,
	const ToCharsFormat& format,
	char* dst_buffer,
	Index dst_buffer_size) noexcept;

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

bool from_chars(
	const char* string,
	Index string_size,
	int base,
	std::int32_t& value) noexcept;

bool from_chars(
	const char* string,
	Index string_size,
	int base,
	std::uint32_t& value) noexcept;

bool from_chars(
	const char* string,
	Index string_size,
	int base,
	std::uint64_t& value) noexcept;

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // ltjs


#endif // !LTJS_CHAR_CONV_INCLUDED

