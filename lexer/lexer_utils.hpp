#ifndef LEXER_UTILS_HPP
#define LEXER_UTILS_HPP

#include <string>
#include <exception>

class utf8_parse_error: std::exception
{
public:
	virtual const char* what() const noexcept {
		return "Invalid UTF8 sequence.";
	}
};

/**
 * Fetches a single, complete UTF8 code point, returning it as a std::string.
 * Returns an empty string on a malformed codepoint.
 *
 * @param in  Reference to a const string iterator where the parsing is to begin.
 * @param end Reference to the corresponding end iterator for the string.
 *
 * TODO: throw an exception on malformed codepoints.
 */
static inline std::string cur_utf8(const std::string::const_iterator& in, const std::string::const_iterator& end)
{
	const unsigned char* c = reinterpret_cast<const unsigned char*>(&(*in));

	if (in == end)
		return std::string("");

	// Determine the length of the encoded codepoint
	int len = 0;
	if ((c[0] & 0b10000000) == 0b00000000)
		len = 1;
	else if ((c[0] & 0b11100000) == 0b11000000)
		len = 2;
	else if ((c[0] & 0b11110000) == 0b11100000)
		len = 3;
	else if ((c[0] & 0b11111000) == 0b11110000)
		len = 4;
	else if ((c[0] & 0b11111100) == 0b11111000)
		len = 5;
	else if ((c[0] & 0b11111110) == 0b11111100)
		len = 6;

	// Abort if malformed
	if (len == 0 || len > (end-in))
		throw utf8_parse_error {};

	// Read the rest of the bytes of the codepoint,
	// checking for malformed bytes.
	for (int i = 1; i < len; ++i) {
		if ((c[i] & 0b11000000) != 0b10000000)
			throw utf8_parse_error {}; // Abort, malformed
	}

	// Success!
	return std::string(in, in+len);
}

/**
 * Like cur_utf8, except it advances the string iterator after parsing the token.
 */
static inline std::string next_utf8(std::string::const_iterator& in, const std::string::const_iterator& end)
{
	std::string c = cur_utf8(in, end);

	in += c.length();

	return c;
}


/**
 * Returns whether the given utf character is whitespace or not.
 */
static inline bool is_ws_char(const std::string& s)
{
	if (s.length() == 0)
		return false;

	switch (s[0]) {
		case ' ':
		case '\t':
			return true;
		default:
			break;
	}

	return false;
}


/**
 * Returns whether the given utf character is a newline or not.
 */
static inline bool is_nl_char(const std::string& s)
{
	if (s.length() == 0)
		return false;

	switch (s[0]) {
		case '\n':
		case '\r':
			return true;
		default:
			break;
	}

	return false;
}


/**
 * Returns whether the given utf character is a newline or not.
 */
static inline bool is_comment_char(const std::string& s)
{
	if (s.length() == 0)
		return false;

	if (s[0] == '#')
		return true;
	else
		return false;
}


/**
 * Returns whether the given utf character is a reserved character or not.
 */
static inline bool is_reserved_char(const std::string& s)
{
	if (s.length() == 0)
		return false;

	switch (s[0]) {
		case '(':
		case ')':
		case '{':
		case '}':
		case '[':
		case ']':
		case '\\':
		case '"':
		case '\'':
		case '`':
		case ':':
		case ';':
		case '.':
		case ',':
			return true;
		default:
			break;
	}

	return false;
}


/**
 * Returns whether the given utf character is an operator character or not.
 */
static inline bool is_op_char(const std::string& s)
{
	if (s.length() != 1)
		return false;

	switch (s[0]) {
		case '=':
		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
		case '!':
		case '^':
		case '&':
		case '|':
		case '<':
		case '>':
		case '?':
		case '@':
		case '$':
		case '~':
			return true;
		default:
			break;
	}

	return false;
}


/**
 * Returns whether the given utf character is a numerical digit or not.
 */
static inline bool is_digit_char(const std::string& s)
{
	if (s.length() != 1)
		return false;

	switch (s[0]) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			return true;
		default:
			break;
	}

	return false;
}


/**
 * Returns whether the given utf character is a legal identifier character or not.
 */
static inline bool is_ident_char(const std::string& s)
{
	if (s.length() == 0)
		return false;

	// Anything that isn't whitespace, reserved, or an operator character
	if (!is_ws_char(s) && !is_nl_char(s) && !is_reserved_char(s) && !is_op_char(s))
		return true;

	return false;
}

#endif // LEXER_UTILS_HPP