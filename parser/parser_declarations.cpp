#include "parser.hpp"


// Declaration
DeclNode* Parser::parse_declaration()
{
	switch (token_iter->type) {
		case K_CONST:
			return parse_constant_decl();

		case K_LET:
		case K_VAR: {
			return parse_variable_decl();
		}

		case K_FN: {
			return parse_func_definition();
		}

		default: {
			// TODO
			std::ostringstream msg;
			msg << "TODO: not all declarations are implemented yet. ('" << token_iter->text << "')";
			parsing_error(*token_iter, msg.str());;
			throw 0; // Silence warnings about not returning, parsing_error throws anyway
		}
	}
}


// Constant
ConstantDeclNode* Parser::parse_constant_decl()
{
	auto node = ast.store.alloc<ConstantDeclNode>();

	++token_iter;
	skip_newlines();

	// Get name
	if (token_iter->type == IDENTIFIER) {
		node->name = token_iter->text;
	} else {
		// Error
		std::ostringstream msg;
		msg << "Invalid constant name: '" << token_iter->text << "'.";
		parsing_error(*token_iter, msg.str());
	}

	++token_iter;

	// Optional ":"
	if (token_iter->type == COLON) {
		++token_iter;
		skip_newlines();

		if (token_iter->type == IDENTIFIER) {
			// TODO
			++token_iter;
			node->type = ast.store.alloc<TypeExprNode>();
		} else {
			// Error
			std::ostringstream msg;
			msg << "Invalid type name: '" << token_iter->text << "'.";
			parsing_error(*token_iter, msg.str());
		}
	} else {
		// Unknown type
		// TODO
		node->type = ast.store.alloc<TypeExprNode>();
	}

	// Initializer is required for constants
	if (token_iter->type != OPERATOR || token_iter->text != "=") {
		// Error
		std::ostringstream msg;
		msg << "Constant '" << node->name << "' has no initializer.";
		parsing_error(*token_iter, msg.str());
	}

	++token_iter;
	skip_newlines();
	if (token_iter->type == K_FN) {
		if (!scope_stack.push_symbol(node->name, SymbolType::CONST_FUNCTION)) {
			// Error
			std::ostringstream msg;
			msg << "Attempted to declare const function '" << node->name << "', but something with the same name is already in scope.";
			parsing_error(*token_iter, msg.str());
		}
	} else {
		if (!scope_stack.push_symbol(node->name, SymbolType::CONST_VARIABLE)) {
			// Error
			std::ostringstream msg;
			msg << "Attempted to declare const variable '" << node->name << "', but something with the same name is already in scope.";
			parsing_error(*token_iter, msg.str());
		}
	}

	// Get initializer
	node->initializer = parse_expression();

	if (!token_is_terminator(*token_iter)) {
		// Error
		std::ostringstream msg;
		msg << "Invalid continuation of initializer. ('" << token_iter->text << "')";
		parsing_error(*token_iter, msg.str());
	}

	return node;
}


// Variable
VariableDeclNode* Parser::parse_variable_decl()
{
	auto node = ast.store.alloc<VariableDeclNode>();

	if (token_iter->type == K_LET)
		node->mut = false;
	else if (token_iter->type == K_VAR)
		node->mut = true;

	// Variable name
	++token_iter;
	skip_newlines();
	if (token_iter->type == IDENTIFIER) {
		node->name = token_iter->text;
	} else {
		// Error
		std::ostringstream msg;
		msg << "Invalid variable name: '" << token_iter->text << "'.";
		parsing_error(*token_iter, msg.str());
	}

	// Push symbol onto scope stack
	if (!scope_stack.push_symbol(node->name, SymbolType::VARIABLE)) {
		// Error
		std::ostringstream msg;
		msg << "Attempted to declare variable '" << token_iter->text << "', but something with the same name is already in scope.";
		parsing_error(*token_iter, msg.str());
	}

	++token_iter;

	// Optional ":"
	if (token_iter->type == COLON) {
		++token_iter;
		skip_newlines();

		if (token_iter->type == IDENTIFIER) {
			// TODO
			++token_iter;
			node->type = ast.store.alloc<TypeExprNode>();
		} else {
			// Error
			std::ostringstream msg;
			msg << "Invalid type name: '" << token_iter->text << "'.";
			parsing_error(*token_iter, msg.str());
		}
	} else {
		// Unknown type
		// TODO
		node->type = ast.store.alloc<TypeExprNode>();
	}

	// Optional "="
	if (token_iter->type == OPERATOR && token_iter->text == "=") {
		++token_iter;
		node->initializer = parse_expression();
	} else {
		// No initializer
		// TODO
		node->initializer = ast.store.alloc<ExprNode>();
	}

	if (!token_is_terminator(*token_iter)) {
		// Error
		std::ostringstream msg;
		msg << "Invalid continuation of expression: '" << token_iter->text << "'.";
		parsing_error(*token_iter, msg.str());
	}

	return node;
}


// Function definition
ConstantDeclNode* Parser::parse_func_definition()
{
	// A function is really just a constant with a function
	// literal assigned to it.
	auto node = ast.store.alloc<ConstantDeclNode>();

	// Function name
	++token_iter;
	skip_newlines();
	if (token_iter->type == IDENTIFIER || token_iter->type == OPERATOR) {
		node->name = token_iter->text;
	} else {
		// Error
		std::ostringstream msg;
		msg << "Invalid function name: '" << token_iter->text << "'.";
		parsing_error(*token_iter, msg.str());
	}

	// Push name onto scope stack
	if (!scope_stack.push_symbol(node->name, SymbolType::CONST_FUNCTION)) {
		// Error
		std::ostringstream msg;
		msg << "Attempted to declare function '" << node->name << "', but something with the same name is already in scope.";
		parsing_error(*token_iter, msg.str());
	}

	// Function definition
	++token_iter;
	skip_newlines();
	node->initializer = parse_function_literal(false);

	// TODO: type
	node->type = ast.store.alloc<TypeExprNode>();

	return node;
}