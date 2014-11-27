#include "parser.hpp"

#include "ast.hpp"
#include "tokens.hpp"
#include "string_slice.hpp"
#include "scope_stack.hpp"

#include <iostream>
#include <exception>
#include <string>
#include <sstream>
#include <unordered_map>


class Parser
{
	std::string file_path;

	std::vector<Token>::const_iterator begin;
	std::vector<Token>::const_iterator end;
	std::vector<Token>::const_iterator token_iter;

	ScopeStack scope_stack;

	std::unordered_map<StringSlice, int> op_prec; // Binary operator precidence map
	std::string binary_op_list; // Storage for operator strings, referenced by op_prec

	std::string error_message;

	AST ast;

	void add_op_prec(const char* op, int prec) {
		auto itr = binary_op_list.cend();
		binary_op_list.append(op);
		op_prec.emplace(StringSlice(itr, binary_op_list.cend()), prec);
	}

	int get_op_prec(StringSlice symbol) {
		if (op_prec.count(symbol) > 0)
			return op_prec[symbol];
		else
			return 0;
	}

	// Log/throw parse error
	void parsing_error(Token t, std::string msg = "") {
		std::ostringstream fmt;
		fmt << "\x1b[31;1mParse error:\033[0m \033[1m" << file_path << ":" << t.line + 1 << ":" << t.column << ":\033[0m\n    " << msg << "\n";
		error_message = fmt.str();
		std::cout << error_message;
		throw ParseError {t};
	}

public:
	Parser(std::string file_path, const std::vector<Token>& tokens): file_path {file_path}, begin {tokens.cbegin()}, end {tokens.cend()}, token_iter {tokens.cbegin()} {
		// Build operator precidence map
		// Note that this is only for function-like binary operators.
		// Non-function-like operators such as . have their own rules.
		// Unary operators always bind more tightly than binary operators.
		binary_op_list.reserve(256); // Make sure we have enough space to avoid iterator invalidation
		binary_op_list.append(" "); // To get it started, so that add_op_prec()'s logic works

		add_op_prec("*", 100); // Multiply
		add_op_prec("/", 100); // Divide
		add_op_prec("//", 100); // Modulus/remainder

		add_op_prec("+", 90); // Add
		add_op_prec("-", 90); // Subtract

		add_op_prec("<<", 80); // Bit shift left
		add_op_prec(">>", 80); // Bit shift right

		add_op_prec("<", 70); // Less than
		add_op_prec(">", 70); // Greater than
		add_op_prec("<=", 70); // Less than or equal
		add_op_prec(">=", 70); // Greater than or equal

		add_op_prec("==", 60); // Equal
		add_op_prec("!=", 60); // Not equal

		add_op_prec("&", 50); // Bit-wise and

		add_op_prec("^", 40); // Bit-wise xor

		add_op_prec("|", 30); // Bit-wise or

		add_op_prec("and", 20); // Logical and

		add_op_prec("or", 10); // Logical or

		add_op_prec("=", -10); // Assignment
	}


	AST parse() {
		ast.root = std::unique_ptr<NamespaceNode>(new NamespaceNode());
		std::vector<NamespaceNode*> namespaces;
		std::vector<DeclNode*> declarations;

		// Iterate over the tokens and collect all top-level
		// declarations and namespaces
		while (token_iter < end) {
			skip_comments_and_newlines();

			// Call the appropriate parsing function for the token type
			switch (token_iter->type) {
					// Declarations
				case K_CONST:
				case K_LET:
				case K_VAR:
				case K_FN:
				case K_STRUCT: {
					declarations.push_back(parse_declaration().release());
					break;
				}

				case K_NAMESPACE:
					// TODO
					parsing_error(*token_iter, "TODO: namespaces not yet implemented.");
					break;

				case LEX_EOF:
					goto done;

					// Something else, not allowed at this level
				default: {
					// Error
					parsing_error(*token_iter, "Only declarations are allowed at the namespace level");
				}
			}

		}

done:

		// Move lists of declarations and namespaces into root
		ast.root->namespaces = ast.store.alloc_from_iters(namespaces.begin(), namespaces.end());
		ast.root->declarations = ast.store.alloc_from_iters(declarations.begin(), declarations.end());

		// Return the AST
		return std::move(ast);
	}


private:

