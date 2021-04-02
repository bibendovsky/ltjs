#include "ltjs_sdl_exception.h"

#include "SDL.h"


namespace ltjs
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

SdlException::SdlException()
	:
	Exception{"LTJS_SDL", make_message()}
{
}

const char* SdlException::make_message()
{
	const auto sdl_error_message = ::SDL_GetError();

	return sdl_error_message ? sdl_error_message : "Generic error.";
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // ltjs
