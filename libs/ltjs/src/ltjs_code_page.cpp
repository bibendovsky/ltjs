#include "ltjs_code_page.h"


namespace ltjs
{
namespace code_page
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

Result::Result() noexcept
	:
	code_point_{-1}
{
}

Result::Result(
	CodePoint code_point) noexcept
	:
	code_point_{code_point}
{
}

Result::operator bool() const noexcept
{
	return code_point_ >= 0x00 && code_point_ <= 0xFF;
}

Result::operator char() const
{
	return static_cast<char>(static_cast<unsigned char>(code_point_));
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // code_page
} // ltjs
