#include "ltjs_sdl_ensure_result.h"

#include "ltjs_sdl_exception.h"


namespace ltjs
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

[[noreturn]] void sdl_fail()
{
	throw SdlException{};
}

int sdl_ensure_result(
	int int_result)
{
	if (int_result == 0)
	{
		return int_result;
	}

	sdl_fail();
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // ltjs
