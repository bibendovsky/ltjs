#include "ltjs_sdl_joystick_guid.h"

#include <algorithm>
#include <array>


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

bool operator==(
	const ::SDL_JoystickGUID& lhs_joystick_guid,
	const ::SDL_JoystickGUID& rhs_joystick_guid) noexcept
{
	return std::equal(
		std::begin(lhs_joystick_guid.data),
		std::end(rhs_joystick_guid.data),
		std::begin(rhs_joystick_guid.data),
		[](
			const ::Uint8 lhs_byte,
			const ::Uint8 rhs_byte)
		{
			return lhs_byte == rhs_byte;
		}
	);
}

bool operator!=(
	const ::SDL_JoystickGUID& lhs_joystick_guid,
	const ::SDL_JoystickGUID& rhs_joystick_guid) noexcept
{
	return !(lhs_joystick_guid == rhs_joystick_guid);
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
