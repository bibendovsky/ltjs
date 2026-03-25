/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// SDL subsystem

#ifndef LTJS_SDL_SUBSYSTEM_INCLUDED
#define LTJS_SDL_SUBSYSTEM_INCLUDED

#include <string_view>
#include "SDL3/SDL_init.h"

namespace ltjs::sdl {

class Subsystem
{
public:
	Subsystem() = default;
	explicit Subsystem(SDL_InitFlags sdl_init_flags);
	Subsystem(const Subsystem& rhs) = delete;
	Subsystem(Subsystem&& rhs) noexcept;
	Subsystem& operator=(const Subsystem& rhs) = delete;
	void operator=(Subsystem&& rhs) noexcept;
	~Subsystem();

	explicit operator bool() const noexcept;

private:
	SDL_InitFlags sdl_init_flags_{};

	[[noreturn]] static void fail(std::string_view message);
	void close() const noexcept;
};

} // namespace ltjs::sdl

#endif // LTJS_SDL_SUBSYSTEM_INCLUDED
