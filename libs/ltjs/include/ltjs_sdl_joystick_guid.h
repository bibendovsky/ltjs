#ifndef LTJS_SDL_JOYSTICK_GUID_INCLUDED
#define LTJS_SDL_JOYSTICK_GUID_INCLUDED


#include "SDL3/SDL_guid.h"


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

bool operator==(
	const SDL_GUID& lhs_joystick_guid,
	const SDL_GUID& rhs_joystick_guid) noexcept;

bool operator!=(
	const SDL_GUID& lhs_joystick_guid,
	const SDL_GUID& rhs_joystick_guid) noexcept;

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#endif // !LTJS_SDL_JOYSTICK_GUID_INCLUDED
