#include "parser.hpp"


// Declaration
DeclNode* Parser::parse_declaration()
{
	switch (token_iter->type) {
		case K_CONST:
			return parse_constant_decl();

		case K_VAL:
		case K_VAR: {
			return parse_variable_decl();
		}

		case K_FN: {
			return parse_func_definition();
		}

		case K_TYPE: {
			return parse_nominal_type_decl();
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
	node->code = *token_iter;

	++token_iter;
	skip_newlines();

	// Get name
	if (token_iter->type == IDENTIFIER) {
		node->name = token_iter->text;
	}
	else {
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
		node->type = parse_type();
	}
	else {
		// Unknown type
		node->type = ast.store.alloc<Void_T>();
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
		fn_scope.push_symbol(node->name, node);
	}

	// Get initializer
	node->initializer = parse_expression();

	// If it's a function literal, also set the type
	// TODO: this is copy & paste
	if (dynamic_cast<FuncLiteralNode*>(node->initializer)) {
		auto init = dynamic_cast<FuncLiteralNode*>(node->initializer);
		auto init_t = ast.store.alloc<Function_T>();
		std::vector<Type*> ts;
		for (auto& p: init->parameters) {
			ts.push_back(p->type);
		}
		init_t->parameter_ts = ast.store.alloc_from_iters(ts.begin(), ts.end());
		init_t->return_t = init->return_type;
		node->type = init_t;
	}

	if (!token_is_terminator(*token_iter)) {
		// Error
		std::ostringstream msg;
		msg << "Invalid continuation of initializer. ('" << token_iter->text << "')";
		parsing_error(*token_iter, msg.str());
	}

	node->code.text.set_end((token_iter - 1)->text.end());
	return node;
}


// Variable
VariableDeclNode* Parser::parse_variable_decl()
{
	auto node = ast.store.alloc<VariableDeclNode>();
	node->code = *token_iter;

	if (token_iter->type == K_VAL)
		node->mut = false;
	else if (token_iter->type == K_VAR)
		node->mut = true;

	// Variable name
	++token_iter;
	skip_newlines();
	if (token_iter->type == IDENTIFIER) {
		node->name = token_iter->text;
	}
	else {
		// Error
		std::ostringstream msg;
		msg << "Invalid variable name: '" << token_iter->text << "'.";
		parsing_error(*token_iter, msg.str());
	}

	++token_iter;

	// Optional ":"
	if (token_iter->type == COLON) {
		++token_iter;
		skip_newlines();
		node->type = parse_type();
	}
	else {
		// Unknown type
		node->type = ast.store.alloc<Void_T>();
	}

	// Optional "="
	if (token_iter->type == OPERATOR && token_iter->text == "=") {
		++token_iter;
		node->initializer = parse_expression();

		// If it's a function literal, also set the type
		// TODO: this is copy & paste
		if (dynamic_cast<FuncLiteralNode*>(node->initializer)) {
			auto init = dynamic_cast<FuncLiteralNode*>(node->initializer);
			auto init_t = ast.store.alloc<Function_T>();
			std::vector<Type*> ts;
			for (auto& p: init->parameters) {
				ts.push_back(p->type);
			}
			init_t->parameter_ts = ast.store.alloc_from_iters(ts.begin(), ts.end());
			init_t->return_t = init->return_type;
			node->type = init_t;
		}
	}
	else {
		// No initializer
		// TODO
		node->initializer = ast.store.alloc<EmptyExprNode>();
	}

	if (!token_is_terminator(*token_iter)) {
		// Error
		std::ostringstream msg;
		msg << "Invalid continuation of expression: '" << token_iter->text << "'.";
		parsing_error(*token_iter, msg.str());
	}

	node->code.text.set_end((token_iter - 1)->text.end());
	return node;
}


// Function definition
ConstantDeclNode* Parser::parse_func_definition()
{
	// A function is really just a constant with a function
	// literal assigned to it.
	auto node = ast.store.alloc<ConstantDeclNode>();
	node->code = *token_iter;

	// Function name
	++token_iter;
	skip_newlines();
	if (token_iter->type == IDENTIFIER || token_iter->type == OPERATOR) {
		node->name = token_iter->text;
	}
	else {
		// Error
		std::ostringstream msg;
		msg << "Invalid function name: '" << token_iter->text << "'.";
		parsing_error(*token_iter, msg.str());
	}

	// Push name onto scope stack
	fn_scope.push_symbol(node->name, node);

	// Function definition
	++token_iter;
	skip_newlines();
	node->initializer = parse_function_literal(false);

	// Set the type
	// TODO: this is copy & paste
	auto init = dynamic_cast<FuncLiteralNode*>(node->initializer);
	auto init_t = ast.store.alloc<Function_T>();
	std::vector<Type*> ts;
	for (auto& p: init->parameters) {
		ts.push_back(p->type);
	}
	init_t->parameter_ts = ast.store.alloc_from_iters(ts.begin(), ts.end());
	init_t->return_t = init->return_type;
	node->type = init_t;

	node->code.text.set_end((token_iter - 1)->text.end());
	return node;
}


NominalTypeDeclNode* Parser::parse_nominal_type_decl()
{
	auto node = ast.store.alloc<NominalTypeDeclNode>();
	node->code = *token_iter;

	// Skip "type"
	++token_iter;
	skip_newlines();

	// Type name
	if (token_iter->type == IDENTIFIER) {
		node->name = token_iter->text;
	}
	else {
		// Error
		std::ostringstream msg;
		msg << "Invalid type name: '" << token_iter->text << "'.";
		parsing_error(*token_iter, msg.str());
	}

	// Iterate past ":"
	++token_iter;
	skip_newlines();
	if (token_iter->type != COLON) {
		// Error
		std::ostringstream msg;
		msg << "Unexpected token: '" << token_iter->text << "'.";
		parsing_error(*token_iter, msg.str());
	}

	++token_iter;
	skip_newlines();
	node->type = parse_type();
	node->type->name = node->name;


	node->code.text.set_end((token_iter - 1)->text.end());
	return node;
}