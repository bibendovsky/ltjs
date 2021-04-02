#ifndef LTJS_SDL_UTILS_INCLUDED
#define LTJS_SDL_UTILS_INCLUDED


#include "SDL.h"

#include "ltjs_sdl_uresources.h"


namespace ltjs
{
namespace sdl_utils
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

char key_code_to_char(
	int key_code) noexcept;

void fill_window_black(
	::SDL_Window* sdl_window);

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // sdl_utils
} // ltjs


#endif // !LTJS_SDL_UTILS_INCLUDED
