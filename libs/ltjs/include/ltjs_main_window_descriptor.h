/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Main window info

#ifndef LTJS_MAIN_WINDOW_DESCRIPTOR_INCLUDED
#define LTJS_MAIN_WINDOW_DESCRIPTOR_INCLUDED

#include "SDL3/SDL_video.h"

namespace ltjs {

struct MainWindowDescriptor
{
	SDL_Window* sdl_window;
	void* native_handle;
};

} // namespace ltjs

#endif // LTJS_MAIN_WINDOW_DESCRIPTOR_INCLUDED