	////////////////////
	// Helper Methods
	////////////////////

	void skip_comments() {
		while (token_iter->type == COMMENT || token_iter->type == DOC_COMMENT)
			++token_iter;
	}


	void skip_comments_and_newlines() {
		while (token_iter->type == COMMENT || token_iter->type == DOC_COMMENT || token_iter->type == NEWLINE)
			++token_iter;
	}

	// Returns whether the token is a function identifier or operator
	bool token_is_const_function(Token t) {
		if (t.type == OPERATOR) {
			return true;
		} else if (t.type == IDENTIFIER &&
		           scope_stack.is_symbol_in_scope(t.text) &&
		           scope_stack.symbol_type(t.text) == SymbolType::CONST_FUNCTION
		          ) {
			return true;
		} else {
			return false;
		}
	}

	// Returns whether the token is a variable identifier
	bool token_is_variable(Token t) {
		if (t.type == IDENTIFIER &&
		        scope_stack.is_symbol_in_scope(t.text) &&
		        scope_stack.symbol_type(t.text) == SymbolType::VARIABLE
		   ) {
			return true;
		} else {
			return false;
		}
	}

	bool token_in_scope(Token t) {
		return scope_stack.is_symbol_in_scope(t.text);
	}

	// Throws an error if the given token isn't in scope
	void assert_in_scope(Token t) {
		if (!scope_stack.is_symbol_in_scope(t.text)) {
			std::ostringstream msg;
			msg << "No symbol in scope named '" << token_iter->text << "'.";
			parsing_error(*token_iter, msg.str());
		}
	}

	// Returns whether the token is a terminator token, i.e. a token
	// that ends an expression.
	bool token_is_terminator(Token t) {
		return (
		           t.type == NEWLINE ||
		           t.type == COMMA ||
		           t.type == RPAREN ||
		           t.type == RSQUARE ||
		           t.type == RCURLY
		       );
	}


	////////////////////////////////////////////////
	// Parser Methods
	//
	// All the parsing methods below should adhere to the following
	// conventions:
	//
	// - When they are called, they assume token_iter is on the first
	//   character for them to consume.
	//
	// - When they return, they leave token_iter on the first character that
	//   they don't consume (as opposed to the last character they do).  In
	//   particular they should not consume trailing whitespace unless it
	//   is actually syntactically meaningful to them.
	//
	// - When calling another parsing method, the call should be done in a
	//   state consistent with the above, and handle things afterwards
	//   assuming a state consistent with the above.
	////////////////////////////////////////////////


