/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// SDL utility

#ifndef LTJS_SDL_UTILITY_INCLUDED
#define LTJS_SDL_UTILITY_INCLUDED

#include "SDL3/SDL_video.h"

namespace ltjs::sdl {

char key_code_to_char(int key_code) noexcept;
void fill_window_black(SDL_Window* sdl_window);

} // namespace ltjs::sdl

#endif // LTJS_SDL_UTILITY_INCLUDED
