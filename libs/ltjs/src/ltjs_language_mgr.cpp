#include "ltjs_language_mgr.h"

#include <algorithm>
#include <unordered_map>
#include <string>
#include <vector>

#include "ltjs_c_string.h"
#include "ltjs_exception.h"
#include "ltjs_file.h"
#include "ltjs_file_system_path.h"
#include "ltjs_script_tokenizer.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

LanguageMgr::LanguageMgr() = default;

LanguageMgr::~LanguageMgr() = default;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class LanguageMgrImplException :
	public Exception
{
public:
	explicit LanguageMgrImplException(
		const char* message)
		:
		Exception{"LTJS_LANGUAGE_MGR", message}
	{
	}
}; // LanguageMgrImplException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class LanguageMgrImpl :
	public LanguageMgr
{
public:
	LanguageMgrImpl();


	void initialize(
		const char* base_path) noexcept override;


	const Language* get_current() const noexcept override;

	void set_current_by_id_string(
		const char* id_string) noexcept override;

	LanguageMgrLanguages get_languages() const noexcept override;


	void load() noexcept override;

	void save() noexcept override;


private:
	static constexpr auto max_file_size = 4 * 1'024;
	static constexpr auto min_language_capacity = 32;

	static constexpr auto language_file_name = "language.txt";
	static constexpr auto languages_file_name = "languages.txt";


	using Buffer = std::vector<char>;


	using ApiLanguages = std::vector<Language>;


	struct LanguageInternal
	{
		std::string id_string{};
		std::string name{};
	}; // LanguageInternal

	using LanguagesInternal = std::vector<LanguageInternal>;


	file_system::Path base_path_{};
	const Language* api_current_language_{};
	Language api_default_language_{};
	ApiLanguages api_languages_{};

	const LanguageInternal* current_language_{};
	LanguageInternal default_language_{};
	LanguagesInternal languages_{};


	void set_default_language();

	void load_languages();

	void load_language();

	void make_api() noexcept;
}; // LanguageMgrImpl

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

LanguageMgrImpl::LanguageMgrImpl()
{
	default_language_.id_string = "en";
	default_language_.name = "ENGLISH";

	api_default_language_.id_string.data = default_language_.id_string.data();
	api_default_language_.id_string.size = static_cast<Index>(default_language_.id_string.size());

	set_default_language();
}

void LanguageMgrImpl::initialize(
	const char* base_path) noexcept
{
	base_path_ = (base_path ? base_path : "");

	load();
}

const Language* LanguageMgrImpl::get_current() const noexcept
{
	return api_current_language_;
}

void LanguageMgrImpl::set_current_by_id_string(
	const char* id_string) noexcept
{
	set_default_language();

	if (!id_string)
	{
		return;
	}

	const auto id_string_size = c_string::get_size(id_string);

	const auto language_begin_it = languages_.cbegin();
	const auto language_end_it = languages_.cend();

	const auto language_it = std::find_if(
		language_begin_it,
		language_end_it,
		[id_string, id_string_size](
			const LanguageInternal& language_internal)
		{
			return std::equal(
				id_string,
				id_string + id_string_size,
				language_internal.id_string.cbegin(),
				language_internal.id_string.cend()
			);
		}
	);

	if (language_it == language_end_it)
	{
		return;
	}

	const auto index = language_it - language_begin_it;
	current_language_ = &languages_[index];
	api_current_language_ = &api_languages_[index];
}

LanguageMgrLanguages LanguageMgrImpl::get_languages() const noexcept
{
	return LanguageMgrLanguages{api_languages_.data(), static_cast<Index>(api_languages_.size())};
}

void LanguageMgrImpl::load() noexcept
{
	try
	{
		load_languages();
		load_language();
	}
	catch (const std::exception& ex)
	{
		const auto what = ex.what();
		static_cast<void>(what);

		api_languages_ = {};
		languages_.clear();

		set_default_language();
	}

	make_api();
}

void LanguageMgrImpl::save() noexcept
try
{
	auto string_buffer = std::string{};
	string_buffer.reserve(256);

	string_buffer += "/*\n";
	string_buffer += "LTJS\n";
	string_buffer += "SHARED\n";
	string_buffer += "Current language\n";
	string_buffer += "WARNING This is auto-generated file.\n";
	string_buffer += "*/\n";
	string_buffer += '\n';

	if (current_language_)
	{
		string_buffer += '\"';
		string_buffer += current_language_->id_string;
		string_buffer += "\"\n";
	}

	const auto file_path = base_path_ / language_file_name;

	file::save(
		file_path.get_data(),
		string_buffer.data(),
		static_cast<Index>(string_buffer.size())
	);
}
catch (const std::exception& ex)
{
	const auto what = ex.what();
	static_cast<void>(what);
}

void LanguageMgrImpl::set_default_language()
{
	current_language_ = &default_language_;
	api_current_language_ = &api_default_language_;
}

void LanguageMgrImpl::load_languages()
{
	const auto file_path = base_path_ / languages_file_name;

	auto buffer = Buffer{};
	buffer.resize(max_file_size);

	const auto loaded_size = file::load(
		file_path.get_data(),
		buffer.data(),
		static_cast<Index>(buffer.size())
	);

	auto script_tokenizer = ScriptTokenizer{};

	auto script_tokenizer_init_param = ScriptTokenizerInitParam{};
	script_tokenizer_init_param.data = buffer.data();
	script_tokenizer_init_param.size = loaded_size;

	script_tokenizer.initialize(script_tokenizer_init_param);

	ScriptTokenizerToken tokens[2];

	struct LanguageMapValue
	{
		Index index{};
		std::string value{};
	}; // LanguageMapValue

	using LanguageMap = std::unordered_map<std::string, LanguageMapValue>;

	auto language_map = LanguageMap{};
	language_map.reserve(min_language_capacity);

	auto language_index = Index{};

	while (true)
	{
		const auto script_line = script_tokenizer.tokenize_line(tokens, 2, 2, 2);

		if (script_line.is_end_of_data)
		{
			break;
		}

		if (script_line.is_empty())
		{
			continue;
		}

		const auto& key_token = script_line[0];

		if (key_token.is_escaped())
		{
			throw LanguageMgrImplException{"Escaped key."};
		}

		if (key_token.is_empty())
		{
			throw LanguageMgrImplException{"Empty key."};
		}

		const auto& value_token = script_line[1];

		if (value_token.is_escaped())
		{
			throw LanguageMgrImplException{"Escaped value."};
		}

		if (value_token.is_empty())
		{
			throw LanguageMgrImplException{"Empty value."};
		}

		const auto key_string = std::string{key_token.data, static_cast<std::size_t>(key_token.size)};
		auto value_string = std::string{value_token.data, static_cast<std::size_t>(value_token.size)};

		if (language_map.count(key_string) == 0)
		{
			auto& map_value = language_map.emplace(key_string, LanguageMapValue{}).first->second;
			map_value.index = language_index++;
			map_value.value.swap(value_string);
		}
		else
		{
			language_map[key_string].value.swap(value_string);
		}
	}

	languages_.resize(language_map.size());

	for (auto& language_map_item : language_map)
	{
		auto& language = languages_[language_map_item.second.index];
		language.id_string = language_map_item.first;
		language.name.swap(language_map_item.second.value);
	}
}

void LanguageMgrImpl::load_language()
{
	const auto file_path = base_path_ / language_file_name;

	auto buffer = Buffer{};
	buffer.resize(max_file_size);

	const auto loaded_size = file::load(
		file_path.get_data(),
		buffer.data(),
		static_cast<Index>(buffer.size())
	);

	auto script_tokenizer = ScriptTokenizer{};

	auto script_tokenizer_init_param = ScriptTokenizerInitParam{};
	script_tokenizer_init_param.data = buffer.data();
	script_tokenizer_init_param.size = loaded_size;

	script_tokenizer.initialize(script_tokenizer_init_param);

	ScriptTokenizerToken token;

	while (true)
	{
		const auto script_line = script_tokenizer.tokenize_line(&token, 1, 1, 1);

		if (script_line.is_end_of_data)
		{
			break;
		}

		if (script_line.is_empty())
		{
			continue;
		}

		break;
	}

	auto id_string = std::string{};

	if (!token.is_empty())
	{
		id_string.assign(token.data, static_cast<std::size_t>(token.size));
	}

	const auto language_end_it = languages_.cend();

	const auto language_it = std::find_if(
		languages_.cbegin(),
		language_end_it,
		[&id_string](
			const LanguageInternal& language)
		{
			return id_string == language.id_string;
		}
	);

	if (language_it != language_end_it)
	{
		current_language_ = &(*language_it);
	}
	else if (!languages_.empty())
	{
		current_language_ = &languages_.front();
	}
	else
	{
		current_language_ = nullptr;
	}
}

void LanguageMgrImpl::make_api() noexcept
{
	const auto language_count = languages_.size();

	api_languages_.resize(language_count);

	for (auto i = decltype(language_count){}; i < language_count; ++i)
	{
		const auto& language = languages_[i];
		auto& api_language = api_languages_[i];

		api_language.id_string.data = language.id_string.data();
		api_language.id_string.size = static_cast<Index>(language.id_string.size());

		api_language.name.data = language.name.data();
		api_language.name.size = static_cast<Index>(language.name.size());
	}

	if (current_language_)
	{
		const auto index = current_language_ - languages_.data();
		api_current_language_ = &api_languages_[index];
	}
	else
	{
		set_default_language();
	}
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

LanguageMgrUPtr make_language_mgr()
{
	return std::make_unique<LanguageMgrImpl>();
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs
