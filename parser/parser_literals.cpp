#include "parser.hpp"

#include <vector>
#include <unordered_set>


LiteralNode* Parser::parse_literal()
{
	switch (token_iter->type) {
		case INTEGER_LIT: {
			auto node = ast.store.alloc<IntegerLiteralNode>();
			node->text = token_iter->text;
			node->code = *token_iter;
			++token_iter;
			return node;
		}
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
	node->code = *token_iter;
	std::vector<VariableDeclNode*> parameters;

	if (has_fn) {
		if (token_iter->type == K_FN) {
			++token_iter;
			skip_newlines();
		}
		else {
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
		++token_iter;
		skip_newlines();
		auto param_node = ast.store.alloc(VariableDeclNode(name, parse_type(), ast.store.alloc<EmptyExprNode>(), false));
		parameters.push_back(param_node);

		// Push parameter onto scope
		if (!scope_stack.push_symbol(name, param_node)) {
			// Error
			std::ostringstream msg;
			msg << "Function definition has a parameter name '" << name << "', but something with that name is already in scope.";
			parsing_error(*token_iter, msg.str());
		}

		// Either a comma or closing square bracket
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
		++token_iter;
		skip_newlines();
		node->return_type = parse_type();

	}
	else {
		// Empty return type
		node->return_type = ast.store.alloc<Void_T>();
	}

	// Function body
	skip_newlines();
	if (token_iter->type == LPAREN) {
		node->body = parse_scope();
	}
	else {
		// Error
		std::ostringstream msg;
		msg << "Function definition has no body.";
		parsing_error(*token_iter, msg.str());
	}

	scope_stack.pop_scope(); // End parameters scope

	node->code.text.set_end((token_iter - 1)->text.end());
	return node;
}


Type* Parser::parse_type()
{
	switch (token_iter->type) {
		case K_STRUCT: {
			return parse_struct();
		}

		case AT: {
			auto node = ast.store.alloc<Pointer_T>();
			++token_iter;
			node->type = parse_type();
			return node;
		}

		case IDENTIFIER: {
			// Signed integers
			if (token_iter->text == "i8") {
				++token_iter;
				return ast.store.alloc<Int8_T>();
			}
			if (token_iter->text == "i16") {
				++token_iter;
				return ast.store.alloc<Int16_T>();
			}
			if (token_iter->text == "i32") {
				++token_iter;
				return ast.store.alloc<Int32_T>();
			}
			if (token_iter->text == "i64") {
				++token_iter;
				return ast.store.alloc<Int64_T>();
			}

			// Unsigned integers
			if (token_iter->text == "u8") {
				++token_iter;
				return ast.store.alloc<UInt8_T>();
			}
			if (token_iter->text == "u16") {
				++token_iter;
				return ast.store.alloc<UInt16_T>();
			}
			if (token_iter->text == "u32") {
				++token_iter;
				return ast.store.alloc<UInt32_T>();
			}
			if (token_iter->text == "u64") {
				++token_iter;
				return ast.store.alloc<UInt64_T>();
			}

			// Floats
			if (token_iter->text == "f16") {
				++token_iter;
				return ast.store.alloc<Float16_T>();
			}
			if (token_iter->text == "f32") {
				++token_iter;
				return ast.store.alloc<Float32_T>();
			}
			if (token_iter->text == "f64") {
				++token_iter;
				return ast.store.alloc<Float64_T>();
			}

			// User defined type
			if (scope_stack.is_symbol_in_scope(token_iter->text)) {
				DeclNode* decl_node = scope_stack[token_iter->text];
				++token_iter;
				return decl_node->type;
			}

			break;
		}
		
		default: {
		    break;
		}
	}

	// Error, unknown type
	std::ostringstream msg;
	msg << "Invalid type name: '" << token_iter->text << "'.";
	parsing_error(*token_iter, msg.str());

	// Bogus return, will never be reached because parsing_error() throws.
	// It's here just to silence warnings.
	return nullptr;
}


Type* Parser::parse_struct()
{
	auto type = ast.store.alloc<Struct_T>();

	// Skip "struct"
	++token_iter;
	skip_newlines();

	// Iterate past "{"
	if (token_iter->type != LCURLY) {
		// Error
		std::ostringstream msg;
		msg << "Unexpected token: '" << token_iter->text << "'.";
		parsing_error(*token_iter, msg.str());
	}

	std::vector<StringSlice> names;
	std::vector<Type*> types;

	while (true) {
		// Get name
		++token_iter;
		skip_newlines();
		if (token_iter->type == IDENTIFIER) {
			names.push_back(token_iter->text);
		}
		else {
			break;
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

		// Type
		++token_iter;
		skip_newlines();
		types.push_back(parse_type());

		// Possible comma
		skip_newlines();
		if (token_iter->type != COMMA) {
			break;
		}
	}

	// Iterate past "}"
	if (token_iter->type != RCURLY) {
		// Error
		std::ostringstream msg;
		msg << "Unexpected token: '" << token_iter->text << "'.";
		parsing_error(*token_iter, msg.str());
	}
	++token_iter;
	skip_newlines();

	// Make sure there are no duplicate field names
	std::unordered_set<StringSlice> name_dup_check_set_thing;
	for (auto& name : names) {
		auto b = name_dup_check_set_thing.insert(name);
		if (!b.second) {
			// Error
			std::ostringstream msg;
			msg << "Duplicate field name found: '" << name << "'.";
			parsing_error(*token_iter, msg.str());
		}
	}

	// Put the names and types in the struct
	type->field_names = ast.store.alloc_from_iters(names.begin(), names.end());
	type->field_types = ast.store.alloc_from_iters(types.begin(), types.end());

	return type;
}
