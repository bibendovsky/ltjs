#include "ltjs_launcher_utility.h"
#include <algorithm>

namespace ltjs::launcher {

void SdlRawDeleter::operator()(void* pointer) const
{
	SDL_free(pointer);
}

// ======================================

SdlPropertiesIdURes::SdlPropertiesIdURes(SDL_PropertiesID sdl_properties_id)
	:
	sdl_properties_id_{sdl_properties_id}
{}

SdlPropertiesIdURes::SdlPropertiesIdURes(SdlPropertiesIdURes&& rhs) noexcept
	:
	sdl_properties_id_{rhs.sdl_properties_id_}
{
	rhs.sdl_properties_id_ = 0;
}

SdlPropertiesIdURes& SdlPropertiesIdURes::operator=(SdlPropertiesIdURes&& rhs) noexcept
{
	std::swap(sdl_properties_id_, rhs.sdl_properties_id_);
	return *this;
}

SdlPropertiesIdURes::~SdlPropertiesIdURes()
{
	if (sdl_properties_id_ != 0)
	{
		SDL_DestroyProperties(sdl_properties_id_);
	}
}

SDL_PropertiesID SdlPropertiesIdURes::get() const
{
	return sdl_properties_id_;
}

// ======================================

bool starts_with_or(const std::string& string, char char_1, char char_2)
{
	return !string.empty() && (string.front() == char_1 || string.front() == char_2);
}

bool ends_with_or(const std::string& string, char char_1, char char_2)
{
	return !string.empty() && (string.back() == char_1 || string.back() == char_2);
}

// ======================================

std::string normalize_file_path(const std::string& file_path)
{
	std::string dst_file_path{file_path};
	for (char& ch : dst_file_path)
	{
		if (ch == '/' || ch == '\\')
		{
			ch = native_file_path_separator;
		}
	}
	return dst_file_path;
}

// ======================================

std::string combine_file_paths(const std::string& a, const std::string& b)
{
	if (a.empty())
	{
		return b;
	}
	if (b.empty())
	{
		return a;
	}
	const bool lhs_ends_with_separator = ends_with_or(a, '/', '\\');
	const bool rhs_starts_with_separator = starts_with_or(b, '/', '\\');
	std::string file_path{};
	file_path.reserve(a.size() + b.size() + 1);
	if (!lhs_ends_with_separator && !rhs_starts_with_separator)
	{
		file_path += a;
		file_path += native_file_path_separator;
	}
	else if (lhs_ends_with_separator && rhs_starts_with_separator)
	{
		file_path.append(a.data(), a.size() - 1);
	}
	else
	{
		file_path += a;
	}
	file_path += b;
	return file_path;
}

std::string combine_file_paths(const std::string& a, const std::string& b, const std::string& c)
{
	return combine_file_paths(combine_file_paths(a, b), c);
}

std::string combine_and_normalize_file_paths(const std::string& a, const std::string& b)
{
	return normalize_file_path(combine_file_paths(a, b));
}

std::string combine_and_normalize_file_paths(const std::string& a, const std::string& b, const std::string& c)
{
	return normalize_file_path(combine_file_paths(a, b, c));
}

} // namespace ltjs::launcher
