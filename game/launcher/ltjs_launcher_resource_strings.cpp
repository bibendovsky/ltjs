#include "ltjs_launcher_resource_strings.h"
#include "ltjs_launcher_utility.h"
#include "ltjs_script_tokenizer.h"
#include <algorithm>
#include <charconv>
#include <deque>
#include <format>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ltjs::launcher {

class ResourceStrings::Impl
{
public:
	const std::string& impl_get_error_message() const;
	bool impl_initialize(const SearchPaths& search_paths, const std::string& file_name);
	void impl_uninitialize();
	bool impl_is_initialized() const;
	const std::string& impl_get(int id, const std::string& default_string) const;

private:
	using Buffer = std::vector<char>;
	using IdToStringMap = std::unordered_map<int, std::string>;

	bool is_initialized_;
	std::string error_message_;
	IdToStringMap id_to_string_map_;
	Buffer file_buffer_;

	bool parse_file(SDL_IOStream* file_stream, IdToStringMap& string_map);
};

// -------------------------------------

const std::string& ResourceStrings::Impl::impl_get_error_message() const
{
	return error_message_;
}

bool ResourceStrings::Impl::impl_initialize(const SearchPaths& search_paths, const std::string& file_name)
{
	impl_uninitialize();
	if (file_name.empty())
	{
		error_message_ = "No file name.";
		return false;
	}
	for (const std::string& search_path : search_paths)
	{
		const std::string file_path = combine_and_normalize_file_paths(search_path, file_name);
		if (SdlIoStreamUPtr file_stream{SDL_IOFromFile(file_path.c_str(), "rb")};
			file_stream != nullptr)
		{
			if (!parse_file(file_stream.get(), id_to_string_map_))
			{
				error_message_ = file_name + ": " + error_message_;
				return false;
			}
		}
	}
	is_initialized_ = true;
	return true;
}

void ResourceStrings::Impl::impl_uninitialize()
{
	is_initialized_ = false;
	id_to_string_map_.clear();
}

bool ResourceStrings::Impl::impl_is_initialized() const
{
	return is_initialized_;
}

const std::string& ResourceStrings::Impl::impl_get(int id, const std::string& default_string) const
{
	if (!is_initialized_)
	{
		return default_string;
	}
	if (const auto item_it = id_to_string_map_.find(id);
		item_it != id_to_string_map_.cend())
	{
		return item_it->second;
	}
	return default_string;
}

bool ResourceStrings::Impl::parse_file(SDL_IOStream* file_stream, IdToStringMap& id_to_string_map)
{
	// Load the file into memory.
	if (file_stream == nullptr)
	{
		error_message_ = "Failed to open.";
		return false;
	}
	const Sint64 stream_size = SDL_GetIOSize(file_stream);
	if (stream_size < min_file_size || stream_size > max_file_size)
	{
		error_message_ = std::format(
			"File size out of range. (size={}; min_size={}; max_size={})",
			stream_size,
			min_file_size,
			max_file_size);
		return false;
	}
	const int file_size = static_cast<int>(stream_size);
	file_buffer_.resize(file_size);
	const std::size_t read_result = SDL_ReadIO(file_stream, file_buffer_.data(), file_size);
	if (read_result != file_size)
	{
		error_message_ = "Failed to read file.";
		return false;
	}
	// Tokenize strings.
	ScriptTokenizer script_tokenizer{};
	const ScriptTokenizerInitParam script_tokenizer_init_param{.data = file_buffer_.data(), .size = file_size};
	try
	{
		script_tokenizer.initialize(script_tokenizer_init_param);
	}
	catch (const std::exception& ex)
	{
		error_message_ = ex.what();
		return false;
	}
	// Build internal maps.
	ScriptTokenizerToken tokens[2];
	while (true)
	{
		ScriptTokenizerLine script_line{};
		try
		{
			script_line = script_tokenizer.tokenize_line(tokens, 2, 2, 2);
		}
		catch (const std::exception& ex)
		{
			error_message_ = ex.what();
			return false;
		}
		if (!script_line)
		{
			break;
		}
		if (script_line.is_empty())
		{
			continue;
		}
		if (script_line.size != 2)
		{
			error_message_ = "Expected two tokens.";
			return false;
		}
		int id;
		if (const auto [chars_end, ec] = std::from_chars(script_line[0].data, script_line[0].data + script_line[0].size, id);
			ec != std::errc{})
		{
			error_message_ = std::format("Expected a number at line {}.", script_line.get_line_number());
			return false;
		}
		const ScriptTokenizerToken& value_token = tokens[1];
		if (value_token.is_empty())
		{
			error_message_ = std::format("Empty value {}.", script_line.get_line_number());
			return false;
		}
		std::string value_string{};
		if (value_token.is_escaped())
		{
			value_string.resize(value_token.unescaped_size);
			ScriptTokenizer::unescape_string(value_token, &value_string[0], value_token.unescaped_size);
		}
		else
		{
			value_string.assign(value_token.data, static_cast<std::size_t>(value_token.size));
		}
		id_to_string_map[id] = value_string;
	}
	return true;
}

// =====================================

ResourceStrings::ResourceStrings()
	:
	impl_{std::make_unique<Impl>()}
{}

ResourceStrings::~ResourceStrings() = default;

const std::string& ResourceStrings::get_error_message() const
{
	return impl_->impl_get_error_message();
}

bool ResourceStrings::initialize(const SearchPaths& search_path, const std::string& file_name)
{
	return impl_->impl_initialize(search_path, file_name);
}

void ResourceStrings::uninitialize()
{
	impl_->impl_uninitialize();
}

bool ResourceStrings::is_initialized() const
{
	return impl_->impl_is_initialized();
}

const std::string& ResourceStrings::get(int id) const
{
	constinit static const std::string empty_string{};
	return impl_->impl_get(id, empty_string);
}

const std::string& ResourceStrings::get(int id, const std::string& default_string) const
{
	return impl_->impl_get(id, default_string);
}

const std::string& ResourceStrings::operator[](int id) const
{
	return get(id);
}

} // namespace ltjs::launcher
