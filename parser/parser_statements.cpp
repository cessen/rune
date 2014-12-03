#include "parser.hpp"

// Statement
// Parses a single full statement.  In general this means a declaration
// or an expression, but also includes things like return and break
// statements.
StatementNode* Parser::parse_statement()
{
	switch (token_iter->type) {
			// Return statement
		case K_RETURN: {
			// TODO
			parsing_error(*token_iter, "TODO: Return statements haven't been implemented yet.");
		}

		// Declaration
		case K_CONST:
		case K_LET:
		case K_VAR:
		case K_FN:
		case K_STRUCT: {
			return parse_declaration();
		}

		// Expression
		case INTEGER_LIT:
		case FLOAT_LIT:
		case STRING_LIT:
		case RAW_STRING_LIT:
		case LPAREN:
		case IDENTIFIER:
		case OPERATOR: {
			return parse_expression();
		}

		default: {
			// Error
			std::ostringstream msg;
			msg << "Unknown statement '" << token_iter->text << "'.";
			parsing_error(*token_iter, msg.str());
			throw 0; // Silence warnings about not returning, parsing_error throws anyway
		}
	}
}