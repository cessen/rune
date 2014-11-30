#include "parser.hpp"


LiteralNode* Parser::parse_literal()
{
	switch (token_iter->type) {
		case INTEGER_LIT:
		case FLOAT_LIT:
		case STRING_LIT:
		case RAW_STRING_LIT: {
			// TODO
			std::ostringstream msg;
			msg << "TODO: some literals are not parsed yet. ('" << token_iter->text << "').";
			parsing_error(*token_iter, msg.str());
			break;
		}

		case K_FN:
			return parse_function_literal();

		default:
			break;
	}

	// ERROR
	std::ostringstream msg;
	msg << "ICE parse_literal(). ('" << token_iter->text << "').";
	parsing_error(*token_iter, msg.str());
	throw 0;
}


// Function literal
FuncLiteralNode* Parser::parse_function_literal(bool has_fn)
{
	auto node = ast.store.alloc<FuncLiteralNode>();
	std::vector<NameTypePair> parameters;

	if (has_fn) {
		if (token_iter->type == K_FN) {
			++token_iter;
			skip_newlines();
		} else {
			// Error
			std::ostringstream msg;
			msg << "Function literal must start with 'fn'.";
			parsing_error(*token_iter, msg.str());
		}
	}

	// Open bracket
	if (token_iter->type != LSQUARE) {
		// Error
		std::ostringstream msg;
		msg << "Attempted to define a function without a parameter list.";
		parsing_error(*token_iter, msg.str());
	}

	// Parameters
	scope_stack.push_scope(); // Begin parameters scope (ends after body)
	while (true) {
		// Parameter name
		++token_iter;
		skip_newlines();
		StringSlice name;
		if (token_iter->type == IDENTIFIER)
			name = token_iter->text;
		else if (token_iter->type == RSQUARE)
			break;
		else {
			// Error
			std::ostringstream msg;
			msg << "Something fishy with the end of this function definition's parameter list.";
			parsing_error(*token_iter, msg.str());
		}

		// Colon
		++token_iter;
		skip_newlines();
		if (token_iter->type != COLON) {
			// Error
			std::ostringstream msg;
			msg << "Function parameter lacks a type.";
			parsing_error(*token_iter, msg.str());
		}

		// Parameter type
		// TODO: types aren't just names, need to evaluate full type expression here.
		++token_iter;
		skip_newlines();
		if (token_iter->type == IDENTIFIER) {
			parameters.push_back(NameTypePair {name, ast.store.alloc<TypeExprNode>()});

			// Push onto scope
			if (!scope_stack.push_symbol(name, SymbolType::VARIABLE)) {
				// Error
				std::ostringstream msg;
				msg << "Function definition has a parameter name '" << name << "', but something with that name is already in scope.";
				parsing_error(*token_iter, msg.str());
			}
		} else {
			// Error
			std::ostringstream msg;
			msg << "Invalid type name for function parameter: '" << token_iter->text << "'.";
			parsing_error(*token_iter, msg.str());
		}

		// Either a comma or closing square bracket
		++token_iter;
		skip_newlines();
		if (token_iter->type == COMMA)
			continue;
		else if (token_iter->type == RSQUARE)
			break;
		else {
			// Error
			std::ostringstream msg;
			msg << "Something fishy with the end of this function declaration's parameter list.";
			parsing_error(*token_iter, msg.str());
		}
	}

	node->parameters = ast.store.alloc_from_iters(parameters.begin(), parameters.end());

	// -> (optional return type)
	++token_iter;
	skip_newlines();
	if (token_iter->type == OPERATOR && token_iter->text == "->") {
		// Return type
		// TODO: types aren't just names, need to evaluate full type expression here.
		++token_iter;
		skip_newlines();
		if (token_iter->type == IDENTIFIER)
			node->return_type = ast.store.alloc<TypeExprNode>();
		else {
			// Error
			std::ostringstream msg;
			msg << "Invalid type name for return type: '" << token_iter->text << "'.";
			parsing_error(*token_iter, msg.str());
		}
	} else {
		// TODO: empty return type
		node->return_type = ast.store.alloc<TypeExprNode>();
	}

	// Function body
	++token_iter;
	skip_newlines();
	if (token_iter->type == LPAREN) {
		node->body = parse_scope();
	} else {
		// Error
		std::ostringstream msg;
		msg << "Function definition has no body.";
		parsing_error(*token_iter, msg.str());
	}

	scope_stack.pop_scope(); // End parameters scope

	return node;
}