	// Statement
	// Parses a single full statement.  In general this means a declaration
	// or an expression, but also includes things like return and break
	// statements.
	std::unique_ptr<StatementNode> parse_statement() {
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


	// Expression
	// Parses the largest number of tokens that result in a single valid
	// expression.
	std::unique_ptr<ExprNode> parse_expression() {
		std::unique_ptr<ExprNode> lhs;

		// LHS
		lhs = parse_primary_expression();

		// RHS
		// Now that we have an lhs, let's see if there are any binary ops
		// following it.
		if (token_is_terminator(*token_iter)) {
			return lhs;
		} else if (token_is_const_function(*token_iter)) {
			// Parse binary operator
			lhs = parse_binary_func_call(std::move(lhs), -1000000);
		} else {
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
	std::unique_ptr<ExprNode> parse_primary_expression() {
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
					} else {
						// TODO
						std::ostringstream msg;
						msg << "TODO: can't parse const functions as values yet. ('" << token_iter->text << "')";
						parsing_error(*token_iter, msg.str());
					}
				}
				// Token is variable
				else if (token_is_variable(*token_iter)) {
					std::unique_ptr<ExprNode> var = std::unique_ptr<VariableNode>(new VariableNode {token_iter->text});
					++token_iter;
					return var;
				}
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


	// Declaration
	std::unique_ptr<DeclNode> parse_declaration() {
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
	std::unique_ptr<ConstantDeclNode> parse_constant_decl() {
		auto node = std::unique_ptr<ConstantDeclNode>(new ConstantDeclNode);

		++token_iter;
		skip_comments_and_newlines();

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
		skip_comments();

		// Optional ":"
		if (token_iter->type == COLON) {
			++token_iter;
			skip_comments_and_newlines();

			if (token_iter->type == IDENTIFIER) {
				// TODO
				++token_iter;
				node->type = std::unique_ptr<TypeExprNode>(new TypeExprNode());
			} else {
				// Error
				std::ostringstream msg;
				msg << "Invalid type name: '" << token_iter->text << "'.";
				parsing_error(*token_iter, msg.str());
			}
		} else {
			// Unknown type
			// TODO
			node->type = std::unique_ptr<TypeExprNode>(new TypeExprNode());
		}

		skip_comments();

		// Initializer is required for constants
		if (token_iter->type != OPERATOR || token_iter->text != "=") {
			// Error
			std::ostringstream msg;
			msg << "Constant '" << node->name << "' has no initializer.";
			parsing_error(*token_iter, msg.str());
		}

		++token_iter;
		skip_comments_and_newlines();
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

		skip_comments();

		if (!token_is_terminator(*token_iter)) {
			// Error
			std::ostringstream msg;
			msg << "Invalid continuation of initializer. ('" << token_iter->text << "')";
			parsing_error(*token_iter, msg.str());
		}

		return node;
	}


	// Variable
	std::unique_ptr<VariableDeclNode> parse_variable_decl() {
		auto node = std::unique_ptr<VariableDeclNode>(new VariableDeclNode);

		if (token_iter->type == K_LET)
			node->mut = false;
		else if (token_iter->type == K_VAR)
			node->mut = true;

		// Variable name
		++token_iter;
		skip_comments_and_newlines();
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
		skip_comments();

		// Optional ":"
		if (token_iter->type == COLON) {
			++token_iter;
			skip_comments_and_newlines();

			if (token_iter->type == IDENTIFIER) {
				// TODO
				++token_iter;
				node->type = std::unique_ptr<TypeExprNode>(new TypeExprNode());
			} else {
				// Error
				std::ostringstream msg;
				msg << "Invalid type name: '" << token_iter->text << "'.";
				parsing_error(*token_iter, msg.str());
			}
		} else {
			// Unknown type
			// TODO
			node->type = std::unique_ptr<TypeExprNode>(new TypeExprNode());
		}

		skip_comments();

		// Optional "="
		if (token_iter->type == OPERATOR && token_iter->text == "=") {
			++token_iter;
			skip_comments();

			node->initializer = parse_expression();
		} else {
			// No initializer
			// TODO
			node->initializer = std::unique_ptr<ExprNode>(new ExprNode());
		}

		skip_comments();

		if (!token_is_terminator(*token_iter)) {
			// Error
			std::ostringstream msg;
			msg << "Invalid continuation of expression: '" << token_iter->text << "'.";
			parsing_error(*token_iter, msg.str());
		}

		return node;
	}


	// Function definition
	std::unique_ptr<ConstantDeclNode> parse_func_definition() {
		// A function is really just a constant with a function
		// literal assigned to it.
		auto node = std::unique_ptr<ConstantDeclNode>(new ConstantDeclNode);

		// Function name
		++token_iter;
		skip_comments_and_newlines();
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
		skip_comments_and_newlines();
		node->initializer = parse_function_literal(false);

		// TODO: type
		node->type = std::unique_ptr<TypeExprNode>(new TypeExprNode());

		return node;
	}


	// Standard function call syntax
	std::unique_ptr<FuncCallNode> parse_standard_func_call() {
		auto node = std::unique_ptr<FuncCallNode>(new FuncCallNode());

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

			node->parameters.push_back(parse_expression());

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

		return node;
	}


	// Unary function call syntax
	std::unique_ptr<FuncCallNode> parse_unary_func_call() {
		auto node = std::unique_ptr<FuncCallNode>(new FuncCallNode());

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
		node->parameters.push_back(parse_primary_expression());

		return node;
	}


	// Binary infix function call syntax
	std::unique_ptr<ExprNode> parse_binary_func_call(std::unique_ptr<ExprNode> lhs, int lhs_prec) {
		auto node = std::unique_ptr<FuncCallNode>(new FuncCallNode());
		std::unique_ptr<ExprNode> rhs;

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
				node->parameters.push_back(std::move(lhs));
				node->parameters.push_back(std::move(rhs));
				break;
			} else if (lhs_prec >= my_prec) {
				token_iter = pre_rhs;
				return lhs;
			} else if (token_is_const_function(*token_iter)) {
				if (get_op_prec(token_iter->text) > my_prec) {
					rhs = parse_binary_func_call(std::move(rhs), my_prec);
				} else {
					node->parameters.push_back(std::move(lhs));
					node->parameters.push_back(std::move(rhs));
					return parse_binary_func_call(std::move(node), lhs_prec);
				}
			} else {
				// Error
				std::ostringstream msg;
				msg << "GAHWHATDOESTHISMEAN??? parse_binary_func_call()";
				parsing_error(*token_iter, msg.str());
			}
		}

		return std::unique_ptr<ExprNode>(std::move(node));;
	}


	// Scope
	std::unique_ptr<ScopeNode> parse_scope() {
		auto node = std::unique_ptr<ScopeNode>(new ScopeNode());

		// Open scope
		if (token_iter->type != LPAREN) {
			// Error
			std::ostringstream msg;
			msg << "Opening scope with wrong character: '" << token_iter->text << "'.";
			parsing_error(*token_iter, msg.str());
		}
		++token_iter;

		// Push this scope
		scope_stack.push_scope();

		while (true) {
			skip_comments_and_newlines();

			// Close scope?
			if (token_iter->type == RPAREN) {
				++token_iter;
				break;
			}
			// Should be an expression
			else {
				node->statements.push_back(parse_statement());
			}
		}

		// Pop this scope
		scope_stack.pop_scope();

		return node;
	}




	////////////////////////////////////////////////////////
	// LITERALS
	////////////////////////////////////////////////////////

	std::unique_ptr<LiteralNode> parse_literal() {
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


	std::unique_ptr<FuncLiteralNode> parse_function_literal(bool has_fn = true) {
		auto node = std::unique_ptr<FuncLiteralNode>(new FuncLiteralNode);

		if (has_fn) {
			if (token_iter->type == K_FN) {
				++token_iter;
				skip_comments_and_newlines();
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
			skip_comments_and_newlines();
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
			skip_comments_and_newlines();
			if (token_iter->type != COLON) {
				// Error
				std::ostringstream msg;
				msg << "Function parameter lacks a type.";
				parsing_error(*token_iter, msg.str());
			}

			// Parameter type
			// TODO: types aren't just names, need to evaluate full type expression here.
			++token_iter;
			skip_comments_and_newlines();
			if (token_iter->type == IDENTIFIER) {
				node->parameters.push_back(NameTypePair {name, std::unique_ptr<TypeExprNode>(new TypeExprNode())});

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
			skip_comments_and_newlines();
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

		// -> (optional return type)
		++token_iter;
		skip_comments_and_newlines();
		if (token_iter->type == OPERATOR && token_iter->text == "->") {
			// Return type
			// TODO: types aren't just names, need to evaluate full type expression here.
			++token_iter;
			skip_comments_and_newlines();
			if (token_iter->type == IDENTIFIER)
				node->return_type = std::unique_ptr<TypeExprNode>(new TypeExprNode());
			else {
				// Error
				std::ostringstream msg;
				msg << "Invalid type name for return type: '" << token_iter->text << "'.";
				parsing_error(*token_iter, msg.str());
			}
		} else {
			// TODO: empty return type
			node->return_type = std::unique_ptr<TypeExprNode>(new TypeExprNode());
		}

		// Function body
		++token_iter;
		skip_comments_and_newlines();
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
};


AST parse_tokens(const char* file_path, const std::vector<Token>& tokens)
{
	Parser parser(file_path, tokens);
	AST ast;

	ast = parser.parse();

	ast.print();

	return ast;
}