#include "ltjs_sdl_uresources.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void SdlRwOpsUDeleter::operator()(
	SDL_IOStream* resource) const noexcept
{
	SDL_CloseIO(resource);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs
