#include "lexer.hpp"

#include <iostream>
#include <string>


/**
 * Fetches a single, complete UTF8 code point, returning it as a std::string.
 * Returns an empty string on EOF or a malformed codepoint.
 */
std::string next_utf8(std::istream &in)
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


bool is_whitespace(const std::string& s)
{
	if (s.length() == 0)
		return false;

	if (s[0] == ' ' || s[0] == '\n' || s[0] == '\r' || s[0] == '\t')
		return true;
	else
		return false;
}

bool is_identifier_char(const std::string& s)
{
	if (s.length() == 0)
		return false;

	if (s.length() > 1) // Any non-ascii UTF8 codepoints can be used in identifiers
		return true;

	if ((s[0] >= 'a' && s[0] <= 'z') || (s[0] >= 'A' && s[0] <= 'Z')) // Latin characters are okay
		return true;

	if (s[0] == '_') // Underscores are great
		return true;

	return false;
}

bool is_op_char(const std::string& s)
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
			return false;
	}
}


std::string Lexer::next()
{
	std::string token = "";

	// Skip whitespace
	while (is_whitespace(cur_c)) {
		cur_c = next_utf8(in);
	}

	// If it's an identifier
	if (is_identifier_char(cur_c)) {
		do {
			token.append(cur_c);
			cur_c = next_utf8(in);
		} while (is_identifier_char(cur_c));

		current_token = token;
		current_token_type = IDENTIFIER;

		return token;
	}

	// If it's an operator
	if (is_op_char(cur_c)) {
		do {
			token.append(cur_c);
			cur_c = next_utf8(in);
		} while (is_op_char(cur_c));

		current_token = token;
		current_token_type = OPERATOR;

		return token;
	}

	// If it's something else
	token = cur_c;
	cur_c = next_utf8(in);

	return token;
}

