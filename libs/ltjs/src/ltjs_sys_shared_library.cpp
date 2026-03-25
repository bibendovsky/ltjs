/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// System: Shared library

#include "ltjs_sys_shared_library.h"
#include "ltjs_exception.h"
#include "SDL3/SDL_loadso.h"

namespace ltjs::sys {

namespace {

class SharedLibraryImpl final : public SharedLibrary
{
public:
	SharedLibraryImpl(const char* path);
	~SharedLibraryImpl() override;

	SharedLibrarySymbol find_symbol(const char* symbol_name) noexcept override;

private:
	SDL_SharedObject* sdl_shared_object_;
};

// =====================================

SharedLibraryImpl::SharedLibraryImpl(const char* path)
{
	sdl_shared_object_ = SDL_LoadObject(path);
	if (sdl_shared_object_ == nullptr)
	{
		throw Exception{"LTJS_SHARED_LIBRARY", SDL_GetError()};
	}
}

SharedLibraryImpl::~SharedLibraryImpl()
{
	SDL_UnloadObject(sdl_shared_object_);
}

SharedLibrarySymbol SharedLibraryImpl::find_symbol(const char* symbol_name) noexcept
{
	return SDL_LoadFunction(sdl_shared_object_, symbol_name);
}

} // namespace

// =====================================

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

// =====================================

SharedLibraryUPtr make_shared_library(const char* path)
{
	return std::make_unique<SharedLibraryImpl>(path);
}

} // namespace ltjs::sys
