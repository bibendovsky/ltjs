#ifndef LTJS_MAIN_WINDOW_DESCRIPTOR_INCLUDED
#define LTJS_MAIN_WINDOW_DESCRIPTOR_INCLUDED


#include "SDL.h"


namespace ltjs
{


struct MainWindowDescriptor
{
	::SDL_Window* sdl_window{};
	void* native_handle{};
}; // MainWindowDescriptor


} // ltjs


#endif // !LTJS_MAIN_WINDOW_DESCRIPTOR_INCLUDED
