#include "ltjs_shell_resource_mgr.h"

#include <cassert>

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "ltjs_ascii.h"
#include "ltjs_c_string.h"
#include "ltjs_char_conv.h"
#include "ltjs_exception.h"
#include "ltjs_file.h"
#include "ltjs_file_system_path.h"
#include "ltjs_index_type.h"
#include "ltjs_script_tokenizer.h"
#include "ltjs_ucs.h"
#include "ltjs_windows_1252.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

ShellResourceMgr::ShellResourceMgr() noexcept = default;

ShellResourceMgr::~ShellResourceMgr() = default;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class ShellResourceMgrException :
	public Exception
{
public:
	explicit ShellResourceMgrException(
		const char* message)
		:
		Exception{"LTJS_SHELL_RESOURCE_MGR", message}
	{
	}
}; // ShellResourceMgrException
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class ShellResourceMgrCStringPoolException :
	public Exception
{
public:
	explicit ShellResourceMgrCStringPoolException(
		const char* message)
		:
		Exception{"LTJS_SHELL_RESOURCE_MGR_CSTRING_POOL", message}
	{
	}
}; // ShellResourceMgrCStringPoolException
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class ShellResourceMgrImpl final :
	public ShellResourceMgr
{
public:
	// ======================================================================
	// ShellResourceMgr

	void initialize(
		const char* base_path) noexcept override;

	void set_language(
		const char* language_id_name) noexcept override;


	const ShellCursorResource* find_cursor(
		int number) const noexcept override;

	const ShellCursorResource* find_cursor_by_number_ptr(
		const char* number_ptr) const noexcept override;


	const ShellStringResource* find_string(
		int number) const noexcept override;

	Index load_string(
		int number,
		char* buffer,
		Index buffer_size) noexcept override;


	const ShellTextResource* find_text(
		const char* name) const noexcept override;


	ShellResourceCodePage get_code_page() const noexcept override;

	// ShellResourceMgr
	// ======================================================================


private:
	static constexpr auto min_number = 0;
	static constexpr auto max_number = 65'535;

	static constexpr auto min_hot_spot = 0;
	static constexpr auto max_hot_spot = 255;

	static constexpr auto max_file_size = 4 * 1'024 * 1'024;
	static constexpr auto max_name_size = 128;
	static constexpr auto max_path_size = 256;
	static constexpr auto max_string_size = 4'097;
	static constexpr auto max_text_size = 16'384;

	static constexpr auto min_string_map_capacity = 7'000;
	static constexpr auto max_string_pool_size = 2 * 1'024 * 1'024;


	struct CursorContent
	{
		ShellCursorResource shell{};
		std::string bytes{};
	}; // CursorContent

	// name => content
	using CursorMap = std::unordered_map<int, CursorContent>;


	// number => content
	using StringMap = std::unordered_map<int, ShellStringResource>;


	struct TextContent
	{
		ShellTextResource shell{};
		std::string bytes{};
	}; // TextContent

	// number => content
	using TextMap = std::unordered_map<std::string, TextContent>;


	class CStringPool
	{
	public:
		void initialize(
			Index max_size);

		const char* get_string(
			Index offset) const;

		char* allocate(
			Index string_size);

		Index get_offset() const noexcept;

		bool is_empty() const noexcept;


		void shrink();

		void swap(
			CStringPool& rhs) noexcept;


	private:
		using Data = std::vector<char>;

		Data data_{};
		Index size_{};
		Index offset_{};
	}; // CStringPool

	struct Context
	{
		using Tokens = std::array<ScriptTokenizerToken, 5>;
		using Paths = std::vector<file_system::Path>;
		using StringBuffer = std::vector<char>;

		Paths paths{};

		ShellResourceCodePage code_page{};
		CursorMap cursor_map{};
		StringMap string_map{};
		CStringPool string_pool{};
		TextMap text_map{};

		std::string file_buffer{};
		StringBuffer string_buffer{};

		ScriptTokenizer script_tokenizer{};
		Tokens tokens{};
	}; // Context


	file_system::Path base_path_{};
	std::string language_id_name_{};

	ShellResourceCodePage code_page_{};

	CursorMap cursor_map_{};
	StringMap string_map_{};
	CStringPool c_string_pool_{};
	TextMap text_map_{};


	static void clear_code_page(
		Context& context);

	static void clear_cursors(
		Context& context);

	static void clear_strings(
		Context& context);

	static void clear_texts(
		Context& context);

	void reset();

	static int number_ptr_to_number(
		const void* number_ptr) noexcept;

	static bool is_english_char(
		char ch) noexcept;

	static void validate_language_id_name(
		const std::string& language_id_name);

	static bool is_resource_number_in_range(
		int number) noexcept;

	static bool load_file(
		Context& context,
		const file_system::Path& path);

	static int parse_int32(
		const char* string,
		Index string_size);

	static int parse_int32(
		const ScriptTokenizerToken& token);

	static std::string parse_name(
		const ScriptTokenizerToken& token);

	static std::string parse_path(
		const ScriptTokenizerToken& token);


	static Index calculate_u8_to_cp_1252_size(
		const char* string,
		Index string_size) noexcept;

	static Index calculate_u8_to_cp_size(
		ShellResourceCodePage code_page,
		const char* string,
		Index string_size);


	static Index convert_u8_to_cp_1252(
		const char* src_string,
		Index src_string_size,
		char* dst_string,
		Index dst_string_size);

	static Index convert_u8_to_cp(
		ShellResourceCodePage code_page,
		const char* src_string,
		Index src_string_size,
		char* dst_string,
		Index dst_string_size);


	static void convert_u8_to_cp(
		ShellResourceCodePage code_page,
		const char* src_string,
		Index src_string_size,
		CStringPool& c_string_pool);


	static Index parse_string(
		Context& context,
		const ScriptTokenizerToken& token);


	static ShellResourceCodePage parse_code_page_string(
		Context& context,
		const ScriptTokenizerToken& token);

	static void append_code_page(
		Context& context,
		const file_system::Path& script_path);

	static void load_code_pages(
		Context& context);


	static int parse_resource_number(
		const ScriptTokenizerToken& token);

	static int parse_cursor_hot_spot(
		const ScriptTokenizerToken& token);

	static void append_cursors(
		Context& context,
		const file_system::Path& script_path);

	static void load_cursors_contents(
		Context& context);

	static void load_cursors(
		Context& context);


	static void append_strings(
		Context& context,
		const file_system::Path& script_path);

	static void load_strings(
		Context& context);


	static void append_texts(
		Context& context,
		const file_system::Path& script_path);

	static void load_texts_contents(
		Context& context);

	static void load_texts(
		Context& context);
}; // ShellResourceMgrImpl

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void ShellResourceMgrImpl::CStringPool::initialize(
	Index max_size)
{
	assert(max_size >= 0);

	data_.resize(max_size);
	size_ = max_size;
	offset_ = 1; // Reserve ony byte for empty strings.
}

