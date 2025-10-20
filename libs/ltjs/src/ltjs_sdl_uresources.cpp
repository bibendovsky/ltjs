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


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void SdlObjectUDeleter::operator()(
	SDL_SharedObject* resource) const noexcept
{
	SDL_UnloadObject(resource);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs
