#include "lexer.hpp"
#include "lexer_utils.hpp"
#include "tokens.hpp"

#include <iostream>
#include <string>
#include <vector>


void check_for_keyword(Token& token);

class Lexer
{
	std::string::const_iterator str_iter;
	std::string::const_iterator str_iter_end;
	unsigned int line_number = 0;
	unsigned int column_number = 0;
	std::string cur_c = "";
	Token token;


public:
	Lexer(const std::string::const_iterator& str_iter, const std::string::const_iterator& str_iter_end): str_iter {str_iter}, str_iter_end {str_iter_end} {
		cur_utf8(&cur_c, str_iter, str_iter_end);
	}


	/**
	 * Lexes and returns a single token.
	 *
	 * Always leaves the iterator on the last unconsumed character.
	 */
	Token lex_token() {
		// Get past any whitespace
		while (is_ws_char(cur_c)) {
			next_char();
		}

		// Initialize for new token
		init_token();

		// If it's a comment
		if (is_comment_char(cur_c)) {
			lex_comment();
		}

		// If it's a string literal
		else if (cur_c == "\"" || cur_c == "'") {
			lex_string_literal();
		}

		// If it's a number literal
		else if (is_digit_char(cur_c)) {
			lex_number_literal();
		}

		// If it's an identifier
		else if (is_ident_char(cur_c) && !is_digit_char(cur_c)) {
			do {
				next_char();
			} while (is_ident_char(cur_c));
			token.text.end = str_iter;

			token.type = IDENTIFIER;

			// Check if the identifier is actually a
			// keyword, and if so update accordingly
			check_for_keyword(token);
		}

		// If it's an operator
		else if (is_op_char(cur_c)) {
			do {
				next_char();
			} while (is_op_char(cur_c));
			token.text.end = str_iter;

			token.type = OPERATOR;
		}

		// If it's a reserved character
		else if (is_reserved_char(cur_c)) {
			switch (cur_c[0]) {
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
			next_char();
			token.text.end = str_iter;
		}

		// If it's a newline
		else if (is_nl_char(cur_c)) {
			// Consume all subsequent whitespace and newlines,
			// so that multiple newlines in a row end up as a single
			// newline token.
			while (is_ws_char(cur_c) || is_nl_char(cur_c)) {
				next_char();
			}

			token.type = NEWLINE;
		}

		// If it's anything else
		else if (cur_c != "") {
			next_char();
			token.text.end = str_iter;

			token.type = UNKNOWN;
		}

		// EOF
		else {
			token.type = LEX_EOF;
		}

		return token;
	}


private:
	void next_char() {
		if (cur_c == "\n") {
			++line_number;
			column_number = 0;
		} else {
			column_number += cur_c.size();
		}

		str_iter += cur_c.size();
		cur_utf8(&cur_c, str_iter, str_iter_end);
	}

	void init_token() {
		token.type = UNKNOWN;
		token.line = line_number;
		token.column = column_number;
		token.text.iter = str_iter;
		token.text.end = str_iter;
	}

	void lex_string_literal() {
		// Basic string literal
		if (cur_c == "\"") {
			next_char();
			init_token(); // Start the token after the opening quote

			while (cur_c != "\"" && cur_c != "") {
				// Escape sequence, advance one more character
				if (cur_c == "\\") {
					next_char();
				}

				next_char();
			}

			token.text.end = str_iter; // End the token before the closing quote

			if (cur_c == "\"")
				next_char(); // Consume last "

			token.type = STRING_LIT;
		}
		// Raw string literal
		else if (cur_c == "'") {
			// Get opening ' count
			int q_count = 0;
			do {
				++q_count;
				next_char();
			} while (cur_c == "'");

			// If it doesn't end in " it's malformed
			if (cur_c != "\"") {
				token.text.end = str_iter;
				token.type = UNKNOWN;
			} else {
				next_char();
				init_token(); // Start the token after the opening sequence

				while (cur_c != "") {
					// Check for closing pattern
					if (cur_c == "\"") {
						int cq_count = 0;
						next_char();
						while (cur_c == "'" && cq_count < q_count) {
							++cq_count;
							next_char();
						}

						if (cq_count == q_count)
							break;
					}
					// Otherwise just consume normally
					else {
						token.text.end = str_iter;
					}
					next_char();
				}

				++token.text.end; // End the token just before the closing sequence
				token.type = RAW_STRING_LIT;
			}
		}
	}


	void lex_comment() {
		bool is_doc = false;
		if (cur_c == "#") {
			next_char();

			// Check if it's a doc-comment
			if (cur_c == ":") {
				is_doc = true;
				next_char();
			}

			init_token(); // Start token just after the "#" or "#:"

			while (!is_nl_char(cur_c) && cur_c != "") {
				next_char();
			}

			token.text.end = str_iter;
			if (is_doc) {
				token.type = DOC_COMMENT;
			} else {
				token.type = COMMENT;
			}
		}
	}


	void lex_number_literal() {
		if (is_digit_char(cur_c)) {
			int dot_count = 0;
			do {
				next_char();
				if (cur_c == ".") {
					++dot_count;
					next_char();
				}
			} while (is_digit_char(cur_c));

			if (dot_count == 0) {
				token.type = INTEGER_LIT;
			} else if (dot_count == 1) {
				token.type = FLOAT_LIT;
			}
		}

		token.text.end = str_iter;
	}


};


void check_for_keyword(Token& token)
{
	if (token.text == "fn") {
		token.type = K_FUNC;
	} else if (token.text == "struct") {
		token.type = K_STRUCT;
	} else if (token.text == "let") {
		token.type = K_LET;
	} else if (token.text == "return") {
		token.type = K_RETURN;
	}
}


std::vector<Token> lex_string(const std::string& input)
{
	std::vector<Token> tokens;
	Lexer lexer = Lexer(input.cbegin(), input.cend());

	// Lex away!
	while (true) {
		// Get next token
		tokens.emplace_back(lexer.lex_token());

		if (tokens.back().type == LEX_EOF)
			break;
	}

	tokens.pop_back(); // Pop EOL token

	return tokens;
}