#include "lexer.hpp"
#include "lexer_utils.hpp"

#include <iostream>
#include <string>





std::string Lexer::next()
{
	std::string token = "";

	// Skip whitespace
	while (is_ws_char(cur_c)) {
		cur_c = next_utf8(in);
	}

	// If it's an identifier
	if (is_ident_char(cur_c)) {
		do {
			token.append(cur_c);
			cur_c = next_utf8(in);
		} while (is_ident_char(cur_c));

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

