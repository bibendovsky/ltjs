#ifndef LTJS_UTF32_DETAIL_INCLUDED
#define LTJS_UTF32_DETAIL_INCLUDED


#include <climits>


namespace ltjs
{
namespace utf32_detail
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

template<
	typename TCodeUnit
>
inline constexpr void ensure_cu_type() noexcept
{
	static_assert((sizeof(TCodeUnit) * CHAR_BIT) == 32, "Unsupported UTF-32 code unit type.");
};

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // utf32_detail
} // ltjs


#endif // !LTJS_UTF32_DETAIL_INCLUDED
