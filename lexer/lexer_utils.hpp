#ifndef LEXER_UTILS_HPP
#define LEXER_UTILS_HPP

#include <string>
#include <iostream>

/**
 * Fetches a single, complete UTF8 code point, returning it as a std::string.
 * Returns an empty string on EOF or a malformed codepoint.
 */
static inline std::string next_utf8(std::istream &in)
{
	char c[6] = {0, 0, 0, 0, 0, 0};

	// Read in the first byte
	in.read(c, 1);
	if (in.gcount() == 0)
		return std::string("");

	// Determine the length of the encoded codepoint
	const unsigned char first = reinterpret_cast<unsigned char&>(c[0]);
	int length = 0;
	if ((first & 0b10000000) == 0b00000000)
		length = 1;
	else if ((first & 0b11100000) == 0b11000000)
		length = 2;
	else if ((first & 0b11110000) == 0b11100000)
		length = 3;
	else if ((first & 0b11111000) == 0b11110000)
		length = 4;
	else if ((first & 0b11111100) == 0b11111000)
		length = 5;
	else if ((first & 0b11111110) == 0b11111100)
		length = 6;

	// Abort if malformed
	if (length == 0)
		return std::string("");

	// Read the rest of the bytes of the codepoint,
	// checking for malformed bytes and EOF along the way.
	for (int i = 1; i < length ; ++i) {
		in.read(&(c[i]), 1);
		if (in.gcount() == 0)
			return std::string("");  // Abort, malformed
		if ((reinterpret_cast<unsigned char&>(c[i]) & 0b11000000) != 0b10000000)
			return std::string("");  // Abort, malformed
	}

	// Success!
	return std::string(c);
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
		case '\n':
		case '\r':
		case '\t':
			return true;
		default:
			break;
	}

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
		case '#':
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
	if (!is_ws_char(s) && !is_reserved_char(s) && !is_op_char(s))
		return true;

	return false;
}

#endif // LEXER_UTILS_HPP