/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// SDL utility

#include "ltjs_sdl_utility.h"
#include <cassert>
#include <algorithm>
#include "SDL3/SDL_keyboard.h"
#include "SDL3/SDL_keycode.h"

namespace ltjs::sdl {

char key_code_to_char(int key_code) noexcept
{
	const SDL_Keymod sdl_mod_state = SDL_GetModState();
	const bool is_shift_down = ((sdl_mod_state & SDL_KMOD_SHIFT) != 0);
	const bool is_caps_lock_on = ((sdl_mod_state & SDL_KMOD_CAPS) != 0);
	const bool is_num_lock_on = ((sdl_mod_state & SDL_KMOD_NUM) != 0);
	const bool is_upper_case = (is_caps_lock_on && !is_shift_down) || (!is_caps_lock_on && is_shift_down);
	switch (key_code)
	{
		case SDLK_A:
		case SDLK_B:
		case SDLK_C:
		case SDLK_D:
		case SDLK_E:
		case SDLK_F:
		case SDLK_G:
		case SDLK_H:
		case SDLK_I:
		case SDLK_J:
		case SDLK_K:
		case SDLK_L:
		case SDLK_M:
		case SDLK_N:
		case SDLK_O:
		case SDLK_P:
		case SDLK_Q:
		case SDLK_R:
		case SDLK_S:
		case SDLK_T:
		case SDLK_U:
		case SDLK_V:
		case SDLK_W:
		case SDLK_X:
		case SDLK_Y:
		case SDLK_Z:
			(is_upper_case ? key_code -= 32 : 0);
			break;
		// we are not checking explicitly for some of
		// the values included in the ranges above since
		// because they are characters only derived with
		// a shift combination, they should not ever be
		// encountered
		case SDLK_0: (is_shift_down ? key_code = ')' : 0); break;
		case SDLK_1: (is_shift_down ? key_code = '!' : 0); break;
		case SDLK_2: (is_shift_down ? key_code = '@' : 0); break;
		case SDLK_3: (is_shift_down ? key_code = '#' : 0); break;
		case SDLK_4: (is_shift_down ? key_code = '$' : 0); break;
		case SDLK_5: (is_shift_down ? key_code = '%' : 0); break;
		case SDLK_6: (is_shift_down ? key_code = '^' : 0); break;
		case SDLK_7: (is_shift_down ? key_code = '&' : 0); break;
		case SDLK_8: (is_shift_down ? key_code = '*' : 0); break;
		case SDLK_9: (is_shift_down ? key_code = '(' : 0); break;
		//
		case SDLK_COLON: (!is_shift_down ? key_code = ':' : 0); break;
		case SDLK_PLUS: (!is_shift_down ? key_code = '+' : 0); break;
		case SDLK_LESS: (!is_shift_down ? key_code = '<' : 0); break;
		case SDLK_UNDERSCORE: (!is_shift_down ? key_code = '_' : 0); break;
		case SDLK_GREATER: (!is_shift_down ? key_code = '>' : 0); break;
		case SDLK_QUESTION: (!is_shift_down ? key_code = '?' : 0); break;
		case SDLK_LEFTBRACKET: (!is_shift_down ? key_code = '{' : 0); break;
		case SDLK_BACKSLASH: (!is_shift_down ? key_code = '|' : 0); break;
		case SDLK_RIGHTBRACKET: (!is_shift_down ? key_code = '}' : 0); break;
		case SDLK_APOSTROPHE: (!is_shift_down ? key_code = '"' : 0); break;
		//
		case SDLK_KP_0: (is_num_lock_on ? key_code = '0' : 0); break;
		case SDLK_KP_1: (is_num_lock_on ? key_code = '1' : 0); break;
		case SDLK_KP_2: (is_num_lock_on ? key_code = '2' : 0); break;
		case SDLK_KP_3: (is_num_lock_on ? key_code = '3' : 0); break;
		case SDLK_KP_4: (is_num_lock_on ? key_code = '4' : 0); break;
		case SDLK_KP_5: (is_num_lock_on ? key_code = '5' : 0); break;
		case SDLK_KP_6: (is_num_lock_on ? key_code = '6' : 0); break;
		case SDLK_KP_7: (is_num_lock_on ? key_code = '7' : 0); break;
		case SDLK_KP_8: (is_num_lock_on ? key_code = '8' : 0); break;
		case SDLK_KP_9: (is_num_lock_on ? key_code = '9' : 0); break;
		//
		case SDLK_KP_MULTIPLY: key_code = '*'; break;
		case SDLK_KP_PLUS: key_code = '+'; break;
		case SDLK_KP_MINUS: key_code = '-'; break;
		case SDLK_KP_DECIMAL: key_code = '.'; break;
		case SDLK_KP_DIVIDE: key_code = '/'; break;
		//
		default:
			break;
	}
	return static_cast<char>(key_code);
}

void fill_window_black(SDL_Window* sdl_window)
{
	if (SDL_Surface* const sdl_window_surface = SDL_GetWindowSurface(sdl_window);
		sdl_window_surface != nullptr)
	{
		const Uint32 black_color = SDL_MapSurfaceRGB(sdl_window_surface, 0, 0, 0);
		[[maybe_unused]] const bool is_filled = SDL_FillSurfaceRect(sdl_window_surface, nullptr, black_color);
		assert(is_filled && "SDL_FillSurfaceRect");
		[[maybe_unused]] const bool is_updated = SDL_UpdateWindowSurface(sdl_window);
		assert(is_updated && "SDL_UpdateWindowSurface");
	}
	else
	{
		assert(false && "SDL_GetWindowSurface");
	}
}

} // namespace ltjs::sdl
