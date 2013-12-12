#include "lexer.hpp"
#include "lexer_utils.hpp"

#include <iostream>
#include <string>


std::vector<Token> lex_string(const std::string& input)
{
	std::vector<Token> tokens;

	auto str_iter = input.cbegin();
	auto str_itr_end = input.cend();

	std::string cur_c = next_utf8(str_iter, str_itr_end);

	// Keep looping until we reach the end of the string
	// or hit a malformed character.
	// Each loop parses a single token.
	while (cur_c != "") {
		std::string token = "";

		// Skip whitespace
		while (is_ws_char(cur_c)) {
			cur_c = next_utf8(str_iter, str_itr_end);
		}



		// If it's an identifier
		if (is_ident_char(cur_c)) {
			do {
				token.append(cur_c);
				cur_c = next_utf8(str_iter, str_itr_end);
			} while (is_ident_char(cur_c));

			tokens.push_back(Token {IDENTIFIER, token});
			continue;
		}

		// If it's an operator
		if (is_op_char(cur_c)) {
			do {
				token.append(cur_c);
				cur_c = next_utf8(str_iter, str_itr_end);
			} while (is_op_char(cur_c));

			tokens.push_back(Token {OPERATOR, token});
			continue;
		}

		// If it's anything else
		if (cur_c != "") {
			token = cur_c;
			cur_c = next_utf8(str_iter, str_itr_end);

			tokens.push_back(Token {UNKNOWN, token});
			continue;
		}
	}

	return tokens;
}

