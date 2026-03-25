/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// SDL resource management

#include "ltjs_sdl_raii.h"
#include <cassert>

namespace ltjs::sdl {

void IoStreamDeleter::operator()(SDL_IOStream* resource) const noexcept
{
	[[maybe_unused]] const bool is_closed = SDL_CloseIO(resource);
	assert(is_closed);
}

} // namespace ltjs::sdl
