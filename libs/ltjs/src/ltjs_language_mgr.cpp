/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Language resource manager

#include "ltjs_language_mgr.h"
#include "ltjs_exception.h"
#include "ltjs_script_tokenizer.h"
#include "ltjs_sys_file_utility.h"
#include "ltjs_sys_fs_path.h"
#include <cassert>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <vector>

namespace ltjs {

namespace {

class LanguageMgrImpl final : public LanguageMgr
{
public:
	LanguageMgrImpl();
	~LanguageMgrImpl() override = default;

	void initialize(std::string_view base_path) noexcept override;
	const Language* get_current() const noexcept override;
	void set_current_by_id_string(std::string_view id_string) noexcept override;
	LanguageMgrLanguages get_languages() const noexcept override;
	void load() noexcept override;
	void save() noexcept override;

private:
	static constexpr int max_file_size = 4 * 1'024;
	static constexpr int min_language_capacity = 32;

	static constexpr const char* language_file_name = "language.txt";
	static constexpr const char* languages_file_name = "languages.txt";

	using Buffer = std::vector<char>;
	using ApiLanguages = std::vector<Language>;

	struct LanguageInternal
	{
		std::string id_string;
		std::string name;
	};

	using LanguagesInternal = std::vector<LanguageInternal>;

	sys::fs::Path base_path_{};
	const Language* api_current_language_{};
	Language api_default_language_{};
	ApiLanguages api_languages_{};

	const LanguageInternal* current_language_{};
	LanguageInternal default_language_{};
	LanguagesInternal languages_{};

	[[noreturn]] static void fail(std::string_view message);
	void set_default_language();
	void load_languages();
	void load_language();
	void make_api() noexcept;
};

// =====================================

LanguageMgrImpl::LanguageMgrImpl()
{
	default_language_.id_string = "en";
	default_language_.name = "ENGLISH";
	api_default_language_.id_string = default_language_.id_string;
	set_default_language();
}

void LanguageMgrImpl::initialize(std::string_view base_path) noexcept
{
	base_path_ = base_path;
	load();
}

const Language* LanguageMgrImpl::get_current() const noexcept
{
	return api_current_language_;
}

void LanguageMgrImpl::set_current_by_id_string(std::string_view id_string) noexcept
{
	set_default_language();
	if (id_string.empty())
	{
		return;
	}
	const auto language_iter_begin = languages_.cbegin();
	const auto language_iter_end = languages_.cend();
	const auto language_iter = std::find_if(
		language_iter_begin,
		language_iter_end,
		[id_string](const LanguageInternal& language_internal)
		{
			return std::equal(
				id_string.cbegin(),
				id_string.cend(),
				language_internal.id_string.cbegin(),
				language_internal.id_string.cend());
		});
	if (language_iter == language_iter_end)
	{
		return;
	}
	const std::intptr_t index = language_iter - language_iter_begin;
	current_language_ = &languages_[index];
	api_current_language_ = &api_languages_[index];
}

LanguageMgrLanguages LanguageMgrImpl::get_languages() const noexcept
{
	return LanguageMgrLanguages{api_languages_.data(), api_languages_.size()};
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
		[[maybe_unused]] const char* const what = ex.what();
		api_languages_ = {};
		languages_.clear();
		set_default_language();
	}
	make_api();
}

void LanguageMgrImpl::save() noexcept
try
{
	std::string string_buffer{};
	string_buffer.reserve(256);
	string_buffer += "/*\n";
	string_buffer += "LTJS\n";
	string_buffer += "SHARED\n";
	string_buffer += "Current language\n";
	string_buffer += "WARNING This is auto-generated file.\n";
	string_buffer += "*/\n\n";
	if (current_language_ != nullptr)
	{
		string_buffer += '\"';
		string_buffer += current_language_->id_string;
		string_buffer += "\"\n";
	}
	const sys::fs::Path file_path = base_path_ / language_file_name;
	sys::save_file(file_path.get_data(), string_buffer.data(), static_cast<int>(string_buffer.size()));
}
catch (const std::exception& ex)
{
	// FIXME Log?
	[[maybe_unused]] const char* const what = ex.what();
}

[[noreturn]] void LanguageMgrImpl::fail(std::string_view message)
{
	throw Exception{"LTJS_LANGUAGE_MGR", message};
}

void LanguageMgrImpl::set_default_language()
{
	current_language_ = &default_language_;
	api_current_language_ = &api_default_language_;
}

void LanguageMgrImpl::load_languages()
{
	const sys::fs::Path file_path = base_path_ / languages_file_name;
	Buffer buffer{};
	buffer.resize(max_file_size);
	const int loaded_size = sys::load_file(file_path.get_data(), buffer.data(), static_cast<int>(buffer.size()));
	ScriptTokenizer script_tokenizer{};
	const ScriptTokenizerInitParam script_tokenizer_init_param{
		.data = std::string_view{buffer.data(), static_cast<std::size_t>(loaded_size)}};
	script_tokenizer.initialize(script_tokenizer_init_param);
	ScriptTokenizerToken tokens[2];
	struct LanguageMapValue
	{
		int index;
		std::string value;
	};
	using LanguageMap = std::unordered_map<std::string, LanguageMapValue>;
	LanguageMap language_map{};
	language_map.reserve(min_language_capacity);
	int language_index = 0;
	for (;;)
	{
		const ScriptTokenizerLine script_line = script_tokenizer.tokenize_line(tokens, 2, 2, 2);
		if (script_line.is_end_of_data)
		{
			break;
		}
		if (script_line.is_empty())
		{
			continue;
		}
		const ScriptTokenizerToken& key_token = script_line[0];
		if (key_token.is_escaped())
		{
			fail("Escaped key.");
		}
		if (key_token.is_empty())
		{
			fail("Empty key.");
		}
		const ScriptTokenizerToken& value_token = script_line[1];
		if (value_token.is_escaped())
		{
			fail("Escaped value.");
		}
		if (value_token.is_empty())
		{
			fail("Empty value.");
		}
		const std::string key_string{key_token.data};
		std::string value_string{value_token.data};
		if (language_map.count(key_string) == 0)
		{
			LanguageMapValue& map_value = language_map.emplace(key_string, LanguageMapValue{}).first->second;
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
		LanguageInternal& language = languages_[language_map_item.second.index];
		language.id_string = language_map_item.first;
		language.name.swap(language_map_item.second.value);
	}
}

void LanguageMgrImpl::load_language()
{
	const sys::fs::Path file_path = base_path_ / language_file_name;
	Buffer buffer{};
	buffer.resize(max_file_size);
	const int loaded_size = sys::load_file(file_path.get_data(), buffer.data(), static_cast<int>(buffer.size()));
	ScriptTokenizer script_tokenizer{};
	const ScriptTokenizerInitParam script_tokenizer_init_param{
		.data = std::string_view{buffer.data(), static_cast<std::size_t>(loaded_size)}};
	script_tokenizer.initialize(script_tokenizer_init_param);
	ScriptTokenizerToken token;
	for (;;)
	{
		const ScriptTokenizerLine script_line = script_tokenizer.tokenize_line(&token, 1, 1, 1);
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
	std::string id_string{};
	if (!token.is_empty())
	{
		id_string = token.data;
	}
	const auto language_iter_end = languages_.cend();
	const auto language_iter = std::find_if(
		languages_.cbegin(),
		language_iter_end,
		[&id_string](const LanguageInternal& language)
		{
			return id_string == language.id_string;
		});
	if (language_iter != language_iter_end)
	{
		current_language_ = &(*language_iter);
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
	const std::size_t language_count = languages_.size();
	api_languages_.resize(language_count);
	for (std::size_t i = 0; i < language_count; ++i)
	{
		const LanguageInternal& language = languages_[i];
		Language& api_language = api_languages_[i];
		api_language.id_string = language.id_string;
		api_language.name = language.name;
	}
	if (current_language_)
	{
		const std::intptr_t index = current_language_ - languages_.data();
		api_current_language_ = &api_languages_[index];
	}
	else
	{
		set_default_language();
	}
}

} // namespace

// =====================================

LanguageMgrUPtr make_language_mgr()
{
	return std::make_unique<LanguageMgrImpl>();
}

} // namespace ltjs