const char* ShellResourceMgrImpl::CStringPool::get_string(
	Index offset) const
{
	if (offset < 0 || offset >= size_)
	{
		throw ShellResourceMgrCStringPoolException{"Offset out of range."};
	}

	return data_.data() + offset;
}

char* ShellResourceMgrImpl::CStringPool::allocate(
	Index string_size)
{
	if (string_size == 0)
	{
		return data_.data();
	}

	if (string_size < 0)
	{
		throw ShellResourceMgrCStringPoolException{"String size out of range."};
	}

	if ((string_size + 1) > (size_ - offset_))
	{
		throw ShellResourceMgrCStringPoolException{"No free space."};
	}

	auto result = data_.data() + offset_;
	offset_ += string_size + 1;
	auto string_nul = result + string_size;
	(*string_nul) = '\0';

	return result;
}

Index ShellResourceMgrImpl::CStringPool::get_offset() const noexcept
{
	return offset_;
}

bool ShellResourceMgrImpl::CStringPool::is_empty() const noexcept
{
	return size_ == 0;
}

void ShellResourceMgrImpl::CStringPool::shrink()
{
	data_.resize(offset_);
	data_.shrink_to_fit();

	size_ = offset_;
}

void ShellResourceMgrImpl::CStringPool::swap(
	CStringPool& rhs) noexcept
{
	data_.swap(rhs.data_);
	std::swap(size_, rhs.size_);
	std::swap(offset_, rhs.offset_);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void ShellResourceMgrImpl::initialize(
	const char* base_path) noexcept
{
	if (base_path)
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

void ShellResourceMgrImpl::set_language(
	const char* language_id_name) noexcept
try
{
	reset();

	if (!language_id_name)
	{
		assert(!"Null language id name.");
		return;
	}

	validate_language_id_name(language_id_name);

	auto context = Context{};

	auto& paths = context.paths;
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
	c_string_pool_.swap(context.string_pool);
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

const ShellCursorResource* ShellResourceMgrImpl::find_cursor(
	int number) const noexcept
{
	auto cursor_it = cursor_map_.find(number);

	if (cursor_it == cursor_map_.cend())
	{
		return nullptr;
	}

	return &cursor_it->second.shell;
}

const ShellCursorResource* ShellResourceMgrImpl::find_cursor_by_number_ptr(
	const char* number_ptr) const noexcept
{
	const auto number = number_ptr_to_number(number_ptr);

	if (number < 0)
	{
		return nullptr;
	}

	return find_cursor(number);
}

const ShellStringResource* ShellResourceMgrImpl::find_string(
	int number) const noexcept
{
	auto string_it = string_map_.find(number);

	if (string_it == string_map_.cend())
	{
		return nullptr;
	}

	return &string_it->second;
}

Index ShellResourceMgrImpl::load_string(
	int number,
	char* buffer,
	Index buffer_size) noexcept
{
	if (!buffer)
	{
		assert(!"Null buffer.");
		return 0;
	}

	if (buffer_size < 2)
	{
		return 0;
	}

	auto string = find_string(number);

	if (!string)
	{
		return 0;
	}

	const auto copy_count = std::min(buffer_size - 1, string->data.size);
	std::uninitialized_copy_n(string->data.data, copy_count, buffer);
	buffer[copy_count] = '\0';

	return copy_count;
}

const ShellTextResource* ShellResourceMgrImpl::find_text(
	const char* name) const noexcept
{
	if (!name)
	{
		assert(!"Null name.");
		return nullptr;
	}

	const auto text_it = text_map_.find(name);

	if (text_it == text_map_.cend())
	{
		return nullptr;
	}

	return &text_it->second.shell;
}

void ShellResourceMgrImpl::clear_code_page(
	Context& context)
{
	context.code_page = ShellResourceCodePage::none;
}

void ShellResourceMgrImpl::clear_cursors(
	Context& context)
{
	context.cursor_map.clear();
}

void ShellResourceMgrImpl::clear_strings(
	Context& context)
{
	context.string_map.clear();
}

void ShellResourceMgrImpl::clear_texts(
	Context& context)
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

int ShellResourceMgrImpl::number_ptr_to_number(
	const void* number_ptr) noexcept
{
	const auto ptr_as_int = reinterpret_cast<std::uintptr_t>(number_ptr);

	if (ptr_as_int > max_number)
	{
		return -1;
	}

	return static_cast<int>(ptr_as_int);
}

bool ShellResourceMgrImpl::is_english_char(
	char ch) noexcept
{
	return ascii::is_lower(ch) || ascii::is_upper(ch);
}

void ShellResourceMgrImpl::validate_language_id_name(
	const std::string& language_id_name)
{
	if (language_id_name.size() != 2)
	{
		throw ShellResourceMgrException{"Language id name size out of range."};
	}

	const auto ch_0 = language_id_name[0];
	const auto ch_1 = language_id_name[1];

	if (!(ascii::is_lower(ch_0) && ascii::is_lower(ch_1)))
	{
		throw ShellResourceMgrException{"Unsupported language id name."};
	}
}

bool ShellResourceMgrImpl::is_resource_number_in_range(
	int number) noexcept
{
	return number >= min_number && number <= max_number;
}

bool ShellResourceMgrImpl::load_file(
	Context& context,
	const file_system::Path& path)
try
{
	auto& file_buffer = context.file_buffer;

	file_buffer.clear();

	const auto file_size = file::get_size(path.get_data());

	if (file_size <= 0)
	{
		return false;
	}

	file_buffer.resize(file_size);

	const auto loaded_size = ltjs::file::load(
		path.get_data(),
		&file_buffer[0],
		file_size
	);

	return loaded_size == file_size;
}
catch (...)
{
	return false;
}

int ShellResourceMgrImpl::parse_int32(
	const char* string,
	Index string_size)
{
	assert(string);

	if (string_size < 1 || string_size > 10)
	{
		throw ShellResourceMgrException{"Integer string size out of range."};
	}

	auto is_negative = false;

	switch (string[0])
	{
		case '-':
			is_negative = true;
			string += 1;
			string_size -= 1;
			break;

		case '+':
			string += 1;
			string_size -= 1;
			break;

		default:
			break;
	}

	auto number = 0;

	for (auto i = decltype(string_size){}; i < string_size; ++i)
	{
		const auto ch = string[i];
		const auto digit = ch - '0';

		if (digit < 0 || digit > 9)
		{
			throw ShellResourceMgrException{"Expected a decimal digit."};
		}

		number *= 10;
		number += digit;
	}

	if (is_negative)
	{
		number = -number;
	}

	return number;
}

int ShellResourceMgrImpl::parse_int32(
	const ScriptTokenizerToken& token)
{
	assert(!token.is_any_end_of());

	return parse_int32(token.data, token.size);
}

std::string ShellResourceMgrImpl::parse_name(
	const ScriptTokenizerToken& token)
{
	assert(!token.is_any_end_of());

	if (token.is_escaped())
	{
		throw ShellResourceMgrException{"Escaped name string."};
	}

	if (token.size <= 0 || token.size > max_name_size)
	{
		throw ShellResourceMgrException{"Name size out of range."};
	}

	for (auto i = Index{}; i < token.size; ++i)
	{
		const auto ch = token.data[i];

		if (ch != '_' &&
			!is_english_char(ch) &&
			(i > 0 ? !ascii::is_decimal_digit(ch) : true))
		{
			throw ShellResourceMgrException{"Unsupported name character."};
		}
	}

	return std::string{token.data, static_cast<std::size_t>(token.size)};
}

std::string ShellResourceMgrImpl::parse_path(
	const ScriptTokenizerToken& token)
{
	assert(!token.is_any_end_of());

	if (token.is_escaped())
	{
		throw ShellResourceMgrException{"Escaped path string."};
	}

	if (token.size <= 0 || token.size > max_path_size)
	{
		throw ShellResourceMgrException{"Path size out of range."};
	}

	for (auto i = Index{}; i < token.size; ++i)
	{
		const auto ch = token.data[i];
		const auto byte = static_cast<unsigned char>(ch);

		if (byte < 0x20)
		{
			throw ShellResourceMgrException{"Unsupported control character."};
		}

		if (ch != '/' &&
			ch != '.' &&
			ch != '_' &&
			ascii::is_lower(ch) &&
			ascii::is_decimal_digit(ch))
		{
			throw ShellResourceMgrException{"Unsupported path character."};
		}
	}

	return std::string{token.data, static_cast<std::size_t>(token.size)};
}

Index ShellResourceMgrImpl::calculate_u8_to_cp_1252_size(
	const char* string,
	Index string_size) noexcept
{
	assert(string);

	auto result = Index{};
	auto code_point = ucs::CodePoint{};

	while (true)
	{
		const auto utf8_to_utf32_result = ucs::utf8_to_utf32(
			string,
			string_size,
			&code_point,
			1
		);

		if (utf8_to_utf32_result.utf8_used_size <= 0)
		{
			break;
		}

		string += utf8_to_utf32_result.utf8_used_size;
		string_size -= utf8_to_utf32_result.utf8_used_size;

		const auto from_unicode_result = windows_1252::from_unicode(code_point);

		if (from_unicode_result)
		{
			result += 1;
		}
	}

	return result;
}

Index ShellResourceMgrImpl::calculate_u8_to_cp_size(
	ShellResourceCodePage code_page,
	const char* string,
	Index string_size)
{
	switch (code_page)
	{
		case ShellResourceCodePage::windows_1252:
			return calculate_u8_to_cp_1252_size(string, string_size);

		default:
			throw ShellResourceMgrException{"Unsupported code page."};
	}
}

Index ShellResourceMgrImpl::convert_u8_to_cp_1252(
	const char* src_string,
	Index src_string_size,
	char* dst_string,
	Index dst_string_size)
{
	assert(src_string);
	assert(dst_string);

	auto result = Index{};
	auto code_point = ucs::CodePoint{};

	while (true)
	{
		if (dst_string_size <= 0)
		{
			break;
		}

		const auto utf8_to_utf32_result = ucs::utf8_to_utf32(
			src_string,
			src_string_size,
			&code_point,
			1
		);

		if (utf8_to_utf32_result.utf8_used_size <= 0)
		{
			break;
		}

		src_string += utf8_to_utf32_result.utf8_used_size;
		src_string_size -= utf8_to_utf32_result.utf8_used_size;

		const auto from_unicode_result = windows_1252::from_unicode(code_point);

		if (from_unicode_result)
		{
			(*dst_string++) = from_unicode_result;
			dst_string_size -= 1;

			result += 1;
		}
	}

	return result;
}

Index ShellResourceMgrImpl::convert_u8_to_cp(
	ShellResourceCodePage code_page,
	const char* src_string,
	Index src_string_size,
	char* dst_string,
	Index dst_string_size)
{
	switch (code_page)
	{
		case ShellResourceCodePage::windows_1252:
			return convert_u8_to_cp_1252(src_string, src_string_size, dst_string, dst_string_size);

		default:
			throw ShellResourceMgrException{"Unsupported code page."};
	}
}

void ShellResourceMgrImpl::convert_u8_to_cp(
	ShellResourceCodePage code_page,
	const char* src_string,
	Index src_string_size,
	CStringPool& c_string_pool)
{
	const auto dst_string_size = calculate_u8_to_cp_size(
		code_page,
		src_string,
		src_string_size
	);

	const auto dst_string = c_string_pool.allocate(dst_string_size);

	convert_u8_to_cp(
		code_page,
		src_string,
		src_string_size,
		dst_string,
		dst_string_size
	);
}

Index ShellResourceMgrImpl::parse_string(
	Context& context,
	const ScriptTokenizerToken& token)
{
	assert(!token.is_any_end_of());

	if (!token.is_quoted)
	{
		throw ShellResourceMgrException{"Unquoted string."};
	}

	auto& c_string_pool = context.string_pool;
	const auto result = c_string_pool.get_offset();

	const auto is_code_page_utf8 = (context.code_page == ShellResourceCodePage::utf_8);

	if (token.is_escaped())
	{
		if (token.unescaped_size > max_string_size)
		{
			throw ShellResourceMgrException{"Unescaped string size out of range."};
		}

		if (is_code_page_utf8)
		{
			const auto dst_string_size = token.unescaped_size;
			auto dst_string = c_string_pool.allocate(dst_string_size);

			ScriptTokenizer::unescape_string(
				token,
				dst_string,
				dst_string_size
			);
		}
		else
		{
			const auto src_string = context.string_buffer.data();
			const auto src_string_size = token.unescaped_size;

			ScriptTokenizer::unescape_string(
				token,
				src_string,
				src_string_size
			);

			convert_u8_to_cp(
				context.code_page,
				src_string,
				src_string_size,
				c_string_pool
			);
		}
	}
	else
	{
		if (token.size > max_string_size)
		{
			throw ShellResourceMgrException{"String too long."};
		}

		if (is_code_page_utf8)
		{
			const auto string_size = token.size;
			const auto src_string = token.data;
			auto dst_string = c_string_pool.allocate(string_size);

			std::uninitialized_copy_n(src_string, string_size, dst_string);
		}
		else
		{
			const auto src_string = token.data;
			const auto src_string_size = token.size;

			convert_u8_to_cp(
				context.code_page,
				src_string,
				src_string_size,
				c_string_pool
			);
		}
	}

	return result;
}

ShellResourceCodePage ShellResourceMgrImpl::parse_code_page_string(
	Context& context,
	const ScriptTokenizerToken& token)
{
	assert(!token.is_any_end_of());

	if (token.is_escaped())
	{
		throw ShellResourceMgrException{"Escaped string."};
	}

	if (token.size <= 0)
	{
		throw ShellResourceMgrException{"Empty string."};
	}


	struct CodePageInfo
	{
		ShellResourceCodePage id{};

		const char* name{};
		ltjs::Index name_size{};
	}; // CodePageInfo

	static constexpr auto utf_8_string = "utf-8";
	static constexpr auto windows_1252_string = "windows-1252";

	using CodePageData = ShellResourceData<const char>;

	static constexpr CodePageInfo code_page_infos[] =
	{
		CodePageInfo
		{
			ShellResourceCodePage::utf_8,
			utf_8_string,
			c_string::get_size(utf_8_string)
		},

		CodePageInfo
		{
			ShellResourceCodePage::windows_1252,
			windows_1252_string,
			c_string::get_size(windows_1252_string)
		},
	};

	static constexpr auto code_page_info_end_it = std::cend(code_page_infos);


	const auto code_page_info_it = std::find_if(
		std::cbegin(code_page_infos),
		code_page_info_end_it,
		[&token](
			const CodePageInfo& code_page_info)
		{
			return std::equal(
				token.data,
				token.data + token.size,
				code_page_info.name,
				code_page_info.name + code_page_info.name_size
			);
		}
	);

	if (code_page_info_it == code_page_info_end_it)
	{
		throw ShellResourceMgrException{"Unsupported code page."};
	}

	return code_page_info_it->id;
}

void ShellResourceMgrImpl::append_code_page(
	Context& context,
	const file_system::Path& script_path)
{
	if (!load_file(context, script_path))
	{
		return;
	}

	auto tokenizer_init_param = ScriptTokenizerInitParam{};
	tokenizer_init_param.data = context.file_buffer.data();
	tokenizer_init_param.size = static_cast<Index>(context.file_buffer.size());

	auto& script_tokenizer = context.script_tokenizer;
	script_tokenizer.initialize(tokenizer_init_param);

	auto& tokens = context.tokens;

	while (true)
	{
		const auto line = script_tokenizer.tokenize_line(
			tokens.data(),
			static_cast<Index>(tokens.size()),
			1,
			1
		);

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

void ShellResourceMgrImpl::load_code_pages(
	Context& context)
try
{
	for (const auto& path : context.paths)
	{
		const auto script_path = path / "code_page.txt";
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

int ShellResourceMgrImpl::parse_resource_number(
	const ScriptTokenizerToken& token)
{
	assert(!token.is_any_end_of());

	if (token.is_escaped())
	{
		throw ShellResourceMgrException{"Escaped resource number string."};
	}

	const auto number = parse_int32(token);

	if (!is_resource_number_in_range(number))
	{
		throw ShellResourceMgrException{"Resource number out of range."};
	}

	return number;
}

int ShellResourceMgrImpl::parse_cursor_hot_spot(
	const ScriptTokenizerToken& token)
{
	const auto hot_spot = parse_int32(token);

	if (hot_spot < min_hot_spot || hot_spot > max_hot_spot)
	{
		throw ShellResourceMgrException{"Cursor hot spot out of range."};
	}

	return hot_spot;
}

void ShellResourceMgrImpl::append_cursors(
	Context& context,
	const file_system::Path& script_path)
{
	if (!load_file(context, script_path))
	{
		return;
	}

	auto tokenizer_init_param = ScriptTokenizerInitParam{};
	tokenizer_init_param.data = context.file_buffer.data();
	tokenizer_init_param.size = static_cast<Index>(context.file_buffer.size());

	auto& script_tokenizer = context.script_tokenizer;
	script_tokenizer.initialize(tokenizer_init_param);

	auto& tokens = context.tokens;

	while (true)
	{
		const auto line = script_tokenizer.tokenize_line(
			tokens.data(),
			static_cast<Index>(tokens.size()),
			4,
			4
		);

		if (!line)
		{
			break;
		}

		if (line.is_empty())
		{
			continue;
		}

		const auto number = parse_resource_number(line[0]);
		auto path = parse_path(line[1]);
		const auto hot_spot_x = parse_cursor_hot_spot(line[2]);
		const auto hot_spot_y = parse_cursor_hot_spot(line[3]);


		auto& cursor_map = context.cursor_map;
		const auto cursor_map_pair = cursor_map.emplace(number, CursorContent{});
		auto& cursor = cursor_map_pair.first->second;
		auto& shell_cursor = cursor.shell;
		shell_cursor.hot_spot_x = hot_spot_x;
		shell_cursor.hot_spot_y = hot_spot_y;
		cursor.bytes.swap(path);
	}
}

void ShellResourceMgrImpl::load_cursors_contents(
	Context& context)
{
	auto& cursor_map = context.cursor_map;

	if (cursor_map.empty())
	{
		return;
	}

	using PurgeSet = std::unordered_set<int>;
	auto purge_set = PurgeSet{};
	purge_set.reserve(cursor_map.size());

	const auto& paths = context.paths;

	for (auto& cursor_map_item : cursor_map)
	{
		const auto& file_name = file_system::Path{
			cursor_map_item.second.bytes.c_str(),
			static_cast<Index>(cursor_map_item.second.bytes.size())
		};

		auto& cursor = cursor_map_item.second;

		auto is_found = false;

		for (auto path = paths.crbegin(), paths_end = paths.crend(); path != paths_end; ++path)
		{
			const auto cursor_path = (*path) / file_name;

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

	for (const auto& purge_name : purge_set)
	{
		cursor_map.erase(purge_name);
	}

	for (auto& cursor_map_item : cursor_map)
	{
		auto& cursor = cursor_map_item.second;
		auto& shell_cursor_data = cursor.shell.data;
		shell_cursor_data.data = cursor.bytes.data();
		shell_cursor_data.size = static_cast<decltype(shell_cursor_data.size)>(cursor.bytes.size());
	}
}

void ShellResourceMgrImpl::load_cursors(
	Context& context)
try
{
	for (const auto& path : context.paths)
	{
		const auto script_path = path / "cursors.txt";
		append_cursors(context, script_path);
	}

	load_cursors_contents(context);
}
catch (...)
{
	clear_cursors(context);
}

void ShellResourceMgrImpl::append_strings(
	Context& context,
	const file_system::Path& script_path)
{
	if (!load_file(context, script_path))
	{
		return;
	}

	auto tokenizer_init_param = ScriptTokenizerInitParam{};
	tokenizer_init_param.data = context.file_buffer.data();
	tokenizer_init_param.size = static_cast<Index>(context.file_buffer.size());

	auto& script_tokenizer = context.script_tokenizer;
	script_tokenizer.initialize(tokenizer_init_param);

	auto& tokens = context.tokens;

	while (true)
	{
		const auto line = script_tokenizer.tokenize_line(
			tokens.data(),
			static_cast<Index>(tokens.size()),
			2,
			2
		);

		if (!line)
		{
			break;
		}

		if (line.is_empty())
		{
			continue;
		}

		const auto number = parse_resource_number(line[0]);
		auto string_offset = parse_string(context, line[1]);

		auto& string_map = context.string_map;
		const auto& string_map_pair = string_map.emplace(number, ShellStringResource{});
		string_map_pair.first->second.data.size = static_cast<int>(string_offset);
	}
}

void ShellResourceMgrImpl::load_strings(
	Context& context)
try
{
	auto& string_map = context.string_map;
	string_map.reserve(min_string_map_capacity);

	auto& string_pool = context.string_pool;
	string_pool.initialize(max_string_pool_size);

	auto& string_buffer = context.string_buffer;
	string_buffer.resize(max_string_size);

	for (const auto& path : context.paths)
	{
		const auto script_path = path / "strings.txt";
		append_strings(context, script_path);
	}

	string_pool.shrink();

	for (auto& string_map_item : string_map)
	{
		auto& data = string_map_item.second.data;

		const auto string = string_pool.get_string(data.size);

		data.data = string;
		data.size = static_cast<decltype(data.size)>(c_string::get_size(string));
	}
}
catch (...)
{
	clear_strings(context);
}

void ShellResourceMgrImpl::append_texts(
	Context& context,
	const file_system::Path& script_path)
{
	if (!load_file(context, script_path))
	{
		return;
	}

	auto& file_buffer = context.file_buffer;

	if (file_buffer.empty())
	{
		return;
	}

	auto tokenizer_init_param = ScriptTokenizerInitParam{};
	tokenizer_init_param.data = context.file_buffer.data();
	tokenizer_init_param.size = static_cast<Index>(context.file_buffer.size());

	auto& script_tokenizer = context.script_tokenizer;
	script_tokenizer.initialize(tokenizer_init_param);

	auto& tokens = context.tokens;

	while (true)
	{
		const auto line = script_tokenizer.tokenize_line(
			tokens.data(),
			static_cast<Index>(tokens.size()),
			2,
			2
		);

		if (!line)
		{
			break;
		}

		if (line.is_empty())
		{
			continue;
		}

		auto name = parse_name(line[0]);
		auto path = parse_path(line[1]);

		auto& text_map = context.text_map;
		const auto& text_map_pair = text_map.emplace(std::move(name), TextContent{});
		text_map_pair.first->second.bytes.swap(path);
	}
}

void ShellResourceMgrImpl::load_texts_contents(
	Context& context)
{
	auto& text_map = context.text_map;

	if (text_map.empty())
	{
		return;
	}


	const auto is_code_page_utf8 = (context.code_page == ShellResourceCodePage::utf_8);

	using PurgeSet = std::unordered_set<std::string>;
	auto purge_set = PurgeSet{};
	purge_set.reserve(text_map.size());

	const auto& paths = context.paths;

	for (auto& text_map_item : text_map)
	{
		auto is_found = false;

		for (auto path = paths.crbegin(), paths_end = paths.crend(); path != paths_end; ++path)
		{
			const auto text_path = (*path) / text_map_item.second.bytes.c_str();

			if (load_file(context, text_path))
			{
				if (context.file_buffer.size() > max_text_size)
				{
					throw ShellResourceMgrCStringPoolException{"Text size out of range."};
				}

				is_found = true;

				if (is_code_page_utf8)
				{
					text_map_item.second.bytes = context.file_buffer;
				}
				else
				{
					const auto code_page = context.code_page;

					const auto src_string = context.file_buffer.c_str();
					const auto src_string_size = static_cast<Index>(context.file_buffer.size());

					const auto dst_string_size = calculate_u8_to_cp_size(
						code_page,
						src_string,
						src_string_size
					);

					text_map_item.second.bytes.resize(dst_string_size);

					auto dst_string = &text_map_item.second.bytes[0];

					convert_u8_to_cp(
						code_page,
						src_string,
						src_string_size,
						dst_string,
						dst_string_size
					);
				}

				break;
			}
		}

		if (!is_found)
		{
			purge_set.emplace(text_map_item.first);
		}
	}

	for (const auto& purge_name : purge_set)
	{
		text_map.erase(purge_name);
	}

	for (auto& text_map_item : text_map)
	{
		auto& text = text_map_item.second;
		auto& shell_text_data = text.shell.data;
		shell_text_data.data = text.bytes.data();
		shell_text_data.size = static_cast<decltype(shell_text_data.size)>(text.bytes.size());
	}
}

void ShellResourceMgrImpl::load_texts(
	Context& context)
try
{
	for (const auto& path : context.paths)
	{
		const auto script_path = path / "texts.txt";
		append_texts(context, script_path);
	}

	load_texts_contents(context);
}
catch (...)
{
	clear_texts(context);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

ShellResourceMgrUPtr make_shell_resource_mgr()
{
	return std::make_unique<ShellResourceMgrImpl>();
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs
