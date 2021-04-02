#ifndef LTJS_SDL_JOYSTICK_GUID_INCLUDED
#define LTJS_SDL_JOYSTICK_GUID_INCLUDED


#include "SDL.h"


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

bool operator==(
	const ::SDL_JoystickGUID& lhs_joystick_guid,
	const ::SDL_JoystickGUID& rhs_joystick_guid) noexcept;

bool operator!=(
	const ::SDL_JoystickGUID& lhs_joystick_guid,
	const ::SDL_JoystickGUID& rhs_joystick_guid) noexcept;

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#endif // !LTJS_SDL_JOYSTICK_GUID_INCLUDED
