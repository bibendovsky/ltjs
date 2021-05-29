#ifndef LTJS_UCS_DETAIL_INCLUDED
#define LTJS_UCS_DETAIL_INCLUDED


#include <climits>

#include <type_traits>


namespace ltjs
{
namespace ucs_detail
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using CodePoint = unsigned int;

// --------------------------------------------------------------------------

constexpr auto replacement_code_point = CodePoint{0xFFFD};

constexpr auto min_code_point = CodePoint{};
constexpr auto max_code_point = CodePoint{0x10FFFF};

constexpr auto min_high_surrogate_code_point = CodePoint{0xD800};
constexpr auto max_high_surrogate_code_point = CodePoint{0xDBFF};

constexpr auto min_low_surrogate_code_point = CodePoint{0xDC00};
constexpr auto max_low_surrogate_code_point = CodePoint{0xDFFF};

// --------------------------------------------------------------------------

template<
	typename TCodePoint
>
inline constexpr void ensure_cp_type() noexcept
{
	static_assert(
		((sizeof(TCodePoint) * CHAR_BIT) - std::is_signed<TCodePoint>::value) >= 32,
		"Unsupported code point type."
	);
}; // ensure_cp_type

// --------------------------------------------------------------------------

template<
	typename TCodePoint
>
inline constexpr bool is_code_point_surrogate(
	TCodePoint code_point) noexcept
{
	return
		static_cast<CodePoint>(code_point) >= min_high_surrogate_code_point &&
		static_cast<CodePoint>(code_point) <= max_low_surrogate_code_point
	;
}

// --------------------------------------------------------------------------

template<
	typename TCodePoint
>
inline constexpr bool is_code_point_valid(
	TCodePoint code_point) noexcept
{
	return
		!is_code_point_surrogate(code_point) &&
		static_cast<CodePoint>(code_point) >= min_code_point &&
		static_cast<CodePoint>(code_point) <= max_code_point
	;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ucs_detail
} // ltjs


#endif // !LTJS_UCS_DETAIL_INCLUDED
