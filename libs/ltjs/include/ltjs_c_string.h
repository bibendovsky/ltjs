#ifndef LTJS_C_STRING_INCLUDED
#define LTJS_C_STRING_INCLUDED


#include <cassert>

#include "ltjs_index_type.h"


namespace ltjs
{
namespace c_string
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

template<
	typename TChar
>
inline constexpr Index get_size(
	const TChar* string) noexcept
{
	assert(string);

	auto string_size = Index{};

	while (string[string_size] != TChar{})
	{
		string_size += 1;
	}

	return string_size;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // c_string
} // ltjs


#endif // !LTJS_C_STRING_INCLUDED

