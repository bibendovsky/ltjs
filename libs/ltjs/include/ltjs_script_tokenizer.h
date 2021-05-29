#ifndef LTJS_SCRIPT_TOKENIZER_INCLUDED
#define LTJS_SCRIPT_TOKENIZER_INCLUDED


#include "ltjs_index_type.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

struct ScriptTokenizerToken
{
	bool is_end_of_data{};
	bool is_end_of_line{};
	bool is_quoted{};

	Index line_number{};
	Index column_number{};

	const char* data{};
	Index size{};
	Index unescaped_size{};


	bool is_any_end_of() const noexcept;

	bool is_escaped() const noexcept;

	bool is_empty() const noexcept;
}; // ScriptTokenizerToken

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

struct ScriptTokenizerLine
{
	bool is_end_of_data{};
	const ScriptTokenizerToken* tokens{};
	Index size{};


	bool is_empty() const noexcept;

	explicit operator bool() const noexcept;

	const ScriptTokenizerToken& operator[](
		Index index) const;

	ltjs::Index get_line_number() const;
}; // ScriptTokenizerLine

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

struct ScriptTokenizerInitParam
{
	const char* data{};
	Index size{};
}; // ScriptTokenizerInitParam


class ScriptTokenizer
{
public:
	void initialize(
		const ScriptTokenizerInitParam& param);

	void tokenize(
		ScriptTokenizerToken& token);

	void tokenize(
		ScriptTokenizerToken* tokens,
		Index token_count);

	ScriptTokenizerLine tokenize_line(
		ScriptTokenizerToken* buffer,
		Index buffer_size,
		Index min_tokens,
		Index max_tokens);


	static Index escape_string(
		const char* src_string,
		Index src_string_size,
		char* dst_string,
		Index dst_string_size);

	static Index unescape_string(
		const char* src_string,
		Index src_string_size,
		char* dst_string,
		Index dst_string_size);

	static Index unescape_string(
		const ScriptTokenizerToken& token,
		char* dst_string,
		Index dst_max_string_size);


private:
	enum class State
	{
		none,

		find_token,
		end_of_data,
		end_of_line,
		skip_single_line_comment,
		skip_multi_line_comment,
		parse_quoted_token,
		check_end_of_quoted_token,
		quoted_token,
		parse_unquoted_token,
		unquoted_token,
	}; // State


	bool is_initialized_{};

	const char* data_{};
	Index size_{};
	Index offset_{};
	Index line_number_{};
	Index column_number_{};
	State state_{};
	ScriptTokenizerToken* token_{};


	static void validate_param(
		const ScriptTokenizerInitParam& param);


	[[noreturn]]
	void fail(
		const char* message) const;


	bool is_end_of_data() const noexcept;

	char peek(
		Index offset) const noexcept;

	bool is_end_of_line() const noexcept;

	bool is_windows_end_of_line() const noexcept;

	bool is_backslash() const noexcept;

	bool is_whitespace() const noexcept;

	static bool is_control_code(
		char ch) noexcept;

	bool is_control_code() const noexcept;

	bool is_double_quotation_mark() const noexcept;

	bool is_single_line_comment() const noexcept;

	bool is_multi_line_comment_begin() const noexcept;

	bool is_multi_line_comment_end() const noexcept;


	void advance_data(
		Index count) noexcept;

	void set_end_of_data() noexcept;

	void set_end_of_line() noexcept;

	void set_quoted() noexcept;

	void set_unquoted() noexcept;

	void skip_end_of_line();

	void skip_single_line_comment() noexcept;

	void skip_multi_line_comment();


	void parse_quoted_token();

	void check_end_of_quoted_token();


	void parse_unquoted_token();


	void find_token();
}; // ScriptTokenizer

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs


#endif // !LTJS_SCRIPT_TOKENIZER_INCLUDED
