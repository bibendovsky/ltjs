/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Script tokenizer

#ifndef LTJS_SCRIPT_TOKENIZER_INCLUDED
#define LTJS_SCRIPT_TOKENIZER_INCLUDED

#include <string_view>

namespace ltjs {

struct ScriptTokenizerToken
{
	bool is_end_of_data;
	bool is_end_of_line;
	bool is_quoted;

	int line_number;
	int column_number;

	std::string_view data;
	int unescaped_size;

	bool is_any_end_of() const noexcept;
	bool is_escaped() const noexcept;
	bool is_empty() const noexcept;
};

// =====================================

struct ScriptTokenizerLine
{
	bool is_end_of_data;
	const ScriptTokenizerToken* tokens;
	int token_count;

	[[noreturn]] static void fail(std::string_view message);
	bool is_empty() const noexcept;
	explicit operator bool() const noexcept;
	const ScriptTokenizerToken& operator[](int index) const;
	int get_line_number() const;
};

// =====================================

struct ScriptTokenizerInitParam
{
	std::string_view data;
};

class ScriptTokenizer
{
public:
	void initialize(const ScriptTokenizerInitParam& param);
	void tokenize(ScriptTokenizerToken& token);
	void tokenize(ScriptTokenizerToken* tokens, int token_count);
	ScriptTokenizerLine tokenize_line(ScriptTokenizerToken* buffer, int buffer_size, int min_tokens, int max_tokens);
	static int escape_string(const char* src_string, int src_string_size, char* dst_string, int dst_string_size);
	static int unescape_string(const char* src_string, int src_string_size, char* dst_string, int dst_string_size);
	static int unescape_string(const ScriptTokenizerToken& token, char* dst_string, int dst_max_string_size);

private:
	enum class State
	{
		none = 0,

		find_token,
		end_of_data,
		end_of_line,
		skip_single_line_comment,
		skip_multi_line_comment,
		parse_quoted_token,
		check_end_of_quoted_token,
		quoted_token,
		parse_unquoted_token,
		unquoted_token
	};

	const char* data_{};
	int size_{};
	int offset_{};
	int line_number_{};
	int column_number_{};
	State state_{};
	ScriptTokenizerToken* token_{};

	[[noreturn]] static void fail(std::string_view message);
	[[noreturn]] void fail_at(std::string_view message) const;
	bool is_end_of_data() const noexcept;
	char peek(int offset) const noexcept;
	bool is_end_of_line() const noexcept;
	bool is_windows_end_of_line() const noexcept;
	bool is_backslash() const noexcept;
	bool is_whitespace() const noexcept;
	static bool is_control_code(char ch) noexcept;
	bool is_control_code() const noexcept;
	bool is_double_quotation_mark() const noexcept;
	bool is_single_line_comment() const noexcept;
	bool is_multi_line_comment_begin() const noexcept;
	bool is_multi_line_comment_end() const noexcept;

	void advance_data(int count) noexcept;
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
};

} // namespace ltjs

#endif // LTJS_SCRIPT_TOKENIZER_INCLUDED
