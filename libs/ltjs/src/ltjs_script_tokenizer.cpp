/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Script tokenizer

#include "ltjs_script_tokenizer.h"
#include "ltjs_exception.h"
#include <cassert>
#include <format>

namespace ltjs {

bool ScriptTokenizerToken::is_any_end_of() const noexcept
{
	return is_end_of_data || is_end_of_line;
}

bool ScriptTokenizerToken::is_escaped() const noexcept
{
	return is_quoted && static_cast<int>(data.size()) > unescaped_size;
}

bool ScriptTokenizerToken::is_empty() const noexcept
{
	return data.empty();
}

// =====================================

[[noreturn]] void ScriptTokenizerLine::fail(std::string_view message)
{
	throw Exception{"LTJS_SCRIPT_TOKENIZER_LINE", message};
}

bool ScriptTokenizerLine::is_empty() const noexcept
{
	return token_count == 0;
}

ScriptTokenizerLine::operator bool() const noexcept
{
	return !is_end_of_data;
}

const ScriptTokenizerToken& ScriptTokenizerLine::operator[](int index) const
{
	if (index < 0 || index >= token_count)
	{
		fail("Token index out of range.");
	}
	return tokens[index];
}

int ScriptTokenizerLine::get_line_number() const
{
	return tokens->line_number;
}

// =====================================

void ScriptTokenizer::initialize(const ScriptTokenizerInitParam& param)
{
	data_ = param.data.data();
	size_ = static_cast<int>(param.data.size());
	offset_ = 0;
	line_number_ = 1;
	column_number_ = 1;
	state_ = State::none;
	token_ = nullptr;
}

void ScriptTokenizer::tokenize(ScriptTokenizerToken& token)
{
	token = ScriptTokenizerToken{};
	token_ = &token;
	if (state_ != State::end_of_data)
	{
		state_ = State::find_token;
	}
	bool is_token_found = false;
	while (!is_token_found)
	{
		switch (state_)
		{
			case State::find_token:
				find_token();
				break;
			case State::end_of_data:
				is_token_found = true;
				set_end_of_data();
				break;
			case State::end_of_line:
				is_token_found = true;
				set_end_of_line();
				skip_end_of_line();
				break;
			case State::skip_single_line_comment:
				skip_single_line_comment();
				break;
			case State::skip_multi_line_comment:
				skip_multi_line_comment();
				break;
			case State::parse_quoted_token:
				parse_quoted_token();
				break;
			case State::check_end_of_quoted_token:
				check_end_of_quoted_token();
				break;
			case State::quoted_token:
				is_token_found = true;
				break;
			case State::parse_unquoted_token:
				parse_unquoted_token();
				break;
			case State::unquoted_token:
				is_token_found = true;
				break;
			default:
				fail("Unknown state.");
		}
	}
}

void ScriptTokenizer::tokenize(ScriptTokenizerToken* tokens, int token_count)
{
	assert(tokens != nullptr);
	assert(token_count >= 0);
	for (int i = 0; i < token_count; ++i)
	{
		tokenize(tokens[i]);
	}
}

ScriptTokenizerLine ScriptTokenizer::tokenize_line(ScriptTokenizerToken* buffer, int buffer_size, int min_tokens, int max_tokens)
{
	assert(buffer != nullptr);
	assert(buffer_size >= 0);
	assert(min_tokens >= 0);
	assert(max_tokens >= 0);
	assert(min_tokens <= max_tokens);
	ScriptTokenizerLine result{};
	result.tokens = buffer;
	ScriptTokenizerToken token{};
	for (;;)
	{
		tokenize(token);
		if (token.is_any_end_of())
		{
			result.is_end_of_data = token.is_end_of_data;
			break;
		}
		if (result.token_count == max_tokens)
		{
			result.token_count += 1;
		}
		else if (result.token_count > max_tokens)
		{
			break;
		}
		else
		{
			buffer[result.token_count++] = token;
		}
	}
	if (result.token_count > 0 && (result.token_count < min_tokens || result.token_count > max_tokens))
	{
		constexpr int max_chars = 63;
		char chars[max_chars + 1];
		if (min_tokens != max_tokens)
		{
			const auto [chars_end, char_count] = std::format_to_n(chars, max_chars, "Expected {}-{} tokens.", min_tokens, max_tokens);
			*chars_end = '\0';
		}
		else
		{
			const auto [chars_end, char_count] = std::format_to_n(chars, max_chars, "Expected {} tokens.", min_tokens);
			*chars_end = '\0';
		}
		fail(chars);
	}
	return result;
}

int ScriptTokenizer::escape_string(const char* src_string, int src_string_size, char* dst_string, int dst_string_size)
{
	assert(src_string != nullptr);
	assert(src_string_size >= 0);
	assert(dst_string != nullptr);
	assert(dst_string_size >= 0);
	const int old_dst_string_size = dst_string_size;
	for (;;)
	{
		if (src_string_size <= 0)
		{
			break;
		}
		const char ch = *src_string++;
		const unsigned char byte = static_cast<unsigned char>(ch);
		char escape_value = '\0';
		switch (ch)
		{
			case '"':
				escape_value = '"';
				break;
			case '\n':
				escape_value = 'n';
				break;
			case '\\':
				escape_value = '\\';
				break;
			default:
				if (byte < 0x20 || byte == 0x7E)
				{
					fail("Unsupported char value.");
				}
				break;
		}
		const bool is_escape = (escape_value != '\0');
		const int sequence_size = 1 + is_escape;
		if (sequence_size > dst_string_size)
		{
			fail("Destination string too small.");
		}
		if (is_escape)
		{
			*dst_string++ = '\\';
			*dst_string++ = escape_value;
		}
		else
		{
			*dst_string++ = ch;
		}
		src_string_size -= 1;
		dst_string_size -= sequence_size;
	}
	return old_dst_string_size - dst_string_size;
}

int ScriptTokenizer::unescape_string(const char* src_string, int src_string_size, char* dst_string, int dst_string_size)
{
	assert(src_string != nullptr);
	assert(src_string_size >= 0);
	assert(dst_string != nullptr);
	assert(dst_string_size >= 0);
	int src_string_offset = 0;
	int dst_string_offset = 0;
	for (;;)
	{
		if (src_string_offset >= src_string_size)
		{
			break;
		}
		if (dst_string_offset >= dst_string_size)
		{
			fail("Destination string too small.");
		}
		const char ch = src_string[src_string_offset];
		if (ch == '\\')
		{
			if (src_string_offset + 1 >= src_string_size)
			{
				fail("Unexpected end of source string.");
			}
			char dst_ch = '\0';
			const char ch_1 = src_string[src_string_offset + 1];
			switch (ch_1)
			{
				case '"':
					dst_ch = '"';
					break;
				case 'n':
					dst_ch = '\n';
					break;
				case '\\':
					dst_ch = '\\';
					break;
				default:
					fail("Unsupported escape sequence.");
			}
			src_string_offset += 2;
			dst_string[dst_string_offset] = dst_ch;
			dst_string_offset += 1;
		}
		else if (ch == '"')
		{
			fail("Unexpected double quotaion mark.");
		}
		else if (is_control_code(ch))
		{
			fail("Control character.");
		}
		else
		{
			src_string_offset += 1;
			dst_string[dst_string_offset] = ch;
			dst_string_offset += 1;
		}
	}
	return dst_string_offset;
}

int ScriptTokenizer::unescape_string(const ScriptTokenizerToken& token, char* dst_string, int dst_max_string_size)
{
	assert(dst_string != nullptr);
	assert(dst_max_string_size >= 0);
	if (token.is_any_end_of())
	{
		fail("No string.");
	}
	return unescape_string(token.data.data(), static_cast<int>(token.data.size()), dst_string, dst_max_string_size);
}

[[noreturn]] void ScriptTokenizer::fail(std::string_view message)
{
	throw Exception{"LTJS_SCRIPT_TOKENIZER", message};
}

[[noreturn]] void ScriptTokenizer::fail_at(std::string_view message) const
{
	constexpr int max_chars = 255;
	char chars[max_chars + 1];
	const auto [chars_end, char_count] = std::format_to_n(chars, max_chars, "[{}:{}] {}", line_number_, column_number_, message);
	*chars_end = '\0';
	fail(chars);
}

bool ScriptTokenizer::is_end_of_data() const noexcept
{
	return offset_ >= size_;
}

char ScriptTokenizer::peek(int offset) const noexcept
{
	const int new_offset = offset_ + offset;
	if (new_offset < 0 || new_offset >= size_)
	{
		return '\0';
	}
	return data_[new_offset];
}

bool ScriptTokenizer::is_end_of_line() const noexcept
{
	switch (peek(0))
	{
		case '\n':
		case '\r':
			return true;
		default:
			return false;
	}
}

bool ScriptTokenizer::is_windows_end_of_line() const noexcept
{
	return peek(0) == '\r' && peek(1) == '\n';
}

bool ScriptTokenizer::is_backslash() const noexcept
{
	return peek(0) == '\\';
}

bool ScriptTokenizer::is_whitespace() const noexcept
{
	switch (peek(0))
	{
		case ' ':
		case '\t':
			return true;
		default:
			return false;
	}
}

bool ScriptTokenizer::is_control_code(char ch) noexcept
{
	const unsigned char byte = static_cast<unsigned char>(ch);
	return byte < 0x20 || byte == 0x7F;
}

bool ScriptTokenizer::is_control_code() const noexcept
{
	return is_control_code(peek(0));
}

bool ScriptTokenizer::is_double_quotation_mark() const noexcept
{
	return peek(0) == '"';
}

bool ScriptTokenizer::is_single_line_comment() const noexcept
{
	return peek(0) == '/' && peek(1) == '/';
}

bool ScriptTokenizer::is_multi_line_comment_begin() const noexcept
{
	return peek(0) == '/' && peek(1) == '*';
}

bool ScriptTokenizer::is_multi_line_comment_end() const noexcept
{
	return peek(0) == '*' && peek(1) == '/';
}

void ScriptTokenizer::advance_data(int count) noexcept
{
	offset_ += count;
	column_number_ += count;
}

void ScriptTokenizer::set_end_of_data() noexcept
{
	token_->is_end_of_data = true;
}

void ScriptTokenizer::set_end_of_line() noexcept
{
	token_->is_end_of_line = true;
}

void ScriptTokenizer::set_quoted() noexcept
{
	token_->is_quoted = true;
	token_->data = std::string_view{data_ + offset_, token_->data.size()};
}

void ScriptTokenizer::set_unquoted() noexcept
{
	token_->data = std::string_view{data_ + offset_, token_->data.size()};
}

void ScriptTokenizer::skip_end_of_line()
{
	line_number_ += 1;
	column_number_ = 1;
	if (is_windows_end_of_line())
	{
		offset_ += 2;
	}
	else
	{
		offset_ += 1;
	}
}

void ScriptTokenizer::skip_single_line_comment() noexcept
{
	if (is_end_of_data())
	{
		state_ = State::end_of_data;
	}
	else if (is_end_of_line())
	{
		state_ = State::end_of_line;
	}
	else
	{
		advance_data(1);
	}
}

void ScriptTokenizer::skip_multi_line_comment()
{
	if (is_end_of_data())
	{
		fail_at("Unclosed multi-line comment.");
	}
	else if (is_end_of_line())
	{
		skip_end_of_line();
	}
	else if (is_multi_line_comment_end())
	{
		advance_data(2);
		state_ = State::find_token;
	}
	else
	{
		advance_data(1);
	}
}

void ScriptTokenizer::parse_quoted_token()
{
	if (is_end_of_data() || is_end_of_line())
	{
		fail_at("Unclosed string.");
	}
	else if (is_control_code())
	{
		fail_at("Unexpected control character.");
	}
	else if (is_backslash())
	{
		switch (peek(1))
		{
			case '"':
			case 'n':
			case '\\':
				token_->data = std::string_view{token_->data.data(), token_->data.size() + 2};
				token_->unescaped_size += 1;
				advance_data(2);
				break;
			default:
				fail_at("Unsupported escape sequence.");
		}
	}
	else if (is_double_quotation_mark())
	{
		advance_data(1);
		state_ = State::check_end_of_quoted_token;
	}
	else
	{
		token_->data = std::string_view{token_->data.data(), token_->data.size() + 1};
		token_->unescaped_size += 1;
		advance_data(1);
	}
}

void ScriptTokenizer::check_end_of_quoted_token()
{
	if (!is_end_of_data() &&
		!is_end_of_line() &&
		!is_whitespace() &&
		!is_single_line_comment() &&
		!is_multi_line_comment_begin())
	{
		fail_at("Unexpected character after a string.");
	}
	state_ = State::quoted_token;
}

void ScriptTokenizer::parse_unquoted_token()
{
	if (is_end_of_data() || is_end_of_line() || is_whitespace())
	{
		state_ = State::unquoted_token;
	}
	else if (is_control_code())
	{
		fail_at("Unexpected control character.");
	}
	else
	{
		advance_data(1);
		token_->data = std::string_view{token_->data.data(), token_->data.size() + 1};
	}
}

void ScriptTokenizer::find_token()
{
	if (is_end_of_data())
	{
		state_ = State::end_of_data;
	}
	else if (is_end_of_line())
	{
		state_ = State::end_of_line;
	}
	else if (is_control_code())
	{
		fail_at("Unexpected control character.");
	}
	else if (is_single_line_comment())
	{
		advance_data(2);
		state_ = State::skip_single_line_comment;
	}
	else if (is_multi_line_comment_begin())
	{
		advance_data(2);
		state_ = State::skip_multi_line_comment;
	}
	else if (is_multi_line_comment_end())
	{
		fail_at("Unopen multi-line comment.");
	}
	else if (is_whitespace())
	{
		advance_data(1);
	}
	else if (is_double_quotation_mark())
	{
		token_->line_number = line_number_;
		token_->column_number = column_number_;
		advance_data(1);
		set_quoted();
		state_ = State::parse_quoted_token;
	}
	else
	{
		token_->line_number = line_number_;
		token_->column_number = column_number_;
		set_unquoted();
		state_ = State::parse_unquoted_token;
	}
}

} // namespace ltjs
