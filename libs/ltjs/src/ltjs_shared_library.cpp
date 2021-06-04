#include "ltjs_shared_library.h"

#include "ltjs_sdl_ensure_result.h"
#include "ltjs_sdl_uresources.h"


namespace ltjs
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

class SharedLibraryImpl :
	public SharedLibrary
{
public:
	SharedLibraryImpl(
		const char* path);


	void* find_symbol(
		const char* symbol_name) noexcept override;


private:
	SdlObjectUResource resource_{};
}; // SharedLibraryImpl

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

SharedLibraryImpl::SharedLibraryImpl(
	const char* path)
{
	resource_ = SdlObjectUResource{sdl_ensure_result(::SDL_LoadObject(path))};
}

void* SharedLibraryImpl::find_symbol(
	const char* symbol_name) noexcept
{
	return ::SDL_LoadFunction(resource_.get(), symbol_name);
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

SharedLibrary::SharedLibrary() noexcept = default;

SharedLibrary::~SharedLibrary() = default;

const char* SharedLibrary::get_default_suffix() noexcept
{
	return
#if _WIN32
		".dll"
#else
		".so"
#endif
	;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

SharedLibraryUPtr make_shared_library(
	const char* path)
{
	return std::make_unique<SharedLibraryImpl>(path);
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // ltjs
