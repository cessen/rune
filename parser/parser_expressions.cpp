#include "parser.hpp"


// Expression
// Parses the largest number of tokens that result in a single valid
// expression.
ExprNode* Parser::parse_expression()
{
	ExprNode* lhs;
	CodeSlice code_slice;
	code_slice = *token_iter;

	// LHS
	lhs = parse_primary_expression();

	// RHS
	// Now that we have an lhs, let's see if there are any binary ops
	// following it.
	if (token_is_terminator(*token_iter)) {
		code_slice.text.set_end((token_iter - 1)->text.end());
		lhs->code = code_slice;
		return lhs;
	}
	else if (token_is_const_function(*token_iter)) {
		// Parse binary operator
		lhs = parse_binary_func_call(std::move(lhs), -1000000);
	}
	else {
		// Error
		std::ostringstream msg;
		msg << "Expected a binary operator, but instead found '" << token_iter->text << "'.";
		parsing_error(*token_iter, msg.str());
	}

	code_slice.text.set_end((token_iter - 1)->text.end());
	lhs->code = code_slice;
	return lhs;
}


// Primary expression
// Parses the fewest number of tokens that result in a single
// valid expression (while keeping surrounding code valid).
ExprNode* Parser::parse_primary_expression()
{
	switch (token_iter->type) {
		case LPAREN:
			return parse_scope();

		// Dereference
		case DOLLAR: {
			auto node = ast.store.alloc<DerefNode>();
			node->code = *token_iter;
			++token_iter;
			node->expr = parse_expression();
			// TODO: handle node->code properly
			return node;
		}

		// Address of
		case AT: {
			auto node = ast.store.alloc<AddressOfNode>();
			node->code = *token_iter;
			++token_iter;
			node->expr = parse_expression();
			// TODO: handle node->code properly
			return node;
		}

		// Literal
		case K_FN:
		case INTEGER_LIT:
		case FLOAT_LIT:
		case STRING_LIT:
		case RAW_STRING_LIT: {
			return parse_literal();
		}

		case OPERATOR:
		case IDENTIFIER: {
			// Standard function call
			if (token_iter[1].type == LSQUARE) {
				return parse_standard_func_call();
			}
			// Token is const function
			else if (token_is_const_function(*token_iter)) {
				if (!token_is_terminator(token_iter[1])) {
					return parse_unary_func_call();
				}
				else {
					// TODO
					std::ostringstream msg;
					msg << "TODO: can't parse const functions as values yet. ('" << token_iter->text << "')";
					parsing_error(*token_iter, msg.str());
				}
			}
			// Token is some other identifier
			else {
				ExprNode* var = ast.store.alloc<UnknownIdentifierNode>();
				var->code = *token_iter;
				++token_iter;
				return var;
			}
		}

		default:
			break;
	}

	// Error
	std::ostringstream msg;
	msg << "ICE parse_primary_expression(). ('" << token_iter->text << "')";
	parsing_error(*token_iter, msg.str());
	throw 0; // Silence warnings about not returning, parsing_error throws anyway
}
