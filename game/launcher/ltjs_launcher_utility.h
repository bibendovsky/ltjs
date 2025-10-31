#ifndef LTJS_LAUNCHER_UTILITY_INCLUDED
#define LTJS_LAUNCHER_UTILITY_INCLUDED

#include <memory>
#include <string>
#include <utility>
#include "SDL3/SDL.h"

namespace ltjs::launcher {

template<typename TDst, typename TSrc>
TDst narrow_cast(TSrc&& src)
{
	return static_cast<TDst>(std::forward<TSrc>(src));
}

// ======================================

struct SdlRawDeleter
{
	void operator()(void* pointer) const;
};

// --------------------------------------

template<typename TFunc>
struct GenericDeleterHelper;

template<typename TArg>
struct GenericDeleterHelper<void (*)(TArg*)>
{
	using Arg = TArg;
};

template<typename TArg>
struct GenericDeleterHelper<bool (*)(TArg*)>
{
	using Arg = TArg;
};

template<auto TFunc>
using GetGenericDeleterArg = typename GenericDeleterHelper<decltype(TFunc)>::Arg;

template<auto TFunc>
struct GenericDeleter
{
	using Arg = GetGenericDeleterArg<TFunc>;

	void operator()(Arg* pointer) const
	{
		TFunc(pointer);
	}
};

template<auto TFunc>
using GenericUPtr = std::unique_ptr<GetGenericDeleterArg<TFunc>, GenericDeleter<TFunc>>;

// ======================================

using SdlIoStreamUPtr = GenericUPtr<SDL_CloseIO>;
using SdlSharedObjectUPtr = GenericUPtr<SDL_UnloadObject>;
using SdlSurfaceUPtr = GenericUPtr<SDL_DestroySurface>;
using SdlTextureUPtr = GenericUPtr<SDL_DestroyTexture>;

// ======================================

constexpr inline const char native_file_path_separator =
#ifdef _WIN32
	'\\'
#else // _WIN32
	'/'
#endif // _WIN32
;

// ======================================

bool starts_with_or(const std::string& string, char char_1, char char_2);
bool ends_with_or(const std::string& string, char char_1, char char_2);

// ======================================

std::string normalize_file_path(const std::string& file_path);

// ======================================

std::string combine_file_paths(const std::string& a, const std::string& b);
std::string combine_file_paths(const std::string& a, const std::string& b, const std::string& c);

std::string combine_and_normalize_file_paths(const std::string& a, const std::string& b);
std::string combine_and_normalize_file_paths(const std::string& a, const std::string& b, const std::string& c);

} // namespace ltjs::launcher

#endif // LTJS_LAUNCHER_UTILITY_INCLUDED
