//
// Simple tokenizer for resource strings, configuration files, etc.
//


#include <algorithm>
#include <string>
#include <utility>
#include "ltjs_script_tokenizer.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// ScriptLine

ScriptLine::ScriptLine()
	:
	number_{},
	tokens_{}
{
}

// ScriptLine
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// ScriptTokenizer::Impl

class ScriptTokenizer::Impl final
{
public:
	Impl();

	~Impl();


	// Splits strings into tokens.
	bool impl_tokenize(
		const char* const string,
		const int string_size);

	// Returns an error message.
	const std::string& impl_get_error_message() const;

	// Returns lines of tokens.
	const ScriptLines& impl_get_lines() const;


private:
	const char* string_;
	int string_size_;
	int string_index_;
	int line_number_;
	int line_column_;
	ScriptToken script_token_;
	ScriptLine script_line_;
	ScriptLines script_lines_;

	std::string error_message_;


	// Indicated the end of the string.
	bool is_eos() const;

	// Returns a character in the string at the specified offset of
	// the current position.
	// Returns '\0' if index out of range.
	char peek_char(
		const int offset = 0) const;

	// Returns true if there is a whitespace at the current position.
	bool is_whitespace() const;

	// Returns true if there is a double quotation mark at the current position.
	bool is_double_quotes() const;

	// Returns true if there is a new line at the current position.
	bool is_new_line() const;

	// Returns true if there is a comment at the current position.
	bool is_comment() const;

	// Returns true if there is a short comment at the current position.
	bool is_short_comment() const;

	// Returns true if there is beginning of a long comment at the current position.
	bool is_long_comment_begin() const;

	// Returns true if there is ending of a long comment at the current position.
	bool is_long_comment_end() const;

	// Advances current index of the string by specified count.
	// Throws exception if count zero or negative.
	void advance_char(
		const int count = 1);

	// Flushes line's buffer.
	void flush_line();

	// Adds a new line of tokens.
	void advance_line();

	// Looks up for a token.
	bool find_token();

	// Skips consequent whitespaces.
	void skip_whitespaces();

	// Skips consequent new lines.
	void skip_new_lines();

	// Skips consequent comments.
	// Returns true on success or false otherwise.
	bool skip_comments();

	// Parses current token.
	// Returns true on success and false otherwise.
	bool parse_token();

	// Sets an error message with a current line number and line column.
	void set_error(
		const char* const error_message);

	// Sets an error message with a specified line number and line column.
	void set_error(
		const int line_number,
		const int line_column,
		const char* const message);

	// Formats an error or warning message with a specified line number and line column.
	static std::string format_issue_string(
		const int line_number,
		const int line_column,
		const char* const message);
}; // ScriptTokenizer

ScriptTokenizer::Impl::Impl()
	:
	string_{},
	string_size_{},
	string_index_{},
	line_number_{},
	line_column_{},
	script_token_{},
	script_line_{},
	script_lines_{},
	error_message_{}
{
}

ScriptTokenizer::Impl::~Impl()
{
}

bool ScriptTokenizer::Impl::impl_tokenize(
	const char* const string,
	const int string_size)
{
	error_message_ = {};

	if (!string || string_size == 0)
	{
		return true;
	}

	string_ = string;
	string_size_ = string_size;
	string_index_ = 0;
	line_number_ = 1;
	line_column_ = 1;
	script_token_ = {};
	script_line_ = {};
	script_lines_ = {};

	while (find_token())
	{
		if (!parse_token())
		{
			break;
		}
	}

	if (error_message_.empty())
	{
		flush_line();
	}

	return error_message_.empty();
}

const std::string& ScriptTokenizer::Impl::impl_get_error_message() const
{
	return error_message_;
}

const ScriptLines& ScriptTokenizer::Impl::impl_get_lines() const
{
	return script_lines_;
}

bool ScriptTokenizer::Impl::is_eos() const
{
	return string_index_ >= string_size_;
}

char ScriptTokenizer::Impl::peek_char(
	const int offset) const
{
	const auto new_string_index = string_index_ + offset;

	if (new_string_index < 0 || new_string_index >= string_size_)
	{
		return '\0';
	}

	return string_[new_string_index];
}

bool ScriptTokenizer::Impl::is_whitespace() const
{
	if (is_eos())
	{
		return false;
	}

	switch (peek_char())
	{
	case ' ':
	case '\t':
		return true;

	default:
		return false;
	}
}

bool ScriptTokenizer::Impl::is_double_quotes() const
{
	return peek_char() == '\"';
}

bool ScriptTokenizer::Impl::is_new_line() const
{
	switch (peek_char())
	{
	case '\n':
	case '\r':
		return true;

	default:
		return false;
	}
}

bool ScriptTokenizer::Impl::is_comment() const
{
	return is_short_comment() || is_long_comment_begin();
}

bool ScriptTokenizer::Impl::is_short_comment() const
{
	return peek_char(0) == '/' && peek_char(1) == '/';
}

bool ScriptTokenizer::Impl::is_long_comment_begin() const
{
	return peek_char(0) == '/' && peek_char(1) == '*';
}

bool ScriptTokenizer::Impl::is_long_comment_end() const
{
	return peek_char(0) == '*' && peek_char(1) == '/';
}

void ScriptTokenizer::Impl::advance_char(
	const int count)
{
	if (count <= 0)
	{
		throw "Negative or zero count.";
	}

	if (!is_eos())
	{
		string_index_ += count;
		line_column_ += count;
	}
}

