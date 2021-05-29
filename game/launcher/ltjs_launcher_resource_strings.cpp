#include "ltjs_launcher_resource_strings.h"

#include <algorithm>
#include <deque>
#include <unordered_map>
#include <utility>
#include <vector>

#include "bibendovsky_spul_file_stream.h"
#include "bibendovsky_spul_path_utils.h"

#include "ltjs_script_tokenizer.h"


namespace ltjs
{
namespace launcher
{


namespace ul = bibendovsky::spul;


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// ResourceStrings::Impl
//

class ResourceStrings::Impl final
{
public:
	const std::string& impl_get_error_message() const;


	bool impl_initialize(
		const SearchPaths& search_paths,
		const std::string& file_name);

	void impl_uninitialize();

	bool impl_is_initialized() const;

	const std::string& impl_get(
		const int id,
		const std::string& default_string) const;


private:
	using Buffer = std::vector<char>;
	using IdToStringMap = std::unordered_map<int, std::string>;


	bool is_initialized_;
	std::string error_message_;
	IdToStringMap id_to_string_map_;
	Buffer file_buffer_;


	bool parse_file(
		ul::Stream& file_stream,
		IdToStringMap& string_map);
}; // ResourceStrings::Impl


const std::string& ResourceStrings::Impl::impl_get_error_message() const
{
	return error_message_;
}

bool ResourceStrings::Impl::impl_initialize(
	const SearchPaths& search_paths,
	const std::string& file_name)
{
	impl_uninitialize();

	if (file_name.empty())
	{
		error_message_ = "No file name.";
		return false;
	}

	for (const auto& search_path : search_paths)
	{
		const auto file_path = ul::PathUtils::normalize(
			ul::PathUtils::append(search_path, file_name));

		auto file_stream = ul::FileStream{file_path, ul::Stream::OpenMode::read};

		if (file_stream.is_open())
		{
			if (!parse_file(file_stream, id_to_string_map_))
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
	is_initialized_ = {};
	id_to_string_map_ = {};
}

bool ResourceStrings::Impl::impl_is_initialized() const
{
	return is_initialized_;
}

const std::string& ResourceStrings::Impl::impl_get(
	const int id,
	const std::string& default_string) const
{
	if (!is_initialized_)
	{
		return default_string;
	}

	const auto item_it = id_to_string_map_.find(id);

	if (item_it == id_to_string_map_.cend())
	{
		return default_string;
	}

	return item_it->second;
}

bool ResourceStrings::Impl::parse_file(
	ul::Stream& file_stream,
	IdToStringMap& id_to_string_map)
{
	// Load the file into memory.
	//
	if (!file_stream.is_open())
	{
		error_message_ = "Failed to open.";
		return false;
	}

	auto stream_size = file_stream.get_size();

	if (stream_size <= 0)
	{
		error_message_ = "Empty file.";
		return false;
	}

	if (stream_size > max_file_size)
	{
		error_message_ = "File too big.";
		return false;
	}

	const auto file_size = static_cast<int>(stream_size);

	file_buffer_.resize(file_size);

	const auto read_result = file_stream.read(file_buffer_.data(), file_size);

	if (read_result != file_size)
	{
		error_message_ = "Failed to read file.";
		return false;
	}


	// Tokenize strings.
	//
	auto script_tokenizer = ScriptTokenizer{};

	auto script_tokenizer_init_param = ScriptTokenizerInitParam{};
	script_tokenizer_init_param.data = file_buffer_.data();
	script_tokenizer_init_param.size = file_size;

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
	//

	ScriptTokenizerToken tokens[2];

	while (true)
	{
		auto script_line = ScriptTokenizerLine{};

		try
		{
			script_line = script_tokenizer.tokenize_line(
				tokens,
				2,
				2,
				2
			);
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

		auto id = 0;

		try
		{
			const auto id_string = std::string{script_line[0].data, static_cast<std::size_t>(script_line[0].size)};
			id = std::stoi(id_string);
		}
		catch (...)
		{
			error_message_ = "Expected a number at line " + std::to_string(script_line.get_line_number()) + ".";
			return false;
		}

		const auto& value_token = tokens[1];

		if (value_token.is_empty())
		{
			error_message_ = "Empty value " + std::to_string(script_line.get_line_number()) + ".";
			return false;
		}

		auto value_string = std::string{};

		if (value_token.is_escaped())
		{
			value_string.resize(value_token.unescaped_size);

			ScriptTokenizer::unescape_string(
				value_token,
				&value_string[0],
				value_token.unescaped_size
			);
		}
		else
		{
			value_string.assign(value_token.data, static_cast<std::size_t>(value_token.size));
		}

		id_to_string_map[id] = value_string;
	}

	return true;
}

//
// ResourceStrings::Impl
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// ResourceStrings
//

ResourceStrings::ResourceStrings()
	:
	impl_{std::make_unique<Impl>()}
{
}

ResourceStrings::ResourceStrings(
	ResourceStrings&& rhs)
	:
	impl_{std::move(rhs.impl_)}
{
}

ResourceStrings::~ResourceStrings() = default;

const std::string& ResourceStrings::get_error_message() const
{
	return impl_->impl_get_error_message();
}

bool ResourceStrings::initialize(
	const SearchPaths& search_path,
	const std::string& file_name)
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

const std::string& ResourceStrings::get(
	const int id) const
{
	static const auto empty_string = std::string{};

	return impl_->impl_get(id, empty_string);
}

const std::string& ResourceStrings::get(
	const int id,
	const std::string& default_string) const
{
	return impl_->impl_get(id, default_string);
}

const std::string& ResourceStrings::operator[](
	const int id) const
{
	return get(id);
}

//
// ResourceStrings
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // launcher
} // ltjs
