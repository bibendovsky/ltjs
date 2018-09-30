#include "ltjs_resource_strings.h"
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


namespace ul = bibendovsky::spul;


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// ResourceStrings::Impl
//

class ResourceStrings::Impl final
{
public:
	const std::string& impl_get_error_message() const;


	bool impl_initialize(
		const std::string& language_utf8,
		const std::string& directory_utf8,
		const std::string& file_name_utf8);

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
	const std::string& language_name_utf8,
	const std::string& directory_utf8,
	const std::string& file_name_utf8)
{
	impl_uninitialize();

	if (language_name_utf8.empty())
	{
		error_message_ = "No language name.";
		return false;
	}

	if (directory_utf8.empty())
	{
		error_message_ = "No directory.";
		return false;
	}

	if (file_name_utf8.empty())
	{
		error_message_ = "No file name.";
		return false;
	}

	// Shared.
	//
	auto shared_map = IdToStringMap{};

	const auto shared_file_name = ul::PathUtils::normalize(
		ul::PathUtils::append(directory_utf8, file_name_utf8));

	auto shared_file_stream = ul::FileStream{shared_file_name, ul::Stream::OpenMode::read};

	if (shared_file_stream.is_open())
	{
		if (!parse_file(shared_file_stream, shared_map))
		{
			error_message_ = shared_file_name + ": " + error_message_;
			return false;
		}
	}

	// Language-specific.
	//
	const auto language_file_name = ul::PathUtils::normalize(
		ul::PathUtils::append(ul::PathUtils::append(directory_utf8, language_name_utf8), file_name_utf8));

	auto language_map = IdToStringMap{};

	auto language_file_stream = ul::FileStream{language_file_name, ul::Stream::OpenMode::read};

	if (language_file_stream.is_open())
	{
		if (!parse_file(language_file_stream, language_map))
		{
			error_message_ = language_file_name + ": " + error_message_;
			return false;
		}
	}


	// Make one map.
	//
	if (!shared_map.empty() && !language_map.empty())
	{
		id_to_string_map_ = std::move(shared_map);

		for (const auto& language_item : language_map)
		{
			id_to_string_map_.insert_or_assign(language_item.first, language_item.second);
		}
	}
	else if (shared_map.empty() && !language_map.empty())
	{
		id_to_string_map_ = std::move(language_map);
	}
	else if (!shared_map.empty() && language_map.empty())
	{
		id_to_string_map_ = std::move(shared_map);
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
	id_to_string_map = {};

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

	if (!script_tokenizer.tokenize(
		file_buffer_.data(),
		file_size))
	{
		error_message_ = script_tokenizer.get_error_message();
		return false;
	}


	// Build internal maps.
	//
	using NameToIdMap = std::unordered_map<std::string, int>;
	using NameToStringMap = std::unordered_map<std::string, std::string>;

	auto name_to_id_map = NameToIdMap{};
	auto name_to_string_map = NameToStringMap{};

	for (const auto& script_line : script_tokenizer.get_lines())
	{
		const auto& tokens = script_line.tokens_;
		const auto token_count = tokens.size();

		// Identifiers.
		//
		if (tokens.front().content_ == "#define")
		{
			if (token_count != 3)
			{
				error_message_ = "Expected only three tokens at line " + std::to_string(script_line.number_) + ".";
				return false;
			}

			const auto is_already_added = name_to_id_map.find(tokens[1].content_) != name_to_id_map.cend();

			if (is_already_added)
			{
				error_message_ = "Id \"" + tokens[1].content_ +
					"\" already exists (line " + std::to_string(script_line.number_) + ").";

				return false;
			}

			auto id = 0;

			try
			{
				id = std::stoi(tokens[2].content_);
			}
			catch(...)
			{
				error_message_ = "Expected a number at line " + std::to_string(script_line.number_) + ".";
				return false;
			}

			name_to_id_map[tokens[1].content_] = id;
		}
		// Strings.
		//
		else
		{
			if (token_count != 2)
			{
				error_message_ = "Expected only two tokens at line " + std::to_string(script_line.number_) + ".";
				return false;
			}

			const auto is_already_added = name_to_string_map.find(tokens[0].content_) != name_to_string_map.cend();

			if (is_already_added)
			{
				error_message_ = "String \"" + tokens[1].content_ +
					"\" already exists (line " + std::to_string(script_line.number_) + ").";

				return false;
			}

			name_to_string_map[tokens[0].content_] = tokens[1].content_;
		}
	}

	// Build string to id map.
	//
	for (const auto& name_to_value_item : name_to_string_map)
	{
		const auto& name = name_to_value_item.first;

		const auto name_to_id_it = name_to_id_map.find(name);

		if (name_to_id_it == name_to_id_map.cend())
		{
			error_message_ = "No id for name \"" + name + "\".";
			return false;
		}

		id_to_string_map[name_to_id_it->second] = name_to_value_item.second;
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

ResourceStrings::~ResourceStrings()
{
}

const std::string& ResourceStrings::get_error_message() const
{
	return impl_->impl_get_error_message();
}

bool ResourceStrings::initialize(
	const std::string& language_name_utf8,
	const std::string& directory_utf8,
	const std::string& file_name_utf8)
{
	return impl_->impl_initialize(language_name_utf8, directory_utf8, file_name_utf8);
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


} // ltjs