void ScriptTokenizer::Impl::flush_line()
{
	if (!script_line_.tokens_.empty())
	{
		script_lines_.push_back(script_line_);
		script_line_ = {};
	}
}

void ScriptTokenizer::Impl::advance_line()
{
	line_number_ += 1;
	line_column_ = 1;

	flush_line();
}

bool ScriptTokenizer::Impl::skip_comments()
{
	if (is_short_comment())
	{
		while (!is_eos())
		{
			if (!is_short_comment())
			{
				return true;
			}

			while (!is_eos() && !is_new_line())
			{
				advance_char();
			}

			skip_new_lines();
		}

		return true;
	}
	else if (is_long_comment_begin())
	{
		const auto error_line_number = line_number_;
		const auto error_line_column = line_column_;

		advance_char(2);

		while (!is_eos() && !is_long_comment_end())
		{
			if (is_new_line())
			{
				skip_new_lines();
			}
			else
			{
				advance_char();
			}
		}

		if (is_eos())
		{
			set_error(error_line_number, error_line_column, "Unclosed long comment.");
			return false;
		}
		else
		{
			advance_char(2);
			skip_new_lines();
		}
	}

	return true;
}

bool ScriptTokenizer::Impl::find_token()
{
	while (!is_eos())
	{
		if (is_whitespace())
		{
			skip_whitespaces();
		}
		else if (is_new_line())
		{
			skip_new_lines();
		}
		else if (is_comment())
		{
			if (!skip_comments())
			{
				return false;
			}
		}
		else
		{
			return true;
		}
	}

	return false;
}

void ScriptTokenizer::Impl::skip_whitespaces()
{
	while (is_whitespace())
	{
		advance_char();
	}
}

void ScriptTokenizer::Impl::skip_new_lines()
{
	while (!is_eos())
	{
		if (peek_char(0) == '\r' && peek_char(1) == '\n')
		{
			// CR and LF
			advance_char(2);
			advance_line();
		}
		else if (is_new_line())
		{
			// CR or LF
			advance_char();
			advance_line();
		}
		else
		{
			break;
		}
	}
}

bool ScriptTokenizer::Impl::parse_token()
{
	script_token_ = {};

	auto& content = script_token_.content_;
	auto is_quoted = is_double_quotes();
	const auto start_line_number = line_number_;
	const auto start_line_column = line_column_;

	if (is_quoted)
	{
		advance_char();
	}

	while (!is_eos())
	{
		if (is_quoted)
		{
			const auto current_char = peek_char();

			if (current_char == '\"')
			{
				if (peek_char(1) == '\"')
				{
					content += '\"';
					advance_char(2);
				}
				else
				{
					advance_char();
					break;
				}
			}
			else if (current_char == '\\')
			{
				switch (peek_char(1))
				{
				case 'n':
					content += '\n';
					advance_char(2);
					break;

				case '\"':
					content += '\"';
					advance_char(2);
					break;

				case '\\':
					content += '\\';
					advance_char(2);
					break;

				default:
					set_error("Unsupported escape character.");
					return false;
				}
			}
			else if (is_new_line())
			{
				set_error("Unclosed string.");
				return false;
			}
			else if (current_char == '\t')
			{
				set_error("Tab not supported inside a string.");
				return false;
			}
			else
			{
				content += current_char;
				advance_char();
			}
		}
		else
		{
			if (is_whitespace() || is_new_line() || is_comment())
			{
				break;
			}
			else
			{
				content += peek_char();
				advance_char();
			}
		}
	}

	if (is_quoted)
	{
		if (is_double_quotes())
		{
			set_error("Redundant double quotation marks.");
			return false;
		}

		if (!(is_eos() || is_whitespace() || is_new_line() || is_comment()))
		{
			set_error("Expected whitespace, comment or newline.");
			return false;
		}
	}
	else
	{
		if (content.empty())
		{
			set_error("Empty token.");
			return false;
		}
	}

	script_line_.number_ = line_number_;
	script_line_.tokens_.emplace_back(script_token_);

	return true;
}

void ScriptTokenizer::Impl::set_error(
	const char* const error_message)
{
	set_error(line_number_, line_column_, error_message);
}

void ScriptTokenizer::Impl::set_error(
	const int line_number,
	const int line_column,
	const char* const message)
{
	error_message_ = format_issue_string(line_number, line_column, message);
}

std::string ScriptTokenizer::Impl::format_issue_string(
	const int line_number,
	const int line_column,
	const char* const message)
{
	return std::to_string(line_number) + ":" + std::to_string(line_column) + ": " + message;
}

// ScriptTokenizer::Impl
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// ScriptTokenizer

ScriptTokenizer::ScriptTokenizer()
	:
	impl_{std::make_unique<Impl>()}
{
}

ScriptTokenizer::ScriptTokenizer(
	ScriptTokenizer&& rhs)
	:
	impl_{std::move(rhs.impl_)}
{
}

ScriptTokenizer::~ScriptTokenizer()
{
}

bool ScriptTokenizer::tokenize(
	const char* const string,
	const int string_size)
{
	return impl_->impl_tokenize(string, string_size);
}

const std::string& ScriptTokenizer::get_error_message() const
{
	return impl_->impl_get_error_message();
}

const ScriptLines& ScriptTokenizer::get_lines() const
{
	return impl_->impl_get_lines();
}

// ScriptTokenizer
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs
