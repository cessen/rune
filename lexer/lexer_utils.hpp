#ifndef LEXER_UTILS_HPP
#define LEXER_UTILS_HPP

#include <string>
#include <exception>
#include <cassert>

class utf8_parse_error: std::exception
{
public:
	virtual const char* what() const {
		return "Invalid UTF8 sequence.";
	}
};

/**
 * Fetches a single, complete UTF8 code point, placing it in cur_c.
 * Puts an empty string in cur_c when there's nothing left to read.
 *
 * @param cur_c Pointer to a std:string to fill in with the utf8 code point.
 * @param in  Reference to a const string iterator where the parsing is to begin.
 * @param end Reference to the corresponding end iterator for the string.
 *
 * Throws a utf8_parse_error exception on malformed utf8 input.
 */
static inline void cur_utf8(std::string *cur_c, const std::string::const_iterator& in, const std::string::const_iterator& end)
{
	assert(cur_c != nullptr);

	const unsigned char* c = reinterpret_cast<const unsigned char*>(&(*in));

	// Clear out cur_c
	cur_c->clear();

	if (in == end)
		return;

	// Determine the length of the encoded codepoint
	int len = 0;
	if (c[0] < 0x80)
		len = 1;
	else if (c[0] < 0xC0)
		throw utf8_parse_error {}; // Malformed: continuation byte as first byte
	else if (c[0] < 0xE0)
		len = 2;
	else if (c[0] < 0xF0)
		len = 3;
	else if (c[0] < 0xF8)
		len = 4;
	else
		throw utf8_parse_error {}; // Malformed: current utf8 standard only allows up to four bytes

	if (len == 0 || len > (end-in))
		throw utf8_parse_error {}; // Malformed: not enough bytes

	// Copy the bytes of the codepoint to cur_c,
	// making sure they're well-formed UTF8
	cur_c->push_back(c[0]);
	for (int i = 1; i < len; ++i) {
		if ((c[i] & 0xC0) != 0x80)
			throw utf8_parse_error {}; // Malformed: not a continuation byte

		cur_c->push_back(c[i]);
	}

	// Success!
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
		case '@':
		case '$':
		case '%':
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
		case '!':
		case '^':
		case '&':
		case '|':
		case '<':
		case '>':
		case '?':
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