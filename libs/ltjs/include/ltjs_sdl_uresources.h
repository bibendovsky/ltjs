#ifndef LTJS_SDL_URESOURCES_INCLUDED
#define LTJS_SDL_URESOURCES_INCLUDED


#include <memory>

#include "SDL3/SDL.h"


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

using SdlWindowUDeleter = SdlUResourceDeleter<SDL_Window, SDL_DestroyWindow>;
using SdlWindowUResource = std::unique_ptr<SDL_Window, SdlWindowUDeleter>;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using SdlJoystickUDeleter = SdlUResourceDeleter<SDL_Joystick, SDL_CloseJoystick>;
using SdlJoystickUResource = std::unique_ptr<SDL_Joystick, SdlJoystickUDeleter>;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using SdlGamepadUDeleter = SdlUResourceDeleter<SDL_Gamepad, SDL_CloseGamepad>;
using SdlGamepadUResource = std::unique_ptr<SDL_Gamepad, SdlGamepadUDeleter>;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

struct SdlRwOpsUDeleter
{
	void operator()(
		SDL_IOStream* resource) const noexcept;
}; // SdlRwOpsUDeleter

using SdlRwOpsUResource = std::unique_ptr<SDL_IOStream, SdlRwOpsUDeleter>;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using SdlSurfaceUDeleter = SdlUResourceDeleter<SDL_Surface, SDL_DestroySurface>;
using SdlSurfaceUResource = std::unique_ptr<SDL_Surface, SdlSurfaceUDeleter>;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using SdlObjectUDeleter = SdlUResourceDeleter<SDL_SharedObject, SDL_UnloadObject>;
using SdlObjectUResource = std::unique_ptr<SDL_SharedObject, SdlObjectUDeleter>;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs


#endif // !LTJS_SDL_URESOURCES_INCLUDED
