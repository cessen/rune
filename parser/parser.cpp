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

		// Iterate over the tokens
		while (token_iter < end) {
			skip_comments_and_newlines();

			// Call the appropriate parsing function for the token type
			switch (token_iter->type) {
					// Declarations
				case K_FN:
				case K_STRUCT:
				case K_LET: {
					ast.root->declarations.push_back(parse_declaration());
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
	bool token_is_function(Token t) {
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

	// Returns whether the token is a delimeter token, i.e. a token
	// that ends an expression.
	bool token_is_delimeter(Token t) {
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


	// Expression
	std::unique_ptr<ExprNode> parse_expression() {
		switch (token_iter->type) {
				// Scope
			case LPAREN: {
				return parse_scope();
			}

			// Return statement
			case K_RETURN: {
				// TODO
				parsing_error(*token_iter, "TODO: Return statements haven't been implemented yet.");
			}

			// Declaration
			case K_FN:
			case K_STRUCT:
			case K_LET: {
				return parse_declaration();
			}

			// Literal
			case INTEGER_LIT:
			case FLOAT_LIT:
			case STRING_LIT:
			case RAW_STRING_LIT: {
				// TODO
				parsing_error(*token_iter, "TODO: Parsing literals hasn't been implemented yet.");
			}

			// Identifier or operator, means this is a more
			// complex case.
			case IDENTIFIER:
			case OPERATOR: {
				// Check if symbol is in scope
				if (!scope_stack.is_symbol_in_scope(token_iter->text)) {
					// Error
					std::ostringstream msg;
					msg << "No symbol in scope named '" << token_iter->text << "'.";
					parsing_error(*token_iter, msg.str());
				}

				return parse_compound_expression();
			}

			default: {
				// Error
				std::ostringstream msg;
				msg << "Unknown expression '" << token_iter->text << "'.";
				parsing_error(*token_iter, msg.str());
				throw 0; // Silence warnings about not returning, parsing_error throws anyway
			}
		}
	}


	// Compound expression
	// Parses the largest number of tokens that result in a single valid
	// expression.
	std::unique_ptr<ExprNode> parse_compound_expression() {
		std::unique_ptr<ExprNode> lhs;

		// LHS
		// If next token is a delimeter, we have a singleton
		if (token_is_delimeter(token_iter[1])) {
			// TODO
			std::ostringstream msg;
			msg << "TODO: singleton expressions not yet supported. ('" << token_iter->text << "')";
			parsing_error(*token_iter, msg.str());
		}
		// If next token is [ then this starts with a standard function
		// call
		else if (token_iter[1].type == LSQUARE) {
			lhs = parse_standard_func_call();
		}
		// If it's a unary operator, parse to the first
		// non-operator
		else if (token_is_function(*token_iter)) {
			// Unary operator, consume as many unary operators as possible
			lhs = parse_unary_func_call();
		}
		// If it's a variable
		else if (token_is_variable(*token_iter)) {
			// TODO: do this properly
			lhs = std::unique_ptr<VariableNode>(new VariableNode {token_iter->text});
			++token_iter;
		}
		// Who knows what it is...
		else {
			// Error
			std::ostringstream msg;
			msg << "parse_compound_expression() OMGYOUSHOULDNEVERSEETHIS '" << token_iter->text << "'.";
			parsing_error(*token_iter, msg.str());
		}


		// RHS
		// Now that we have an lhs, let's see if there are any binary ops
		// following it.
		if (token_is_delimeter(*token_iter)) {
			return lhs;
		} else if (token_is_function(*token_iter)) {
			// Parse binary operator
			lhs = parse_binary_func_call(std::move(lhs), -1000000);
		} else {
			// Error
			std::ostringstream msg;
			msg << "No binary operator in scope named '" << token_iter->text << "'.";
			parsing_error(*token_iter, msg.str());
		}

		return lhs;
	}


	// Declaration
	std::unique_ptr<DeclNode> parse_declaration() {
		switch (token_iter->type) {
			case K_LET: {
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


	std::unique_ptr<VariableDeclNode> parse_variable_decl() {
		auto node = std::unique_ptr<VariableDeclNode>(new VariableDeclNode);

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

		if (!token_is_delimeter(*token_iter)) {
			// Error
			std::ostringstream msg;
			msg << "Invalid continuation of expression: '" << token_iter->text << "'.";
			parsing_error(*token_iter, msg.str());
		}

		return node;
	}


	// Function definition
	std::unique_ptr<FuncDeclNode> parse_func_definition() {
		auto node = std::unique_ptr<FuncDeclNode>(new FuncDeclNode);

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
			msg << "Attempted to declare function '" << token_iter->text << "', but something with the same name is already in scope.";
			parsing_error(*token_iter, msg.str());
		}

		// Open bracket
		++token_iter;
		skip_comments_and_newlines();
		if (token_iter->type != LSQUARE) {
			// Error
			std::ostringstream msg;
			msg << "Attempted to declare a function without a parameter list.";
			parsing_error(*token_iter, msg.str());
		}

		// Parameters
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
				msg << "Something fishy with the end of this function declaration's parameter list.";
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
			if (token_iter->type == IDENTIFIER)
				node->parameters.push_back(NameTypePair {name, std::unique_ptr<TypeExprNode>(new TypeExprNode())});
			else {
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

		// TODO: push parameter names onto scope stack

		// Function body
		++token_iter;
		skip_comments_and_newlines();
		if (token_iter->type == LPAREN) {
			node->body = parse_scope();
		} else {
			// Error
			std::ostringstream msg;
			msg << "Function definition with no body.";
			parsing_error(*token_iter, msg.str());
		}


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

		// This token should be the argument
		switch (token_iter->type) {
				// Scope
			case LPAREN: {
				node->parameters.push_back(parse_scope());
				break;
			}

			// Literal
			case INTEGER_LIT:
			case FLOAT_LIT:
			case STRING_LIT:
			case RAW_STRING_LIT: {
				// TODO
				std::ostringstream msg;
				msg << "TODO: literals are not parsed yet. ('" << token_iter->text << "').";
				parsing_error(*token_iter, msg.str());
			}

			// Identifier or operator
			case IDENTIFIER:
			case OPERATOR: {
				// Check if symbol is in scope
				if (!scope_stack.is_symbol_in_scope(token_iter->text)) {
					// Error
					std::ostringstream msg;
					msg << "Unary function call argument not in scope: '" << token_iter->text << "'.";
					parsing_error(*token_iter, msg.str());
				}

				// If next token is a [, it's a function call
				if (token_iter[1].type == LSQUARE) {
					node->parameters.push_back(parse_standard_func_call());
				}
				// If token is a function but not being called with normal
				// syntax, it has to be unary as well.
				else if (token_is_function(*token_iter)) {
					node->parameters.push_back(parse_unary_func_call());
				}
				// If token is a variable, but not being called as a function
				else if (token_is_variable(*token_iter)) {
					// TODO: do this properly
					node->parameters.push_back(std::unique_ptr<VariableNode>(new VariableNode {token_iter->text}));
					++token_iter;
				}
				// Otherwise, error
				else {
					// Error
					std::ostringstream msg;
					msg << "Invalid argument to unary function call: '" << token_iter->text << "'.";
					parsing_error(*token_iter, msg.str());
				}
				break;
			}

			default: {
				// Error
				std::ostringstream msg;
				msg << "Invalid argument to unary function call: '" << token_iter->text << "'.";
				parsing_error(*token_iter, msg.str());
			}
		}

		return node;
	}


	// Binary infix function call syntax
	std::unique_ptr<ExprNode> parse_binary_func_call(std::unique_ptr<ExprNode> lhs, int lhs_prec) {
		auto node = std::unique_ptr<FuncCallNode>(new FuncCallNode());
		std::unique_ptr<ExprNode> rhs;

		if (!token_is_function(*token_iter)) {
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
		switch (token_iter->type) {
				// Scope
			case LPAREN: {
				rhs = parse_scope();
				break;
			}

			// Literal
			case INTEGER_LIT:
			case FLOAT_LIT:
			case STRING_LIT:
			case RAW_STRING_LIT: {
				// TODO
				std::ostringstream msg;
				msg << "TODO: literals are not parsed yet. ('" << token_iter->text << "').";
				parsing_error(*token_iter, msg.str());
			}

			// Identifier or operator
			case IDENTIFIER:
			case OPERATOR: {
				// Check if symbol is in scope
				if (!scope_stack.is_symbol_in_scope(token_iter->text)) {
					// Error
					std::ostringstream msg;
					msg << "Binary function call argument not in scope: '" << token_iter->text << "'.";
					parsing_error(*token_iter, msg.str());
				}

				// If next token is a [, it's a function call
				if (token_iter[1].type == LSQUARE) {
					rhs = parse_standard_func_call();
				}
				// If token is a function but not being called with normal
				// syntax, it has to be unary.
				else if (token_is_function(*token_iter)) {
					rhs = parse_unary_func_call();
				}
				// If token is a variable, but not being called as a function
				else if (token_is_variable(*token_iter)) {
					// TODO: do this properly
					node->parameters.push_back(std::unique_ptr<VariableNode>(new VariableNode {token_iter->text}));
					++token_iter;
				}
				// Otherwise, error
				else {
					// Error
					std::ostringstream msg;
					msg << "Invalid argument to binary function call: '" << token_iter->text << "'.";
					parsing_error(*token_iter, msg.str());
				}
				break;
			}

			default: {
				// Error
				std::ostringstream msg;
				msg << "Invalid argument to binary function call: '" << token_iter->text << "'.";
				parsing_error(*token_iter, msg.str());
			}
		}

		while (true) {
			if (token_is_delimeter(*token_iter)) {
				node->parameters.push_back(std::move(lhs));
				node->parameters.push_back(std::move(rhs));
				break;
			} else if (lhs_prec >= my_prec) {
				token_iter = pre_rhs;
				return lhs;
			} else if (token_is_function(*token_iter)) {
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
				node->expressions.push_back(parse_expression());
			}
		}

		// Pop this scope
		scope_stack.pop_scope();

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