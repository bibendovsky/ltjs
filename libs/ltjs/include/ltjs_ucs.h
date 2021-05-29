#ifndef LTJS_UCS_INCLUDED
#define LTJS_UCS_INCLUDED


#include <cassert>
#include <cstdint>

#include "ltjs_index_type.h"

#include "ltjs_ucs_detail.h"
#include "ltjs_utf32_detail.h"
#include "ltjs_utf8_detail.h"


namespace ltjs
{
namespace ucs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using ucs_detail::CodePoint;

using ucs_detail::replacement_code_point;
using ucs_detail::max_code_point;

using ucs_detail::is_code_point_valid;

// ==========================================================================

template<
	typename TU8CodeUnit
>
inline constexpr Index get_utf8_to_utf32_size(
	const TU8CodeUnit* utf8_code_units,
	Index utf8_code_units_size) noexcept
{
	utf8_detail::ensure_cu_type<TU8CodeUnit>();

	assert(utf8_code_units_size >= 0);

	auto utf32_code_units_size = Index{};

	auto code_point = CodePoint{};

	while (true)
	{
		const auto decode_code_point_result = utf8_detail::decode_code_point(
			utf8_code_units,
			utf8_code_units_size,
			code_point
		);

		if (!decode_code_point_result)
		{
			return utf32_code_units_size;
		}

		utf32_code_units_size += 1;
	}
}

// ==========================================================================

struct Utf8ToUtf32Result
{
	Index utf8_used_size{};
	Index utf32_used_size{};
}; // Utf8ToUtf32Result

template<
	typename TU8CodeUnit,
	typename TU32CodeUnit
>
inline constexpr Utf8ToUtf32Result utf8_to_utf32(
	const TU8CodeUnit* utf8_code_units,
	Index utf8_code_units_size,
	TU32CodeUnit* utf32_code_units,
	Index utf32_code_units_size) noexcept
{
	utf8_detail::ensure_cu_type<TU8CodeUnit>();
	utf32_detail::ensure_cu_type<TU32CodeUnit>();

	assert(utf8_code_units);
	assert(utf32_code_units);

	auto result = Utf8ToUtf32Result{};
	result.utf8_used_size = utf8_code_units_size;
	result.utf32_used_size = 0;

	auto code_point = TU32CodeUnit{};

	while (true)
	{
		if (result.utf32_used_size >= utf32_code_units_size)
		{
			break;
		}

		const auto decode_code_point_result = utf8_detail::decode_code_point(
			utf8_code_units,
			utf8_code_units_size,
			code_point
		);

		if (!decode_code_point_result)
		{
			break;
		}

		(*utf32_code_units++) = code_point;
		result.utf32_used_size += 1;
	}

	result.utf8_used_size -= utf8_code_units_size;

	return result;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ucs
} // ltjs


#endif // !LTJS_UCS_INCLUDED
