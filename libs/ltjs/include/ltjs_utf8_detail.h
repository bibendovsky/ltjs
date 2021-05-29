#ifndef LTJS_UTF8_DETAIL_INCLUDED
#define LTJS_UTF8_DETAIL_INCLUDED


#include <cassert>
#include <climits>
#include <cstdint>

#include "ltjs_index_type.h"

#include "ltjs_ucs_detail.h"


namespace ltjs
{
namespace utf8_detail
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

constexpr auto min_byte_1 = 0x00;
constexpr auto max_byte_1 = 0x7F;

constexpr auto min_byte_2 = 0xC0;
constexpr auto max_byte_2 = 0xDF;

constexpr auto min_byte_3 = 0xE0;
constexpr auto max_byte_3 = 0xEF;

constexpr auto min_byte_4 = 0xF0;
constexpr auto max_byte_4 = 0xF7;

constexpr auto min_byte_n = 0x80;
constexpr auto max_byte_n = 0xBF;

constexpr auto byte_n_mask = 0B00111111;

constexpr auto byte_1_min_range = 0x000000;
constexpr auto byte_1_max_range = 0x00007F;

constexpr auto byte_2_min_range = 0x000080;
constexpr auto byte_2_max_range = 0x0007FF;

constexpr auto byte_3_min_range = 0x000800;
constexpr auto byte_3_max_range = 0x00FFFF;

constexpr auto byte_4_min_range = 0x010000;

// --------------------------------------------------------------------------

template<
	typename TCodeUnit
>
inline constexpr void ensure_cu_type() noexcept
{
	static_assert((sizeof(TCodeUnit) * CHAR_BIT) == 8, "Unsupported UTF-8 code unit type.");
};

// --------------------------------------------------------------------------

// Code unit to uint8.
template<
	typename TCodeUnit
>
inline constexpr std::uint8_t cu_to_u8(
	TCodeUnit code_unit) noexcept
{
	ensure_cu_type<TCodeUnit>();

	return static_cast<std::uint8_t>(code_unit);
}

// Code point to code unit.
template<
	typename TCodeUnit,
	typename TCodePoint
>
inline constexpr TCodeUnit cp_to_cu(
	TCodePoint code_point) noexcept
{
	ensure_cu_type<TCodeUnit>();
	ucs_detail::ensure_cp_type<TCodePoint>();

	return static_cast<TCodeUnit>(static_cast<std::uint8_t>(code_point));
}

// --------------------------------------------------------------------------

template<
	typename TCodeUnit,
	typename TCodePoint
>
inline constexpr bool decode_code_point(
	const TCodeUnit*& code_units,
	Index& code_units_size,
	TCodePoint& code_point) noexcept
{
	ensure_cu_type<TCodeUnit>();
	ucs_detail::ensure_cp_type<TCodePoint>();

	assert(code_units);

	if (code_units_size <= 0)
	{
		return false;
	}

	const auto byte_1 = cu_to_u8(code_units[0]);

	if (byte_1 <= max_byte_1)
	{
		code_point = byte_1;

		code_units += 1;
		code_units_size -= 1;

		return true;
	}
	else if (byte_1 >= min_byte_2 && byte_1 <= max_byte_2)
	{
		if (code_units_size >= 2)
		{
			const auto byte_2 = cu_to_u8(code_units[1]);

			if (byte_2 >= min_byte_n && byte_2 <= max_byte_n)
			{
				code_point =
					((byte_1 - min_byte_2) << 6) |
					(byte_2 - min_byte_n)
				;

				if (code_point >= byte_2_min_range && code_point <= byte_2_max_range)
				{
					code_units += 2;
					code_units_size -= 2;

					return true;
				}
			}
		}
	}
	else if (byte_1 >= min_byte_3 && byte_1 <= max_byte_3)
	{
		if (code_units_size >= 3)
		{
			const auto byte_2 = cu_to_u8(code_units[1]);

			if (byte_2 >= min_byte_n && byte_2 <= max_byte_n)
			{
				const auto byte_3 = cu_to_u8(code_units[2]);

				if (byte_3 >= min_byte_n && byte_3 <= max_byte_n)
				{
					code_point =
						((byte_1 - min_byte_3) << 12) |
						((byte_2 - min_byte_n) << 6) |
						(byte_3 - min_byte_n)
					;

					if (code_point >= byte_3_min_range && code_point <= byte_3_max_range &&
						!ucs_detail::is_code_point_surrogate(code_point))
					{
						code_units += 3;
						code_units_size -= 3;

						return true;
					}
				}
			}
		}
	}
	else if (byte_1 >= min_byte_4 && byte_1 <= max_byte_4)
	{
		if (code_units_size >= 4)
		{
			const auto byte_2 = cu_to_u8(code_units[1]);

			if (byte_2 >= min_byte_n && byte_2 <= max_byte_n)
			{
				const auto byte_3 = cu_to_u8(code_units[2]);

				if (byte_3 >= min_byte_n && byte_3 <= max_byte_n)
				{
					const auto byte_4 = cu_to_u8(code_units[3]);

					if (byte_4 >= min_byte_n && byte_4 <= max_byte_n)
					{
						code_point =
							((byte_1 - min_byte_4) << 18) |
							((byte_2 - min_byte_n) << 12) |
							((byte_3 - min_byte_n) << 6) |
							(byte_4 - min_byte_n)
						;

						if (code_point >= byte_4_min_range)
						{
							code_units += 4;
							code_units_size -= 4;

							return true;
						}
					}
				}
			}
		}
	}

	// Not enough code units, overlong encoding, truncated sequence, invalid sequence.

	code_point = static_cast<TCodePoint>(ucs_detail::replacement_code_point);

	code_units += 1;
	code_units_size -= 1;

	return true;
}

// --------------------------------------------------------------------------

template<
	typename TCodeUnit,
	typename TCodePoint
>
inline constexpr bool encode_code_point(
	TCodePoint code_point,
	TCodeUnit*& code_units,
	Index& code_units_size) noexcept
{
	ensure_cu_type<TCodeUnit>();
	ucs_detail::ensure_cp_type<TCodePoint>();

	assert(code_units);

	if (code_units_size <= 0)
	{
		return false;
	}

	if (code_point >= byte_1_min_range && code_point <= byte_1_max_range)
	{
		(*code_units++) = cp_to_cu<TCodeUnit>(code_point);

		code_units_size -= 1;

		return true;
	}
	else if (code_point >= byte_2_min_range && code_point <= byte_2_max_range)
	{
		if (code_units_size >= 2)
		{
			(*code_units++) = cp_to_cu<TCodeUnit>(min_byte_2 | (code_point >> 6));
			(*code_units++) = cp_to_cu<TCodeUnit>(min_byte_n | (code_point & byte_n_mask));

			code_units_size -= 2;

			return true;
		}
	}
	else if (code_point >= byte_3_min_range && code_point <= byte_3_max_range)
	{
		if (code_units_size >= 3 &&
			!ucs_detail::is_code_point_surrogate(code_point))
		{
			(*code_units++) = cp_to_cu<TCodeUnit>(min_byte_3 | (code_point >> 12));
			(*code_units++) = cp_to_cu<TCodeUnit>(min_byte_n | ((code_point >> 6) & byte_n_mask));
			(*code_units++) = cp_to_cu<TCodeUnit>(min_byte_n | (code_point & byte_n_mask));

			code_units_size -= 3;

			return true;
		}
	}
	else if (code_point >= byte_4_min_range &&
		code_point <= static_cast<TCodePoint>(ucs_detail::max_code_point))
	{
		if (code_units_size >= 4)
		{
			(*code_units++) = cp_to_cu<TCodeUnit>(min_byte_4 | (code_point >> 18));
			(*code_units++) = cp_to_cu<TCodeUnit>(min_byte_n | ((code_point >> 12) & byte_n_mask));
			(*code_units++) = cp_to_cu<TCodeUnit>(min_byte_n | ((code_point >> 6) & byte_n_mask));
			(*code_units++) = cp_to_cu<TCodeUnit>(min_byte_n | (code_point & byte_n_mask));

			code_units_size -= 4;

			return true;
		}
	}
	else
	{
		assert(!"Code point out of range.");
	}

	// Not enough destination code units.

	return false;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // utf8_detail
} // ltjs


#endif // !LTJS_UTF8_DETAIL_INCLUDED
