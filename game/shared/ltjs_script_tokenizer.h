//
// Simple tokenizer for resource strings, configuration files, etc.
//


#ifndef LTJS_SCRIPT_TOKENIZER_INCLUDED
#define LTJS_SCRIPT_TOKENIZER_INCLUDED


#include <string>
#include <deque>
#include <memory>


namespace ltjs
{


// The token.
struct ScriptToken final
{
	// Token's content.
	std::string content_;
}; // ScriptToken

using ScriptTokens = std::deque<ScriptToken>;


// A container for all tokens of one line.
struct ScriptLine final
{
	// The line's number in the script.
	int number_;

	// Token list.
	ScriptTokens tokens_;


	ScriptLine();
}; // ScriptLine

using ScriptLines = std::deque<ScriptLine>;


class ScriptTokenizer final
{
public:
	ScriptTokenizer();

	ScriptTokenizer(
		ScriptTokenizer&& rhs);

	~ScriptTokenizer();


	// Splits strings into tokens.
	bool tokenize(
		const char* const string,
		const int string_size);

	// Returns an error message.
	const std::string& get_error_message() const;

	// Returns lines of tokens.
	const ScriptLines& get_lines() const;

	//
	// Replaces special whitespace characters (new line, double quote) with escaped ones.
	//
	// Parameters:
	//    - string - string to modify.
	//
	// Returns:
	//    - Escaped string.
	//
	static std::string escape_string(
		const std::string& string);


private:
	class Impl;

	using ImplUPtr = std::unique_ptr<Impl>;

	ImplUPtr impl_;
}; // ScriptTokenizer


} // ltjs


#endif // LTJS_SCRIPT_TOKENIZER_INCLUDED
