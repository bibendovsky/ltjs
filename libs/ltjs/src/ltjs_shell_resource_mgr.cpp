/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Shell resource manager

#include "ltjs_shell_resource_mgr.h"
#include "ltjs_ascii.h"
#include "ltjs_code_page.h"
#include "ltjs_exception.h"
#include "ltjs_script_tokenizer.h"
#include "ltjs_sys_file_utility.h"
#include "ltjs_sys_fs_path.h"
#include "SDL3/SDL_stdinc.h"
#include <cassert>
#include <cstdint>
#include <algorithm>
#include <array>
#include <charconv>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ltjs {

namespace {

class ShellResourceMgrImplStringPool
{
public:
	void initialize(int max_size);
	const char* get_string(int offset) const;
	char* allocate(int string_size);
	int get_offset() const noexcept;
	bool is_empty() const noexcept;
	void shrink();
	void swap(ShellResourceMgrImplStringPool& rhs) noexcept;

private:
	static constexpr int alignment = 16;

	using Data = std::vector<char>;

	Data data_{};
	int size_{};
	int offset_{};

	[[noreturn]] static void fail(std::string_view message);
	static int align(int value);
};

// -------------------------------------

void ShellResourceMgrImplStringPool::initialize(int max_size)
{
	assert(max_size >= 0);
	data_.resize(static_cast<std::size_t>(max_size));
	size_ = max_size;
	offset_ = alignment; // Reserve for empty strings.
}

const char* ShellResourceMgrImplStringPool::get_string(int offset) const
{
	if (offset < 0 || offset >= size_)
	{
		fail("Offset out of range.");
	}
	return data_.data() + offset;
}

char* ShellResourceMgrImplStringPool::allocate(int string_size)
{
	if (string_size == 0)
	{
		return data_.data();
	}
	if (string_size < 0)
	{
		fail("String size out of range.");
	}
	if ((string_size + 1) > (size_ - offset_))
	{
		fail("No free space.");
	}

	char* const result = data_.data() + offset_;
	result[string_size] = '\0';
	offset_ = align(offset_ + string_size + 1);
	return result;
}

int ShellResourceMgrImplStringPool::get_offset() const noexcept
{
	return offset_;
}

bool ShellResourceMgrImplStringPool::is_empty() const noexcept
{
	return size_ == 0;
}

void ShellResourceMgrImplStringPool::shrink()
{
	data_.resize(static_cast<std::size_t>(offset_));
	data_.shrink_to_fit();
	size_ = offset_;
}

void ShellResourceMgrImplStringPool::swap(ShellResourceMgrImplStringPool& rhs) noexcept
{
	data_.swap(rhs.data_);
	std::swap(size_, rhs.size_);
	std::swap(offset_, rhs.offset_);
}

[[noreturn]] void ShellResourceMgrImplStringPool::fail(std::string_view message)
{
	throw Exception{"LTJS_SHELL_RESOURCE_MGR_STRING_POOL", message};
}

int ShellResourceMgrImplStringPool::align(int value)
{
	return ((value + alignment - 1) / alignment) * alignment;
}

// =====================================

class ShellResourceMgrImpl final : public ShellResourceMgr
{
public:
	void initialize(const char* base_path) noexcept override;
	void set_language(const char* language_id_name) noexcept override;
	const ShellCursorResource* find_cursor(int number) const noexcept override;
	const ShellCursorResource* find_cursor_by_number_ptr(const char* number_ptr) const noexcept override;
	const ShellStringResource* find_string(int number) const noexcept override;
	int load_string(int number, char* buffer, int buffer_size) noexcept override;
	const ShellTextResource* find_text(const char* name) const noexcept override;
	ShellResourceCodePage get_code_page() const noexcept override;

private:
	static constexpr int min_number = 0;
	static constexpr int max_number = UINT16_MAX;

	static constexpr int min_hot_spot = 0;
	static constexpr int max_hot_spot = UINT8_MAX;

	static constexpr int max_file_size = 4 * 1024 * 1024;
	static constexpr int max_name_size = 128;
	static constexpr int max_path_size = 256;
	static constexpr int max_string_size = 4097;
	static constexpr int max_text_size = 16384;

	static constexpr int min_string_map_capacity = 7000;
	static constexpr int max_string_pool_size = 2 * 1024 * 1024;

	struct CursorContent
	{
		ShellCursorResource shell;
		std::string bytes;
	};

	// name => content
	using CursorMap = std::unordered_map<int, CursorContent>;

	// number => content
	using StringMap = std::unordered_map<int, ShellStringResource>;

	struct TextContent
	{
		ShellTextResource shell;
		std::string bytes;
	};

	// number => content
	using TextMap = std::unordered_map<std::string, TextContent>;

	struct Context
	{
		using Tokens = std::array<ScriptTokenizerToken, 5>;
		using Paths = std::vector<sys::fs::Path>;
		using StringBuffer = std::vector<char>;

		Paths paths;

		ShellResourceCodePage code_page;
		CursorMap cursor_map;
		StringMap string_map;
		ShellResourceMgrImplStringPool string_pool;
		TextMap text_map;

		std::string file_buffer;
		StringBuffer string_buffer;

		ScriptTokenizer script_tokenizer;
		Tokens tokens;
	};

	sys::fs::Path base_path_{};
	std::string language_id_name_{};

	ShellResourceCodePage code_page_{};

	CursorMap cursor_map_{};
	StringMap string_map_{};
	ShellResourceMgrImplStringPool string_pool_{};
	TextMap text_map_{};

	[[noreturn]] static void fail(std::string_view message);
	static void clear_code_page(Context& context);
	static void clear_cursors(Context& context);
	static void clear_strings(Context& context);
	static void clear_texts(Context& context);
	void reset();
	static int number_ptr_to_number(const void* number_ptr) noexcept;
	static bool is_english_char(char ch) noexcept;
	static void validate_language_id_name(const std::string& language_id_name);
	static bool is_resource_number_in_range(int number) noexcept;
	static bool load_file(Context& context, const sys::fs::Path& path);
	static int parse_int32(std::string_view string);
	static int parse_int32(const ScriptTokenizerToken& token);
	static std::string parse_name(const ScriptTokenizerToken& token);
	static std::string parse_path(const ScriptTokenizerToken& token);
	static int calculate_u8_to_cp_1252_size(const char* string, int string_size) noexcept;
	static int calculate_u8_to_cp_size(ShellResourceCodePage code_page, const char* string, int string_size);
	static int convert_u8_to_cp_1252(
		const char* src_string,
		int src_string_size,
		char* dst_string,
		int dst_string_size);
	static int convert_u8_to_cp(
		ShellResourceCodePage code_page,
		const char* src_string,
		int src_string_size,
		char* dst_string,
		int dst_string_size);
	static void convert_u8_to_cp(
		ShellResourceCodePage code_page,
		const char* src_string,
		int src_string_size,
		ShellResourceMgrImplStringPool& string_pool);
	static int parse_string(Context& context, const ScriptTokenizerToken& token);
	static ShellResourceCodePage parse_code_page_string(Context& context, const ScriptTokenizerToken& token);
	static void append_code_page(Context& context, const sys::fs::Path& script_path);
	static void load_code_pages(Context& context);
	static int parse_resource_number(const ScriptTokenizerToken& token);
	static int parse_cursor_hot_spot(const ScriptTokenizerToken& token);
	static void append_cursors(Context& context, const sys::fs::Path& script_path);
	static void load_cursors_contents(Context& context);
	static void load_cursors(Context& context);
	static void append_strings(Context& context, const sys::fs::Path& script_path);
	static void load_strings(Context& context);
	static void append_texts(Context& context, const sys::fs::Path& script_path);
	static void load_texts_contents(Context& context);
	static void load_texts(Context& context);
};

// =====================================

void ShellResourceMgrImpl::initialize(const char* base_path) noexcept
{
	if (base_path != nullptr)
	{
		base_path_ = base_path;
	}
	else
	{
		assert(!"Null base path.");
		base_path_.clear();
	}
	reset();
}

void ShellResourceMgrImpl::set_language(const char* language_id_name) noexcept
try
{
	reset();
	if (language_id_name == nullptr)
	{
		assert(!"Null language id name.");
		return;
	}
	validate_language_id_name(language_id_name);
	Context context{};
	Context::Paths& paths = context.paths;
	paths.reserve(4);
	paths.emplace_back(base_path_);
	paths.emplace_back(base_path_ / language_id_name);
	load_code_pages(context);
	load_cursors(context);
	load_strings(context);
	load_texts(context);
	language_id_name_ = language_id_name;
	code_page_ = context.code_page;
	cursor_map_.swap(context.cursor_map);
	string_map_.swap(context.string_map);
	string_pool_.swap(context.string_pool);
	text_map_.swap(context.text_map);
}
catch (...)
{
	reset();
}

ShellResourceCodePage ShellResourceMgrImpl::get_code_page() const noexcept
{
	return code_page_;
}

const ShellCursorResource* ShellResourceMgrImpl::find_cursor(int number) const noexcept
{
	if (const auto cursor_iter = cursor_map_.find(number);
		cursor_iter != cursor_map_.cend())
	{
		return &cursor_iter->second.shell;
	}
	return nullptr;
}

const ShellCursorResource* ShellResourceMgrImpl::find_cursor_by_number_ptr(const char* number_ptr) const noexcept
{
	if (const int number = number_ptr_to_number(number_ptr);
		number >= 0)
	{
		return find_cursor(number);
	}
	return nullptr;
}

const ShellStringResource* ShellResourceMgrImpl::find_string(int number) const noexcept
{
	if (const auto string_iter = string_map_.find(number);
		string_iter != string_map_.cend())
	{
		return &string_iter->second;
	}
	return nullptr;
}

int ShellResourceMgrImpl::load_string(int number, char* buffer, int buffer_size) noexcept
{
	if (buffer == nullptr)
	{
		assert(!"Null buffer.");
		return 0;
	}
	if (buffer_size < 2)
	{
		return 0;
	}
	const ShellStringResource* const string = find_string(number);
	if (string == nullptr)
	{
		return 0;
	}
	const int copy_count = std::min(buffer_size - 1, string->data.size);
	std::copy_n(string->data.data, copy_count, buffer);
	buffer[copy_count] = '\0';
	return copy_count;
}

const ShellTextResource* ShellResourceMgrImpl::find_text(const char* name) const noexcept
{
	if (name == nullptr)
	{
		assert(!"Null name.");
		return nullptr;
	}
	if (const auto text_iter = text_map_.find(name);
		text_iter != text_map_.cend())
	{
		return &text_iter->second.shell;
	}
	return nullptr;
}

[[noreturn]] void ShellResourceMgrImpl::fail(std::string_view message)
{
	throw Exception{"LTJS_SHELL_RESOURCE_MGR", message};
}

void ShellResourceMgrImpl::clear_code_page(Context& context)
{
	context.code_page = ShellResourceCodePage::none;
}

void ShellResourceMgrImpl::clear_cursors(Context& context)
{
	context.cursor_map.clear();
}

void ShellResourceMgrImpl::clear_strings(Context& context)
{
	context.string_map.clear();
}

void ShellResourceMgrImpl::clear_texts(Context& context)
{
	context.text_map.clear();
}

void ShellResourceMgrImpl::reset()
{
	language_id_name_.clear();
	code_page_ = ShellResourceCodePage::none;
	cursor_map_.clear();
	string_map_.clear();
	text_map_.clear();
}

int ShellResourceMgrImpl::number_ptr_to_number(const void* number_ptr) noexcept
{
	if (const std::uintptr_t ptr_as_number = reinterpret_cast<std::uintptr_t>(number_ptr);
		ptr_as_number <= max_number)
	{
		return static_cast<int>(ptr_as_number);
	}
	return -1;
}

bool ShellResourceMgrImpl::is_english_char(char ch) noexcept
{
	return ascii_is_lower(ch) || ascii_is_upper(ch);
}

void ShellResourceMgrImpl::validate_language_id_name(const std::string& language_id_name)
{
	if (language_id_name.size() != 2)
	{
		fail("Language id name size out of range.");
	}
	const auto ch_0 = language_id_name[0];
	const auto ch_1 = language_id_name[1];
	if (!(ascii_is_lower(ch_0) && ascii_is_lower(ch_1)))
	{
		fail("Unsupported language id name.");
	}
}

bool ShellResourceMgrImpl::is_resource_number_in_range(int number) noexcept
{
	return number >= min_number && number <= max_number;
}

bool ShellResourceMgrImpl::load_file(Context& context, const sys::fs::Path& path)
try
{
	std::string& file_buffer = context.file_buffer;
	file_buffer.clear();
	const int file_size = sys::get_file_size(path.get_data());
	if (file_size <= 0)
	{
		return false;
	}
	file_buffer.resize(file_size);
	const auto loaded_size = ltjs::sys::load_file(path.get_data(), file_buffer.data(), file_size);
	return loaded_size == file_size;
}
catch (...)
{
	return false;
}

int ShellResourceMgrImpl::parse_int32(std::string_view string)
{
	int number;
	if (const auto [string_end, ec] = std::from_chars(string.data(), string.data() + string.size(), number);
		ec == std::errc{})
	{
		if (number < INT32_MIN || number > INT32_MAX)
		{
			fail("Parsed int32 out of range.");
		}
		return number;
	}
	fail("Invalid number string.");
}

int ShellResourceMgrImpl::parse_int32(const ScriptTokenizerToken& token)
{
	assert(!token.is_any_end_of());
	return parse_int32(token.data);
}

std::string ShellResourceMgrImpl::parse_name(const ScriptTokenizerToken& token)
{
	assert(!token.is_any_end_of());
	if (token.is_escaped())
	{
		fail("Escaped name string.");
	}
	if (static_cast<int>(token.data.size()) > max_name_size)
	{
		fail("Name size out of range.");
	}
	for (std::size_t i = 0; i < token.data.size(); ++i)
	{
		const char ch = token.data[i];
		if (ch != '_' &&
			!is_english_char(ch) &&
			(i > 0 ? !ascii_is_digit(ch) : true))
		{
			fail("Unsupported name character.");
		}
	}
	return std::string{token.data};
}

std::string ShellResourceMgrImpl::parse_path(const ScriptTokenizerToken& token)
{
	assert(!token.is_any_end_of());
	if (token.is_escaped())
	{
		fail("Escaped path string.");
	}
	if (token.data.empty() || static_cast<int>(token.data.size()) > max_path_size)
	{
		fail("Path size out of range.");
	}
	for (const char ch : token.data)
	{
		const unsigned char byte = static_cast<unsigned char>(ch);
		if (byte < 0x20)
		{
			fail("Unsupported control character.");
		}
		if (ch != '/' &&
			ch != '.' &&
			ch != '_' &&
			ascii_is_lower(ch) &&
			ascii_is_digit(ch))
		{
			fail("Unsupported path character.");
		}
	}
	return std::string{token.data};
}

int ShellResourceMgrImpl::calculate_u8_to_cp_1252_size(const char* string, int string_size) noexcept
{
	return static_cast<int>(SDL_utf8strnlen(string, static_cast<std::size_t>(string_size)));
}

int ShellResourceMgrImpl::calculate_u8_to_cp_size(ShellResourceCodePage code_page, const char* string, int string_size)
{
	switch (code_page)
	{
		case ShellResourceCodePage::windows_1252:
			return calculate_u8_to_cp_1252_size(string, string_size);
		default:
			fail("Unsupported code page.");
	}
}

int ShellResourceMgrImpl::convert_u8_to_cp_1252(const char* src_string, int src_string_size, char* dst_string, int dst_string_size)
{
	int written_size = 0;
	const char* sdl_str = src_string;
	std::size_t sdl_len = src_string_size;
	while (sdl_len > 0 && written_size < dst_string_size)
	{
		Uint32 sdl_codepoint;
		if (*sdl_str != '\0')
		{
			sdl_codepoint = SDL_StepUTF8(&sdl_str, &sdl_len);
		}
		else
		{
			sdl_codepoint = 0;
		}
		char dst_char;
		if (const int cp1252_code_point = windows_1252_from_unicode(static_cast<int>(sdl_codepoint));
			cp1252_code_point >= 0)
		{
			dst_char = static_cast<char>(cp1252_code_point);
		}
		else
		{
			dst_char = '?';
		}
		dst_string[written_size++] = dst_char;
	}
	return written_size;
}

int ShellResourceMgrImpl::convert_u8_to_cp(
	ShellResourceCodePage code_page,
	const char* src_string,
	int src_string_size,
	char* dst_string,
	int dst_string_size)
{
	switch (code_page)
	{
		case ShellResourceCodePage::windows_1252:
			return convert_u8_to_cp_1252(src_string, src_string_size, dst_string, dst_string_size);
		default:
			fail("Unsupported code page.");
	}
}

void ShellResourceMgrImpl::convert_u8_to_cp(
	ShellResourceCodePage code_page,
	const char* src_string,
	int src_string_size,
	ShellResourceMgrImplStringPool& string_pool)
{
	const int dst_string_size = calculate_u8_to_cp_size(code_page, src_string, src_string_size);
	char* const dst_string = string_pool.allocate(dst_string_size);
	convert_u8_to_cp(code_page, src_string, src_string_size, dst_string, dst_string_size);
}

int ShellResourceMgrImpl::parse_string(Context& context, const ScriptTokenizerToken& token)
{
	assert(!token.is_any_end_of());
	if (!token.is_quoted)
	{
		fail("Unquoted string.");
	}
	ShellResourceMgrImplStringPool& string_pool = context.string_pool;
	const int result = string_pool.get_offset();
	const bool is_code_page_utf8 = (context.code_page == ShellResourceCodePage::utf_8);
	if (token.is_escaped())
	{
		if (token.unescaped_size > max_string_size)
		{
			fail("Unescaped string size out of range.");
		}
		if (is_code_page_utf8)
		{
			const int dst_string_size = token.unescaped_size;
			char* const dst_string = string_pool.allocate(dst_string_size);
			ScriptTokenizer::unescape_string(token, dst_string, dst_string_size);
		}
		else
		{
			char* const src_string = context.string_buffer.data();
			const int src_string_size = token.unescaped_size;
			ScriptTokenizer::unescape_string(token, src_string, src_string_size);
			convert_u8_to_cp(context.code_page, src_string, src_string_size, string_pool);
		}
	}
	else
	{
		if (static_cast<int>(token.data.size()) > max_string_size)
		{
			fail("String too long.");
		}
		if (is_code_page_utf8)
		{
			char* const dst_string = string_pool.allocate(static_cast<int>(token.data.size()));
			std::copy(token.data.cbegin(), token.data.cend(), dst_string);
		}
		else
		{
			convert_u8_to_cp(context.code_page, token.data.data(), static_cast<int>(token.data.size()), string_pool);
		}
	}
	return result;
}

ShellResourceCodePage ShellResourceMgrImpl::parse_code_page_string(Context& context, const ScriptTokenizerToken& token)
{
	assert(!token.is_any_end_of());
	if (token.is_escaped())
	{
		fail("Escaped string.");
	}
	if (token.data.empty())
	{
		fail("Empty string.");
	}
	struct CodePageInfo
	{
		ShellResourceCodePage id;
		std::string_view name;
	};
	static constexpr const char* utf_8_string = "utf-8";
	static constexpr const char* windows_1252_string = "windows-1252";
	using CodePageData = ShellResourceData<const char>;
	static constexpr CodePageInfo code_page_infos[] = {
		CodePageInfo
		{
			.id   = ShellResourceCodePage::utf_8,
			.name = utf_8_string
		},
		CodePageInfo
		{
			.id   = ShellResourceCodePage::windows_1252,
			.name = windows_1252_string
		}};
	static constexpr const CodePageInfo* code_page_info_end_iter = std::cend(code_page_infos);
	const auto code_page_info_iter = std::find_if(
		std::cbegin(code_page_infos),
		code_page_info_end_iter,
		[&token](const CodePageInfo& code_page_info)
		{
			return std::equal(token.data.cbegin(), token.data.cend(), code_page_info.name.cbegin(), code_page_info.name.cend());
		});
	if (code_page_info_iter == code_page_info_end_iter)
	{
		fail("Unsupported code page.");
	}
	return code_page_info_iter->id;
}

void ShellResourceMgrImpl::append_code_page(Context& context, const sys::fs::Path& script_path)
{
	if (!load_file(context, script_path))
	{
		return;
	}
	const ScriptTokenizerInitParam tokenizer_init_param{.data = context.file_buffer};
	ScriptTokenizer& script_tokenizer = context.script_tokenizer;
	script_tokenizer.initialize(tokenizer_init_param);
	Context::Tokens& tokens = context.tokens;
	for (;;)
	{
		const ScriptTokenizerLine line = script_tokenizer.tokenize_line(tokens.data(), static_cast<int>(tokens.size()), 1, 1);
		if (!line)
		{
			break;
		}
		if (line.is_empty())
		{
			continue;
		}
		context.code_page = parse_code_page_string(context, line[0]);
	}
}

void ShellResourceMgrImpl::load_code_pages(Context& context)
try
{
	for (const sys::fs::Path& path : context.paths)
	{
		const sys::fs::Path script_path = path / "code_page.txt";
		append_code_page(context, script_path);
	}
	if (context.code_page == ShellResourceCodePage::none)
	{
		context.code_page = ShellResourceCodePage::utf_8;
	}
}
catch (...)
{
	clear_code_page(context);
}

int ShellResourceMgrImpl::parse_resource_number(const ScriptTokenizerToken& token)
{
	assert(!token.is_any_end_of());
	if (token.is_escaped())
	{
		fail("Escaped resource number string.");
	}
	const int number = parse_int32(token);
	if (!is_resource_number_in_range(number))
	{
		fail("Resource number out of range.");
	}
	return number;
}

int ShellResourceMgrImpl::parse_cursor_hot_spot(const ScriptTokenizerToken& token)
{
	const int hot_spot = parse_int32(token);
	if (hot_spot < min_hot_spot || hot_spot > max_hot_spot)
	{
		fail("Cursor hot spot out of range.");
	}
	return hot_spot;
}

void ShellResourceMgrImpl::append_cursors(Context& context, const sys::fs::Path& script_path)
{
	if (!load_file(context, script_path))
	{
		return;
	}
	const ScriptTokenizerInitParam tokenizer_init_param{.data = context.file_buffer};
	ScriptTokenizer& script_tokenizer = context.script_tokenizer;
	script_tokenizer.initialize(tokenizer_init_param);
	Context::Tokens& tokens = context.tokens;
	for (;;)
	{
		const ScriptTokenizerLine line = script_tokenizer.tokenize_line(tokens.data(), static_cast<int>(tokens.size()), 4, 4);
		if (!line)
		{
			break;
		}
		if (line.is_empty())
		{
			continue;
		}
		const int number = parse_resource_number(line[0]);
		std::string path = parse_path(line[1]);
		const int hot_spot_x = parse_cursor_hot_spot(line[2]);
		const int hot_spot_y = parse_cursor_hot_spot(line[3]);
		CursorMap& cursor_map = context.cursor_map;
		const auto cursor_map_item = cursor_map.emplace(number, CursorContent{});
		CursorContent& cursor = cursor_map_item.first->second;
		ShellCursorResource& shell_cursor = cursor.shell;
		shell_cursor.hot_spot_x = hot_spot_x;
		shell_cursor.hot_spot_y = hot_spot_y;
		cursor.bytes.swap(path);
	}
}

void ShellResourceMgrImpl::load_cursors_contents(Context& context)
{
	CursorMap& cursor_map = context.cursor_map;
	if (cursor_map.empty())
	{
		return;
	}
	using PurgeSet = std::unordered_set<int>;
	PurgeSet purge_set{};
	purge_set.reserve(cursor_map.size());
	const Context::Paths& paths = context.paths;
	for (auto& cursor_map_item : cursor_map)
	{
		const auto& file_name = sys::fs::Path{cursor_map_item.second.bytes};
		CursorContent& cursor = cursor_map_item.second;
		bool is_found = false;
		for (auto path = paths.crbegin(), paths_end = paths.crend(); path != paths_end; ++path)
		{
			const sys::fs::Path cursor_path = (*path) / file_name;
			if (load_file(context, cursor_path))
			{
				is_found = true;
				cursor.bytes = context.file_buffer;
				break;
			}
		}
		if (!is_found)
		{
			purge_set.insert(cursor_map_item.first);
		}
	}
	for (int purge_number : purge_set)
	{
		cursor_map.erase(purge_number);
	}
	for (auto& cursor_map_item : cursor_map)
	{
		CursorContent& cursor = cursor_map_item.second;
		ShellCursorResource::Data& shell_cursor_data = cursor.shell.data;
		shell_cursor_data.data = cursor.bytes.data();
		shell_cursor_data.size = static_cast<int>(cursor.bytes.size());
	}
}

void ShellResourceMgrImpl::load_cursors(Context& context)
try
{
	for (const sys::fs::Path& path : context.paths)
	{
		const sys::fs::Path script_path = path / "cursors.txt";
		append_cursors(context, script_path);
	}
	load_cursors_contents(context);
}
catch (...)
{
	clear_cursors(context);
}

void ShellResourceMgrImpl::append_strings(Context& context, const sys::fs::Path& script_path)
{
	if (!load_file(context, script_path))
	{
		return;
	}
	const ScriptTokenizerInitParam tokenizer_init_param{.data = context.file_buffer};
	ScriptTokenizer& script_tokenizer = context.script_tokenizer;
	script_tokenizer.initialize(tokenizer_init_param);
	Context::Tokens& tokens = context.tokens;
	for (;;)
	{
		const ScriptTokenizerLine line = script_tokenizer.tokenize_line(tokens.data(), static_cast<int>(tokens.size()), 2, 2);
		if (!line)
		{
			break;
		}
		if (line.is_empty())
		{
			continue;
		}
		const int number = parse_resource_number(line[0]);
		int string_offset = parse_string(context, line[1]);
		StringMap& string_map = context.string_map;
		const auto& string_map_item = string_map.emplace(number, ShellStringResource{});
		string_map_item.first->second.data.size = static_cast<int>(string_offset);
	}
}

void ShellResourceMgrImpl::load_strings(Context& context)
try
{
	StringMap& string_map = context.string_map;
	string_map.reserve(min_string_map_capacity);
	ShellResourceMgrImplStringPool& string_pool = context.string_pool;
	string_pool.initialize(max_string_pool_size);
	Context::StringBuffer& string_buffer = context.string_buffer;
	string_buffer.resize(max_string_size);
	for (const sys::fs::Path& path : context.paths)
	{
		const sys::fs::Path script_path = path / "strings.txt";
		append_strings(context, script_path);
	}
	string_pool.shrink();
	for (auto& string_map_item : string_map)
	{
		ShellStringResource::Data& data = string_map_item.second.data;
		const std::string_view string = string_pool.get_string(data.size);
		data.data = string.data();
		data.size = static_cast<int>(string.size());
	}
}
catch (...)
{
	clear_strings(context);
}

void ShellResourceMgrImpl::append_texts(Context& context, const sys::fs::Path& script_path)
{
	if (!load_file(context, script_path))
	{
		return;
	}
	std::string& file_buffer = context.file_buffer;
	if (file_buffer.empty())
	{
		return;
	}
	const ScriptTokenizerInitParam tokenizer_init_param{.data = context.file_buffer};
	ScriptTokenizer& script_tokenizer = context.script_tokenizer;
	script_tokenizer.initialize(tokenizer_init_param);
	Context::Tokens& tokens = context.tokens;
	for (;;)
	{
		const ScriptTokenizerLine line = script_tokenizer.tokenize_line(tokens.data(), static_cast<int>(tokens.size()), 2, 2);
		if (!line)
		{
			break;
		}
		if (line.is_empty())
		{
			continue;
		}
		std::string name = parse_name(line[0]);
		std::string path = parse_path(line[1]);
		TextMap& text_map = context.text_map;
		const auto& text_map_item = text_map.emplace(std::move(name), TextContent{});
		text_map_item.first->second.bytes.swap(path);
	}
}

void ShellResourceMgrImpl::load_texts_contents(Context& context)
{
	TextMap& text_map = context.text_map;
	if (text_map.empty())
	{
		return;
	}
	const bool is_code_page_utf8 = (context.code_page == ShellResourceCodePage::utf_8);
	using PurgeSet = std::unordered_set<std::string>;
	PurgeSet purge_set{};
	purge_set.reserve(text_map.size());
	const Context::Paths& paths = context.paths;
	for (auto& text_map_item : text_map)
	{
		bool is_found = false;
		for (auto path = paths.crbegin(), paths_end = paths.crend(); path != paths_end; ++path)
		{
			const sys::fs::Path text_path = (*path) / text_map_item.second.bytes.c_str();
			if (load_file(context, text_path))
			{
				if (context.file_buffer.size() > max_text_size)
				{
					fail("Text size out of range.");
				}
				is_found = true;
				if (is_code_page_utf8)
				{
					text_map_item.second.bytes = context.file_buffer;
				}
				else
				{
					const ShellResourceCodePage code_page = context.code_page;
					const char* const src_string = context.file_buffer.c_str();
					const int src_string_size = static_cast<int>(context.file_buffer.size());
					const int dst_string_size = calculate_u8_to_cp_size(code_page, src_string, src_string_size);
					text_map_item.second.bytes.resize(dst_string_size);
					char* const dst_string = &text_map_item.second.bytes[0];
					convert_u8_to_cp(code_page, src_string, src_string_size, dst_string, dst_string_size);
				}
				break;
			}
		}
		if (!is_found)
		{
			purge_set.emplace(text_map_item.first);
		}
	}
	for (const std::string& purge_name : purge_set)
	{
		text_map.erase(purge_name);
	}
	for (auto& text_map_item : text_map)
	{
		TextContent& text = text_map_item.second;
		ShellTextResource::Data& shell_text_data = text.shell.data;
		shell_text_data.data = text.bytes.data();
		shell_text_data.size = static_cast<int>(text.bytes.size());
	}
}

void ShellResourceMgrImpl::load_texts(Context& context)
try
{
	for (const sys::fs::Path& path : context.paths)
	{
		const sys::fs::Path script_path = path / "texts.txt";
		append_texts(context, script_path);
	}
	load_texts_contents(context);
}
catch (...)
{
	clear_texts(context);
}

} // namespace

// =====================================

ShellResourceMgrUPtr make_shell_resource_mgr()
{
	return std::make_unique<ShellResourceMgrImpl>();
}

} // namespace ltjs
