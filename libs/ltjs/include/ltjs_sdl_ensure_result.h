#ifndef LTJS_SDL_ENSURE_RESULT_INCLUDED
#define LTJS_SDL_ENSURE_RESULT_INCLUDED


namespace ltjs
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

[[noreturn]] void sdl_fail();


int sdl_ensure_result(
	int int_result);

template<
	typename T
>
inline T* sdl_ensure_result(
	T* pointer_result)
{
	if (pointer_result)
	{
		return pointer_result;
	}

	sdl_fail();
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // ltjs


#endif // !LTJS_SDL_ENSURE_RESULT_INCLUDED
