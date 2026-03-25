/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// SDL resource management

#ifndef LTJS_SDL_RAII_INCLUDED
#define LTJS_SDL_RAII_INCLUDED

#include <memory>
#include "SDL3/SDL.h"

namespace ltjs::sdl {

template<typename TResource, void (TDeleter)(TResource*)>
struct UPtrDeleter
{
	void operator()(TResource* resource) const noexcept
	{
		TDeleter(resource);
	}
};

// =====================================

using WindowDeleter = UPtrDeleter<SDL_Window, SDL_DestroyWindow>;
using WindowUPtr = std::unique_ptr<SDL_Window, WindowDeleter>;

// =====================================

using GamepadDeleter = UPtrDeleter<SDL_Gamepad, SDL_CloseGamepad>;
using GamepadUPtr = std::unique_ptr<SDL_Gamepad, GamepadDeleter>;

// =====================================

struct IoStreamDeleter
{
	void operator()(SDL_IOStream* resource) const noexcept;
};

using IoStreamUPtr = std::unique_ptr<SDL_IOStream, IoStreamDeleter>;

// =====================================

using SurfaceDeleter = UPtrDeleter<SDL_Surface, SDL_DestroySurface>;
using SurfaceUPtr = std::unique_ptr<SDL_Surface, SurfaceDeleter>;

} // namespace ltjs::sdl

#endif // LTJS_SDL_RAII_INCLUDED
