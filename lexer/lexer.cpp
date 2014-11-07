#include "lexer.hpp"
#include "lexer_utils.hpp"
#include "tokens.hpp"

#include <iostream>
#include <string>
#include <vector>


Token lex_string_literal(std::string::const_iterator& str_iter, const std::string::const_iterator& str_itr_end);
Token lex_comment(std::string::const_iterator& str_iter, const std::string::const_iterator& str_itr_end);
Token lex_number_literal(std::string::const_iterator& str_iter, const std::string::const_iterator& str_itr_end);
void check_for_keyword(Token& token);


/**
 * Lexes and returns a single token given a pair of string iterators.
 *
 * Always leaves the iterator on the last unconsumed character.
 */
Token lex_token(std::string::const_iterator& str_iter, const std::string::const_iterator& str_itr_end)
{
	Token token;

	std::string cur_c = next_utf8(str_iter, str_itr_end);

	// If it's a comment
	if (is_comment_char(cur_c)) {
		str_iter -= cur_c.length();
		token = lex_comment(str_iter, str_itr_end);
		cur_c = next_utf8(str_iter, str_itr_end);
	}

	// If it's a string literal
	else if (cur_c == "\"" || cur_c == "'") {
		str_iter -= cur_c.length();
		token = lex_string_literal(str_iter, str_itr_end);
		cur_c = next_utf8(str_iter, str_itr_end);
	}

	// If it's a number literal
	else if (is_digit_char(cur_c)) {
		str_iter -= cur_c.length();
		token = lex_number_literal(str_iter, str_itr_end);
		cur_c = next_utf8(str_iter, str_itr_end);
	}

	// If it's an identifier
	else if (is_ident_char(cur_c) && !is_digit_char(cur_c)) {
		do {
			token.str.append(cur_c);
			cur_c = next_utf8(str_iter, str_itr_end);
		} while (is_ident_char(cur_c));

		token.type = IDENTIFIER;

		// Check if the identifier is actually a
		// keyword, and if so update accordingly
		check_for_keyword(token);
	}

	// If it's an operator
	else if (is_op_char(cur_c)) {
		do {
			token.str.append(cur_c);
			cur_c = next_utf8(str_iter, str_itr_end);
		} while (is_op_char(cur_c));

		token.type = OPERATOR;
	}

	// If it's a reserved character
	else if (is_reserved_char(cur_c)) {
		token.str = cur_c;
		cur_c = next_utf8(str_iter, str_itr_end);

		switch (token.str[0]) {
			case '(':
				token.type = LPAREN;
				break;
			case ')':
				token.type = RPAREN;
				break;
			case '[':
				token.type = LSQUARE;
				break;
			case ']':
				token.type = RSQUARE;
				break;
			case '{':
				token.type = LCURLY;
				break;
			case '}':
				token.type = RCURLY;
				break;
			case ',':
				token.type = COMMA;
				break;
			case '.':
				token.type = PERIOD;
				break;
			case ':':
				token.type = COLON;
				break;
			case '`':
				token.type = BACKTICK;
				break;
			default:
				token.type = RESERVED;
				break;
		}
	}

	// If it's a newline
	else if (is_nl_char(cur_c)) {
		token.str = "";
		cur_c = next_utf8(str_iter, str_itr_end);

		token.type = NEWLINE;
	}

	// If it's anything else
	else if (cur_c != "") {
		token.str = cur_c;
		cur_c = next_utf8(str_iter, str_itr_end);

		token.type = UNKNOWN;
	}

	str_iter -= cur_c.length(); // Backup to last non-consumed character
	return token;
}


Token lex_string_literal(std::string::const_iterator& str_iter, const std::string::const_iterator& str_itr_end)
{
	Token token;
	std::string cur_c = next_utf8(str_iter, str_itr_end);

	// Basic string literal
	if (cur_c == "\"") {
		cur_c = next_utf8(str_iter, str_itr_end);
		while (cur_c != "\"" && cur_c != "") {
			// Escape sequence

			if (cur_c == "\\") {
				cur_c = next_utf8(str_iter, str_itr_end);
				if (cur_c != "") {
					switch (cur_c[0]) {
						case 'a':
							token.str.append("\a");
							break;
						case 'b':
							token.str.append("\b");
							break;
						case 'f':
							token.str.append("\f");
							break;
						case 'n':
							token.str.append("\n");
							break;
						case 'r':
							token.str.append("\r");
							break;
						case 't':
							token.str.append("\t");
							break;
						case 'v':
							token.str.append("\v");
							break;
						case 'U':
							// TODO: handle unicode escape sequences, e.g. \U+0D34 and U+F03212
							token.str.append("U");
							break;
						default:
							token.str.append(cur_c);
							break;
					}
				}
			}
			// Normal code point
			else {
				token.str.append(cur_c);
			}
			cur_c = next_utf8(str_iter, str_itr_end);
		}

		if (cur_c == "\"")
			cur_c = next_utf8(str_iter, str_itr_end); // Consume last "

		token.type = STRING_LIT;
	}
	// Raw string literal
	else if (cur_c == "'") {
		// Get opening ' count
		int q_count = 0;
		do {
			++q_count;
			cur_c = next_utf8(str_iter, str_itr_end);
		} while (cur_c == "'");

		// If it doesn't end in " it's malformed
		if (cur_c != "\"") {
			for (int i = 0; i < q_count; ++i)
				token.str.append("'");
			token.type = UNKNOWN;
		} else {
			cur_c = next_utf8(str_iter, str_itr_end);
			while (cur_c != "") {
				// Check for closing pattern
				if (cur_c == "\"") {
					int cq_count = 0;
					cur_c = next_utf8(str_iter, str_itr_end);
					while (cur_c == "'" && cq_count < q_count) {
						++cq_count;
						cur_c = next_utf8(str_iter, str_itr_end);
					}

					if (cq_count == q_count) {
						break;
					} else {
						token.str.append("\"");
						for (int i = 0; i < cq_count; ++i)
							token.str.append("'");
					}
					str_iter -= cur_c.length(); // Back up to last non-quote character
				}
				// Otherwise just consume normally
				else {
					token.str.append(cur_c);
				}
				cur_c = next_utf8(str_iter, str_itr_end);
			}
			token.type = STRING_LIT;
		}
	}

	str_iter -= cur_c.length();

	return token;
}


Token lex_comment(std::string::const_iterator& str_iter, const std::string::const_iterator& str_itr_end)
{
	Token token;
	std::string cur_c = next_utf8(str_iter, str_itr_end);

	if (cur_c == "#") {
		cur_c = next_utf8(str_iter, str_itr_end);

		while (!is_nl_char(cur_c) && cur_c != "") {
			token.str.append(cur_c);
			cur_c = next_utf8(str_iter, str_itr_end);
		}

		token.type = COMMENT;
	}

	str_iter -= cur_c.length();
	return token;
}


Token lex_number_literal(std::string::const_iterator& str_iter, const std::string::const_iterator& str_itr_end)
{
	Token token;
	std::string cur_c = next_utf8(str_iter, str_itr_end);

	if (is_digit_char(cur_c)) {
		int dot_count = 0;

		do {
			token.str.append(cur_c);
			cur_c = next_utf8(str_iter, str_itr_end);
			if (cur_c == ".") {
				++dot_count;
				token.str.append(cur_c);
				cur_c = next_utf8(str_iter, str_itr_end);
			}
		} while (is_digit_char(cur_c));

		if (dot_count == 0) {
			token.type = INTEGER_LIT;
		} else if (dot_count == 1) {
			token.type = FLOAT_LIT;
		}
	}

	str_iter -= cur_c.length();
	return token;
}


void check_for_keyword(Token& token)
{
	if (token.str == "fn") {
		token.type = K_FN;
	} else if (token.str == "struct") {
		token.type = K_STRUCT;
	} else if (token.str == "let") {
		token.type = K_LET;
	} else if (token.str == "return") {
		token.type = K_RETURN;
	}
}

////////////////////////////////////////////////////////////////

inline void up_to_next_nonwhitespace(std::string::const_iterator& str_iter, const std::string::const_iterator& str_itr_end)
{
	std::string cur_c;

	// Get next non-whitespace character
	do  {
		cur_c = next_utf8(str_iter, str_itr_end);
	} while (is_ws_char(cur_c));

	// Move back to last non-whitespace character
	str_iter -= cur_c.length();
}


std::vector<Token> lex_string(const std::string& input)
{
	std::vector<Token> tokens;

	auto str_iter = input.cbegin();
	auto str_itr_end = input.cend();

	// Loop over the string, parsing a single token each loop.
	do {
		// Get next token
		tokens.emplace_back(lex_token(str_iter, str_itr_end));

		// Move to next non-whitespace character
		up_to_next_nonwhitespace(str_iter, str_itr_end);
	} while (str_iter != str_itr_end);

	return tokens;
}

