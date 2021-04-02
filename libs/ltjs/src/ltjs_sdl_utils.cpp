#include "ltjs_sdl_utils.h"

#include "ltjs_sdl_ensure_result.h"


namespace ltjs
{
namespace sdl_utils
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

char key_code_to_char(
	int key_code) noexcept
{
	const auto sdl_mod_state = ::SDL_GetModState();

	const auto bShiftDown = ((sdl_mod_state & ::KMOD_SHIFT) != 0);
	const auto bCapsLockOn = ((sdl_mod_state & ::KMOD_CAPS) != 0);
	const auto bNumLockOn = ((sdl_mod_state & ::KMOD_NUM) != 0);
	const auto bUpperCase = (bCapsLockOn && !bShiftDown) || (!bCapsLockOn && bShiftDown);

	switch (key_code)
	{
		case ::SDLK_a:
		case ::SDLK_b:
		case ::SDLK_c:
		case ::SDLK_d:
		case ::SDLK_e:
		case ::SDLK_f:
		case ::SDLK_g:
		case ::SDLK_h:
		case ::SDLK_i:
		case ::SDLK_j:
		case ::SDLK_k:
		case ::SDLK_l:
		case ::SDLK_m:
		case ::SDLK_n:
		case ::SDLK_o:
		case ::SDLK_p:
		case ::SDLK_q:
		case ::SDLK_r:
		case ::SDLK_s:
		case ::SDLK_t:
		case ::SDLK_u:
		case ::SDLK_v:
		case ::SDLK_w:
		case ::SDLK_x:
		case ::SDLK_y:
		case ::SDLK_z:
			(bUpperCase ? key_code -= 32 : 0);
			break;

		// we are not checking explicitly for some of
		// the values included in the ranges above since
		// because they are characters only derived with
		// a shift combination, they should not ever be
		// encountered
		case ::SDLK_0: (bShiftDown ? key_code = ')' : 0); break;
		case ::SDLK_1: (bShiftDown ? key_code = '!' : 0); break;
		case ::SDLK_2: (bShiftDown ? key_code = '@' : 0); break;
		case ::SDLK_3: (bShiftDown ? key_code = '#' : 0); break;
		case ::SDLK_4: (bShiftDown ? key_code = '$' : 0); break;
		case ::SDLK_5: (bShiftDown ? key_code = '%' : 0); break;
		case ::SDLK_6: (bShiftDown ? key_code = '^' : 0); break;
		case ::SDLK_7: (bShiftDown ? key_code = '&' : 0); break;
		case ::SDLK_8: (bShiftDown ? key_code = '*' : 0); break;
		case ::SDLK_9: (bShiftDown ? key_code = '(' : 0); break;

		case ::SDLK_COLON: (!bShiftDown ? key_code = ':' : 0); break;
		case ::SDLK_PLUS: (!bShiftDown ? key_code = '+' : 0); break;
		case ::SDLK_LESS: (!bShiftDown ? key_code = '<' : 0); break;
		case ::SDLK_UNDERSCORE: (!bShiftDown ? key_code = '_' : 0); break;
		case ::SDLK_GREATER: (!bShiftDown ? key_code = '>' : 0); break;
		case ::SDLK_QUESTION: (!bShiftDown ? key_code = '?' : 0); break;
		case ::SDLK_LEFTBRACKET: (!bShiftDown ? key_code = '{' : 0); break;
		case ::SDLK_BACKSLASH: (!bShiftDown ? key_code = '|' : 0); break;
		case ::SDLK_RIGHTBRACKET: (!bShiftDown ? key_code = '}' : 0); break;
		case ::SDLK_QUOTE: (!bShiftDown ? key_code = '"' : 0); break;

		case ::SDLK_KP_0: (bNumLockOn ? key_code = '0' : 0); break;
		case ::SDLK_KP_1: (bNumLockOn ? key_code = '1' : 0); break;
		case ::SDLK_KP_2: (bNumLockOn ? key_code = '2' : 0); break;
		case ::SDLK_KP_3: (bNumLockOn ? key_code = '3' : 0); break;
		case ::SDLK_KP_4: (bNumLockOn ? key_code = '4' : 0); break;
		case ::SDLK_KP_5: (bNumLockOn ? key_code = '5' : 0); break;
		case ::SDLK_KP_6: (bNumLockOn ? key_code = '6' : 0); break;
		case ::SDLK_KP_7: (bNumLockOn ? key_code = '7' : 0); break;
		case ::SDLK_KP_8: (bNumLockOn ? key_code = '8' : 0); break;
		case ::SDLK_KP_9: (bNumLockOn ? key_code = '9' : 0); break;

		case ::SDLK_KP_MULTIPLY: key_code = '*'; break;
		case ::SDLK_KP_PLUS: key_code = '+'; break;
		case ::SDLK_KP_MINUS: key_code = '-'; break;
		case ::SDLK_KP_DECIMAL: key_code = '.'; break;
		case ::SDLK_KP_DIVIDE: key_code = '/'; break;

		default:
			break;
	}

	return static_cast<char>(key_code);
}

void fill_window_black(
	::SDL_Window* sdl_window)
{
	const auto sdl_window_surface = sdl_ensure_result(::SDL_GetWindowSurface(sdl_window));

	if (sdl_window_surface)
	{
		const auto black_color = ::SDL_MapRGB(sdl_window_surface->format, 0, 0, 0);
		sdl_ensure_result(::SDL_FillRect(sdl_window_surface, nullptr, black_color));
		sdl_ensure_result(::SDL_UpdateWindowSurface(sdl_window));
	}
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // sdl_utils
} // ltjs
