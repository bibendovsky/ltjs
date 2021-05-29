#ifndef LTJS_SDL_URESOURCES_INCLUDED
#define LTJS_SDL_URESOURCES_INCLUDED


#include <memory>

#include "SDL.h"


namespace ltjs
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

template<
	typename TResource,
	void (TDeleter)(TResource*)
>
struct SdlUResourceDeleter
{
	void operator()(
		TResource* resource) const noexcept
	{
		TDeleter(resource);
	}
}; // SdlUResourceDeleter

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using SdlWindowUDeleter = SdlUResourceDeleter<::SDL_Window, ::SDL_DestroyWindow>;
using SdlWindowUResource = std::unique_ptr<::SDL_Window, SdlWindowUDeleter>;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using SdlJoystickUDeleter = SdlUResourceDeleter<::SDL_Joystick, ::SDL_JoystickClose>;
using SdlJoystickUResource = std::unique_ptr<::SDL_Joystick, SdlJoystickUDeleter>;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using SdlGameControllerUDeleter = SdlUResourceDeleter<::SDL_GameController, ::SDL_GameControllerClose>;
using SdlGameControllerUResource = std::unique_ptr<::SDL_GameController, SdlGameControllerUDeleter>;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

struct SdlRwOpsUDeleter
{
	void operator()(
		::SDL_RWops* resource) const noexcept;
}; // SdlRwOpsUDeleter

using SdlRwOpsUResource = std::unique_ptr<::SDL_RWops, SdlRwOpsUDeleter>;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using SdlSurfaceUDeleter = SdlUResourceDeleter<::SDL_Surface, ::SDL_FreeSurface>;
using SdlSurfaceUResource = std::unique_ptr<::SDL_Surface, SdlSurfaceUDeleter>;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs


#endif // !LTJS_SDL_URESOURCES_INCLUDED
