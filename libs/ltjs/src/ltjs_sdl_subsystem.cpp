/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// SDL subsystem

#include "ltjs_sdl_subsystem.h"
#include "ltjs_exception.h"
#include <algorithm>
#include <string>

namespace ltjs::sdl {

Subsystem::Subsystem(SDL_InitFlags sdl_init_flags)
{
	switch (sdl_init_flags)
	{
		case SDL_INIT_EVENTS:
		case SDL_INIT_GAMEPAD:
		case SDL_INIT_JOYSTICK:
		case SDL_INIT_VIDEO:
			break;
		default:
			fail("Unsupported SDL subsystem.");
	}
	if (!SDL_InitSubSystem(sdl_init_flags))
	{
		std::string message{};
		message.reserve(256);
		message += "[SDL_InitSubSystem] ";
		message += SDL_GetError();
		fail(message);
	}
	sdl_init_flags_ = sdl_init_flags;
}

Subsystem::Subsystem(Subsystem&& rhs) noexcept
	:
	sdl_init_flags_{rhs.sdl_init_flags_}
{
	rhs.sdl_init_flags_ = 0;
}

void Subsystem::operator=(Subsystem&& rhs) noexcept
{
	close();
	std::swap(sdl_init_flags_, rhs.sdl_init_flags_);
}

Subsystem::~Subsystem()
{
	close();
}

Subsystem::operator bool() const noexcept
{
	return sdl_init_flags_ != 0;
}

[[noreturn]] void Subsystem::fail(std::string_view message)
{
	throw Exception{"LTJS_SDL_SUBSYSTEM", message};
}

void Subsystem::close() const noexcept
{
	if (sdl_init_flags_ != 0)
	{
		SDL_QuitSubSystem(sdl_init_flags_);
	}
}

} // namespace ltjs::sdl
