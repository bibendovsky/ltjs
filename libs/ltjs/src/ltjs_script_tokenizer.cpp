#include "ltjs_script_tokenizer.h"

#include <cassert>

#include <string>

#include "ltjs_exception.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class ScriptTokenizerException :
	public Exception
{
public:
	explicit ScriptTokenizerException(
		const char* message)
		:
		Exception{"LTJS_SCRIPT_TOKENIZER", message}
	{
	}
}; // ScriptTokenizerException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

bool ScriptTokenizerToken::is_any_end_of() const noexcept
{
	return is_end_of_data || is_end_of_line;
}

bool ScriptTokenizerToken::is_escaped() const noexcept
{
	return is_quoted && size > unescaped_size;
}

bool ScriptTokenizerToken::is_empty() const noexcept
{
	return size <= 0;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

bool ScriptTokenizerLine::is_empty() const noexcept
{
	return size == 0;
}

ScriptTokenizerLine::operator bool() const noexcept
{
	return !is_end_of_data;
}

const ScriptTokenizerToken& ScriptTokenizerLine::operator[](
	Index index) const
{
	if (!tokens)
	{
		throw ScriptTokenizerException{"Null tokens."};
	}

	if (index < 0 || index >= size)
	{
		throw ScriptTokenizerException{"Token index out of range."};
	}

	return tokens[index];
}

ltjs::Index ScriptTokenizerLine::get_line_number() const
{
	if (is_empty())
	{
		throw ScriptTokenizerException{"No tokens."};
	}

	return tokens->line_number;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void ScriptTokenizer::initialize(
	const ScriptTokenizerInitParam& param)
{
	is_initialized_ = false;

	validate_param(param);

	is_initialized_ = true;

	data_ = param.data;
	size_ = param.size;
	offset_ = 0;
	line_number_ = 1;
	column_number_ = 1;
	token_ = {};
	state_ = State::none;
}

void ScriptTokenizer::tokenize(
	ScriptTokenizerToken& token)
{
	if (!is_initialized_)
	{
		throw ScriptTokenizerException{"Not initialized."};
	}

	token = {};
	token_ = &token;

	if (state_ != State::end_of_data)
	{
		state_ = State::find_token;
	}

	auto is_token_found = false;

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
				throw ScriptTokenizerException{"Unsupported state."};
		}
	}
}

void ScriptTokenizer::tokenize(
	ScriptTokenizerToken* tokens,
	Index token_count)
{
	if (!tokens)
	{
		throw ScriptTokenizerException{"Null tokens."};
	}

	if (token_count < 0)
	{
		throw ScriptTokenizerException{"Token count out of range."};
	}

	for (auto i = Index{}; i < token_count; ++i)
	{
		auto& token = tokens[i];
		tokenize(token);
	}
}

ScriptTokenizerLine ScriptTokenizer::tokenize_line(
	ScriptTokenizerToken* buffer,
	Index buffer_size,
	Index min_tokens,
	Index max_tokens)
{
	if (!buffer)
	{
		throw ScriptTokenizerException{"Null tokens."};
	}

	if (buffer_size <= 0)
	{
		throw ScriptTokenizerException{"Token count out of range."};
	}

	if (min_tokens <= 0)
	{
		throw ScriptTokenizerException{"Min token count out of range."};
	}

	if (max_tokens <= 0)
	{
		throw ScriptTokenizerException{"Max token count out of range."};
	}

	if (min_tokens > max_tokens)
	{
		throw ScriptTokenizerException{"Invalid token count range."};
	}

	if (buffer_size < max_tokens)
	{
		throw ScriptTokenizerException{"Token count out of range."};
	}

	auto result = ScriptTokenizerLine{};
	result.tokens = buffer;

	auto token = ScriptTokenizerToken{};

	while (true)
	{
		tokenize(token);

		if (token.is_any_end_of())
		{
			result.is_end_of_data = token.is_end_of_data;
			break;
		}

		if (result.size == max_tokens)
		{
			result.size += 1;
		}
		else if (result.size > max_tokens)
		{
			break;
		}
		else
		{
			buffer[result.size++] = token;
		}
	}

	if (result.size > 0 && (result.size < min_tokens || result.size > max_tokens))
	{
		const auto error_message = std::string{} +
			"Expected " + (
				min_tokens != max_tokens ?
				std::to_string(min_tokens) + '-' + std::to_string(max_tokens) :
				std::to_string(min_tokens)
			) + " tokens.";

		throw ScriptTokenizerException{error_message.c_str()};
	}

	return result;
}

Index ScriptTokenizer::escape_string(
	const char* src_string,
	Index src_string_size,
	char* dst_string,
	Index dst_string_size)
{
	assert(src_string);
	assert(src_string_size >= 0);
	assert(dst_string);
	assert(dst_string_size >= 0);

	const auto old_dst_string_size = dst_string_size;

	while (true)
	{
		if (src_string_size <= 0)
		{
			break;
		}

		const auto ch = (*src_string++);
		const auto byte = static_cast<unsigned char>(ch);

		auto escape_value = '\0';

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
					throw ScriptTokenizerException{"Unsupported char value."};
				}

				break;
		}

		const auto is_escape = (escape_value != '\0');
		const auto sequence_size = 1 + is_escape;

		if (sequence_size > dst_string_size)
		{
			throw ScriptTokenizerException{"Destination string too small."};
		}

		if (is_escape)
		{
			(*dst_string++) = '\\';
			(*dst_string++) = escape_value;
		}
		else
		{
			(*dst_string++) = ch;
		}

		src_string_size -= 1;
		dst_string_size -= sequence_size;
	}

	return old_dst_string_size - dst_string_size;
}

Index ScriptTokenizer::unescape_string(
	const char* src_string,
	Index src_string_size,
	char* dst_string,
	Index dst_string_size)
{
	assert(src_string);
	assert(src_string_size >= 0);
	assert(dst_string);
	assert(dst_string_size >= 0);

	auto src_string_offset = Index{};
	auto dst_string_offset = Index{};
	
	while (true)
	{
		if (src_string_offset >= src_string_size)
		{
			break;
		}

		if (dst_string_offset >= dst_string_size)
		{
			throw ScriptTokenizerException{"Destination string too small."};
		}

		const auto ch = src_string[src_string_offset];

		if (ch == '\\')
		{
			if ((src_string_offset + 1) >= src_string_size)
			{
				throw ScriptTokenizerException{"Unexpected end of source string."};
			}

			auto dst_ch = char{};
			const auto ch_1 = src_string[src_string_offset + 1];

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
					throw ScriptTokenizerException{"Unsupported escape sequence."};
			}

			src_string_offset += 2;

			dst_string[dst_string_offset] = dst_ch;
			dst_string_offset += 1;
		}
		else if (ch == '"')
		{
			throw ScriptTokenizerException{"Unexpected double quotaion mark."};
		}
		else if (is_control_code(ch))
		{
			throw ScriptTokenizerException{"Control character."};
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

Index ScriptTokenizer::unescape_string(
	const ScriptTokenizerToken& token,
	char* dst_string,
	Index dst_max_string_size)
{
	if (token.is_any_end_of())
	{
		throw ScriptTokenizerException{"Not a string."};
	}

	return unescape_string(
		token.data,
		token.size,
		dst_string,
		dst_max_string_size
	);
}

void ScriptTokenizer::validate_param(
	const ScriptTokenizerInitParam& param)
{
	if (!param.data)
	{
		throw ScriptTokenizerException{"Null data."};
	}

	if (param.size < 0)
	{
		throw ScriptTokenizerException{"Size out of range."};
	}
}

[[noreturn]]
void ScriptTokenizer::fail(
	const char* message) const
{
	assert(message);

	const auto error_message =
		std::string{} +
		'[' + std::to_string(line_number_) + ':' + std::to_string(column_number_) + "] " +
		message
	;

	throw ScriptTokenizerException{error_message.c_str()};
}

bool ScriptTokenizer::is_end_of_data() const noexcept
{
	return offset_ >= size_;
}

char ScriptTokenizer::peek(
	Index offset) const noexcept
{
	const auto new_offset = offset_ + offset;

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

bool ScriptTokenizer::is_control_code(
	char ch) noexcept
{
	const auto byte = static_cast<unsigned char>(ch);

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

void ScriptTokenizer::advance_data(
	Index count) noexcept
{
	assert(count > 0);

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
	token_->data = data_ + offset_;
}

void ScriptTokenizer::set_unquoted() noexcept
{
	token_->data = data_ + offset_;
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
	if (false)
	{
	}
	else if (is_end_of_data())
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
	if (false)
	{
	}
	else if (is_end_of_data())
	{
		fail("Unclosed multi-line comment.");
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
	if (false)
	{
	}
	else if (is_end_of_data() || is_end_of_line())
	{
		fail("Unclosed string.");
	}
	else if (is_control_code())
	{
		fail("Unexpected control character.");
	}
	else if (is_backslash())
	{
		switch (peek(1))
		{
			case '"':
			case 'n':
			case '\\':
				token_->size += 2;
				token_->unescaped_size += 1;

				advance_data(2);
				break;

			default:
				fail("Unsupported escape sequence.");
		}
	}
	else if (is_double_quotation_mark())
	{
		advance_data(1);
		state_ = State::check_end_of_quoted_token;
	}
	else
	{
		token_->size += 1;
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
		fail("Unexpected character after a string.");
	}

	state_ = State::quoted_token;
}

void ScriptTokenizer::parse_unquoted_token()
{
	if (false)
	{
	}
	else if (
		is_end_of_data() ||
		is_end_of_line() ||
		is_whitespace())
	{
		state_ = State::unquoted_token;
	}
	else if (is_control_code())
	{
		fail("Unexpected control character.");
	}
	else
	{
		advance_data(1);
		token_->size += 1;
	}
}

void ScriptTokenizer::find_token()
{
	if (false)
	{
	}
	else if (is_end_of_data())
	{
		state_ = State::end_of_data;
	}
	else if (is_end_of_line())
	{
		state_ = State::end_of_line;
	}
	else if (is_control_code())
	{
		fail("Unexpected control character.");
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
		fail("Unopen multi-line comment.");
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

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs
