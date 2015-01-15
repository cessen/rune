#include "parser.hpp"


// Expression
// Parses the largest number of tokens that result in a single valid
// expression.
ExprNode* Parser::parse_expression()
{
	ExprNode* lhs;

	// LHS
	lhs = parse_primary_expression();

	// RHS
	// Now that we have an lhs, let's see if there are any binary ops
	// following it.
	if (token_is_terminator(*token_iter)) {
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
			break;

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
			assert_in_scope(*token_iter);

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
			// Token is variable
			else if (auto var_decl_node = dynamic_cast<VariableDeclNode*>(scope_stack[token_iter->text])) {
				ExprNode* var = ast.store.alloc<VariableNode>(VariableNode(var_decl_node));
				++token_iter;
				return var;
			}
			// Token is a constant
			else if (auto const_decl_node = dynamic_cast<ConstantDeclNode*>(scope_stack[token_iter->text])) {
				ExprNode* var = ast.store.alloc<ConstantNode>(ConstantNode(const_decl_node));
				++token_iter;
				return var;
			}


			std::ostringstream msg;
			msg << "Unknown identifier: '" << token_iter->text << "'";
			parsing_error(*token_iter, msg.str());
			break;
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
