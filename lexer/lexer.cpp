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
	TokenType last_token_type = UNKNOWN;
	Token token;

	std::vector<bool> generic_stack = {false};


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
start_over:

		// Get past any whitespace
		while (is_ws_char(cur_c)) {
			next_char();
		}

		// Initialize for new token
		init_token();

		// If it's a comment
		if (is_comment_char(cur_c)) {
			if (!lex_comment())
				goto start_over;
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
			}
			while (is_ident_char(cur_c));

			token.text.set_end(str_iter);
			token.type = IDENTIFIER;

			// Check if the identifier is actually a
			// keyword, and if so update accordingly
			check_for_keyword(token);
		}

		else if (in_generic() && cur_c[0] == '>') {
			pop_generic(true);
			next_char();

			token.text.set_end(str_iter);
			token.type = RGENERIC;
		}

		// If it's an operator
		else if (is_op_char(cur_c)) {
			do {
				next_char();
			}
			while (is_op_char(cur_c));

			token.text.set_end(str_iter);
			token.type = OPERATOR;
		}

		// If it's a reserved character
		else if (is_reserved_char(cur_c)) {
			switch (cur_c[0]) {
				case '(':
					token.type = LPAREN;
					push_generic(false);
					next_char();
					break;
				case ')':
					token.type = RPAREN;
					pop_generic(false);
					next_char();
					break;
				case '[':
					token.type = LSQUARE;
					push_generic(false);
					next_char();
					break;
				case ']':
					token.type = RSQUARE;
					pop_generic(false);
					next_char();
					break;
				case '{':
					token.type = LCURLY;
					push_generic(false);
					next_char();
					break;
				case '}':
					token.type = RCURLY;
					pop_generic(false);
					next_char();
					break;
				case '@':
					token.type = AT;
					next_char();
					break;
				case ',':
					token.type = COMMA;
					next_char();
					break;
				case '.':
					token.type = PERIOD;
					next_char();
					break;
				case ':':
					token.type = COLON;
					next_char();
					break;
				case '$':
					token.type = DOLLAR;
					next_char();
					break;
				case '`': {
					next_char();
					if (cur_c[0] == '<') {
						token.type = LGENERIC;
						push_generic(true);
						next_char();
					}
					else {
						token.type = BACKTICK;
					}
					break;
				}
				default:
					token.type = RESERVED;
					next_char();
					break;
			}

			token.text.set_end(str_iter);
		}

		// If it's a newline
		else if (is_nl_char(cur_c)) {
			// Consume all subsequent whitespace and newlines,
			// so that multiple newlines in a row end up as a single
			// newline token.  Also escape newlines with a trailing backslash
			while (is_nl_char(cur_c)) {
				next_char();
				while (is_ws_char(cur_c))
					next_char();

				if (cur_c == "\\") {
					next_char();
					goto start_over;
				}
			}

			if (last_token_type == NEWLINE)
				goto start_over;

			token.type = NEWLINE;
		}

		// If it's anything else
		else if (cur_c != "") {
			next_char();

			token.text.set_end(str_iter);
			token.type = UNKNOWN;
		}

		// EOF
		else {
			token.type = LEX_EOF;
		}

		last_token_type = token.type;
		return token;
	}


private:
	void next_char() {
		if (cur_c == "\n") {
			++line_number;
			column_number = 0;
		}
		else {
			column_number += cur_c.size();
		}

		str_iter += cur_c.size();
		cur_utf8(&cur_c, str_iter, str_iter_end);
	}

	void init_token() {
		token.type = UNKNOWN;
		token.line = line_number;
		token.column = column_number;
		token.text.set_begin(str_iter);
		token.text.set_end(str_iter);
	}


	// Some utility functions for lexing generic delimeters properly
	void push_generic(bool state) {
		generic_stack.push_back(state);
	}

	void pop_generic(bool state) {
		if (generic_stack.size() > 0 && generic_stack.back() == state)
			generic_stack.pop_back();
	}

	bool in_generic() {
		if (generic_stack.size() > 0)
			return generic_stack.back();
		else
			return false;
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

			token.text.set_end(str_iter); // End the token before the closing quote

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
			}
			while (cur_c == "'");

			// If it doesn't end in " it's malformed
			if (cur_c != "\"") {
				token.text.set_end(str_iter);
				token.type = UNKNOWN;
			}
			else {
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
						token.text.set_end(str_iter);
					}
					next_char();
				}

				++token.text.iter_end; // End the token just before the closing sequence
				token.type = RAW_STRING_LIT;
			}
		}
	}


	bool lex_comment() {
		// Returns if it was a doc comment or not, so that non-doc comments
		// can be skipped;
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

			token.text.set_end(str_iter);
			if (is_doc) {
				token.type = DOC_STRING;
			}
		}

		return is_doc;
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
			}
			while (is_digit_char(cur_c));

			if (dot_count == 0) {
				token.type = INTEGER_LIT;
			}
			else if (dot_count == 1) {
				token.type = FLOAT_LIT;
			}
		}

		token.text.set_end(str_iter);
	}


};


void check_for_keyword(Token& token)
{
	// Scoping
	if (token.text == "namespace") {
		token.type = K_NAMESPACE;
	}
	else if (token.text == "pub") {
		token.type = K_PUB;
	}
	else if (token.text == "unsafe") {
		token.type = K_UNSAFE;
	}

	// Handle declarations
	else if (token.text == "const") {
		token.type = K_CONST;
	}
	else if (token.text == "val") {
		token.type = K_VAL;
	}
	else if (token.text == "var") {
		token.type = K_VAR;
	}

	// Handle modifiers
	else if (token.text == "mut") {
		token.type = K_MUT;
	}
	else if (token.text == "ref") {
		token.type = K_REF;
	}

	// Functions
	else if (token.text == "fn") {
		token.type = K_FN;
	}

	// Data types
	else if (token.text == "struct") {
		token.type = K_STRUCT;
	}
	else if (token.text == "enum") {
		token.type = K_ENUM;
	}
	else if (token.text == "union") {
		token.type = K_UNION;
	}

	// Traits
	else if (token.text == "trait") {
		token.type = K_TRAIT;
	}
	else if (token.text == "is") {
		token.type = K_IS;
	}

	// Control flow
	else if (token.text == "if") {
		token.type = K_IF;
	}
	else if (token.text == "else") {
		token.type = K_ELSE;
	}
	else if (token.text == "loop") {
		token.type = K_LOOP;
	}
	else if (token.text == "while") {
		token.type = K_WHILE;
	}
	else if (token.text == "until") {
		token.type = K_UNTIL;
	}
	else if (token.text == "for") {
		token.type = K_FOR;
	}
	else if (token.text == "in") {
		token.type = K_IN;
	}
	else if (token.text == "break") {
		token.type = K_BREAK;
	}
	else if (token.text == "continue") {
		token.type = K_CONTINUE;
	}
	else if (token.text == "return") {
		token.type = K_RETURN;
	}

	// Type casting
	else if (token.text == "as") {
		token.type = K_AS;
	}

	// Misc
	else if (token.text == "alias") {
		token.type = K_ALIAS;
	}
	else if (token.text == "type") {
		token.type = K_TYPE;
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