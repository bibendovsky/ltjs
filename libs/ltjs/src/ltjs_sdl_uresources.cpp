#include "ltjs_sdl_uresources.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void SdlRwOpsUDeleter::operator()(
	::SDL_RWops* resource) const noexcept
{
	::SDL_RWclose(resource);
	::SDL_free(resource);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs
