#include "lexer.hpp"
#include "lexer_utils.hpp"

#include <iostream>
#include <string>
#include <vector>



/**
 * Lexes and returns a single token given a pair of string iterators.
 *
 * Always leaves the iterator on the last unconsumed character.
 */
Token lex_token(std::string::const_iterator& str_iter, const std::string::const_iterator& str_itr_end)
{
	Token token {UNKNOWN, ""};

	std::string cur_c = next_utf8(str_iter, str_itr_end);

	// If it's a comment
	if (is_comment_char(cur_c)) {
		cur_c = next_utf8(str_iter, str_itr_end);

		// Single-line comment (easy case)
		if (cur_c != "=") {
			while (!is_nl_char(cur_c) && cur_c != "") {
				token.str.append(cur_c);
				cur_c = next_utf8(str_iter, str_itr_end);
			}
		}
		// Multi-line comment (tricky case--must nest properly)
		else {
			int open = 1;

			do {
				cur_c = next_utf8(str_iter, str_itr_end);
				// Open comment
				if (cur_c == "#" && cur_utf8(str_iter, str_itr_end) == "=") {
					++open;
					token.str.append(cur_c);
					cur_c = next_utf8(str_iter, str_itr_end);
					token.str.append(cur_c);
				}
				// Close comment
				else if (cur_c == "=" && cur_utf8(str_iter, str_itr_end) == "#") {
					--open;
					if (open == 0) {
						// If it's the last close, don't include it in the comment string
						cur_c = next_utf8(str_iter, str_itr_end);
					} else {
						token.str.append(cur_c);
						cur_c = next_utf8(str_iter, str_itr_end);
						token.str.append(cur_c);
					}
				} else {
					token.str.append(cur_c);
				}
			} while (open != 0  && cur_c != "");

			if (open == 0)
				cur_c = next_utf8(str_iter, str_itr_end); // Consume the last "#"
		}

		token.type = COMMENT;
	}

	// If it's an identifier
	else if (is_ident_char(cur_c) && !is_digit_char(cur_c)) {
		do {
			token.str.append(cur_c);
			cur_c = next_utf8(str_iter, str_itr_end);
		} while (is_ident_char(cur_c));

		token.type = IDENTIFIER;
	}

	// If it's a number literal
	else if (is_digit_char(cur_c)) {
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
		} else {
			token.type = UNKNOWN;
		}
	}

	// If it's a string literal
	else if (cur_c == "\"" || cur_c == "'") {
		// Basic string literal
		if (cur_c == "\"") {
			// TODO: handle escape sequences
			cur_c = next_utf8(str_iter, str_itr_end);
			while (cur_c != "\"" && cur_c != "") {
				token.str.append(cur_c);
				cur_c = next_utf8(str_iter, str_itr_end);
			}

			if (cur_c == "\"")
				cur_c = next_utf8(str_iter, str_itr_end); // Consume last "

			token.type = STRING_LIT;
		}
		// Raw string literal
		else {
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




	}

	// If it's an operator
	else if (is_op_char(cur_c)) {
		do {
			token.str.append(cur_c);
			cur_c = next_utf8(str_iter, str_itr_end);
		} while (is_op_char(cur_c));

		token.type = OPERATOR;
	}

	// If it's a newline
	else if (is_reserved_char(cur_c)) {
		token.str = cur_c;
		cur_c = next_utf8(str_iter, str_itr_end);

		token.type = RESERVED;
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

