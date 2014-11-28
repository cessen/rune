#include "parser.hpp"


// Standard function call syntax
FuncCallNode* Parser::parse_standard_func_call()
{
	auto node = ast.store.alloc<FuncCallNode>();
	std::vector<ExprNode*> parameters;

	// Get function name
	if (token_iter->type == IDENTIFIER || token_iter->type == OPERATOR) {
		node->name = token_iter->text;
	} else {
		// Error
		std::ostringstream msg;
		msg << "Invalid name for standard function call: '" << token_iter->text << "'.";
		parsing_error(*token_iter, msg.str());
	}

	// [
	++token_iter;
	skip_comments();
	if (token_iter->type != LSQUARE) {
		// Error
		std::ostringstream msg;
		msg << "Function call without '[]'.";
		parsing_error(*token_iter, msg.str());
	}
	++token_iter;

	skip_comments_and_newlines();

	// ]?
	if (token_iter->type == RSQUARE) {
		++token_iter;
		return node;
	}

	while (true) {
		skip_comments_and_newlines();

		parameters.push_back(parse_expression());

		skip_comments_and_newlines();

		// , or ]
		if (token_iter->type == COMMA) {
			++token_iter;
			continue;
		} else if (token_iter->type == RSQUARE) {
			++token_iter;
			break;
		}

		++token_iter;
	}

	node->parameters = ast.store.alloc_from_iters(parameters.begin(), parameters.end());

	return node;
}


// Unary function call syntax
FuncCallNode* Parser::parse_unary_func_call()
{
	auto node = ast.store.alloc<FuncCallNode>();

	// Get function name
	if (token_iter->type == IDENTIFIER || token_iter->type == OPERATOR) {
		node->name = token_iter->text;
	} else {
		// Error
		std::ostringstream msg;
		msg << "Invalid name for unary function call: '" << token_iter->text << "'.";
		parsing_error(*token_iter, msg.str());
	}
	++token_iter;

	// Next primary expression should be the argument
	node->parameters = ast.store.alloc_array<ExprNode*>(1);
	node->parameters[0] = parse_primary_expression();

	return node;
}


// Binary infix function call syntax
ExprNode* Parser::parse_binary_func_call(ExprNode* lhs, int lhs_prec)
{
	auto node = ast.store.alloc<FuncCallNode>();
	ExprNode* rhs;

	if (!token_is_const_function(*token_iter)) {
		// Error
		std::ostringstream msg;
		msg << "Invalid name for binary function call: '" << token_iter->text << "'.";
		parsing_error(*token_iter, msg.str());
	}

	// Op info
	node->name = token_iter->text;
	const int my_prec = get_op_prec(token_iter->text);

	auto pre_rhs = token_iter;

	// Get rhs argument
	++token_iter;
	rhs = parse_primary_expression();

	// Handle precedence
	while (true) {
		if (token_is_terminator(*token_iter)) {
			node->parameters = ast.store.alloc_array<ExprNode*>(2);
			node->parameters[0] = lhs;
			node->parameters[1] = rhs;
			break;
		} else if (lhs_prec >= my_prec) {
			token_iter = pre_rhs;
			return lhs;
		} else if (token_is_const_function(*token_iter)) {
			if (get_op_prec(token_iter->text) > my_prec) {
				rhs = parse_binary_func_call(std::move(rhs), my_prec);
			} else {
				node->parameters = ast.store.alloc_array<ExprNode*>(2);
				node->parameters[0] = lhs;
				node->parameters[1] = rhs;
				return parse_binary_func_call(std::move(node), lhs_prec);
			}
		} else {
			// Error
			std::ostringstream msg;
			msg << "GAHWHATDOESTHISMEAN??? parse_binary_func_call()";
			parsing_error(*token_iter, msg.str());
		}
	}

	return node;
}
