#include "parser.hpp"

#include "ast.hpp"
#include "tokens.hpp"
#include "string_slice.hpp"
#include "scope_stack.hpp"

#include <iostream>
#include <exception>
#include <string>
#include <unordered_map>


class ParseError: std::exception
{
public:
	Token token;
	ParseError(Token token): token {token} {}
	virtual const char* what() const noexcept {
		return "Parse error.";
	}
};


class Parser
{
	std::vector<Token>::const_iterator begin;
	std::vector<Token>::const_iterator end;
	std::vector<Token>::const_iterator token_iter;

	ScopeStack scope_stack;

	std::unordered_map<StringSlice, int> op_prec; // Binary operator precidence map
	std::string binary_op_list; // Storage for operator strings, referenced by op_prec

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

public:
	Parser(const std::vector<Token>& tokens): begin {tokens.cbegin()}, end {tokens.cend()}, token_iter {tokens.cbegin()} {
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
		// Iterate over the tokens
		while (token_iter < end) {
			skip_comments_and_newlines();

			// Call the appropriate parsing function for the token type
			switch (token_iter->type) {
					// Function declaration
				case K_FUNC: {
					auto subtree = parse_func_definition();
					// TODO: ast.roots.push_back(subtree);
					break;
				}

				case LEX_EOF:
					goto done;

					// Something else, not allowed at this level
				default:
					throw ParseError {*token_iter};
			}

		}

done:

		return std::move(ast);
	}


private:

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
		           scope_stack.symbol_type(t.text) == SymbolType::FUNCTION
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
				throw ParseError {*token_iter};
			}

			// Declaration
			case K_FUNC:
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
				throw ParseError {*token_iter};
			}

			// Identifier or operator, means this is a more
			// complex case.
			case IDENTIFIER:
			case OPERATOR: {
				// Check if symbol is in scope
				if (!scope_stack.is_symbol_in_scope(token_iter->text)) {
					throw ParseError {*token_iter};
				}

				// If next token is a terminator, it's a singleton
				if (token_iter[1].type == NEWLINE || token_iter[1].type == COMMA) {
					// TODO
					throw ParseError {*token_iter};
				}
				// If next token is a [, it's a function call
				else if (token_iter[1].type == LSQUARE) {
					return parse_standard_func_call();
				}
				// Otherwise it's a compound expression
				else {
					return parse_compound_expression();
				}
			}

			default: {
				throw ParseError {*token_iter};
			}
		}
	}


	// Compound expression
	// Parses the largest number of tokens that result in a single valid
	// expression.
	std::unique_ptr<ExprNode> parse_compound_expression() {
		std::unique_ptr<ExprNode> lhs;

		// If it's a unary operator, parse to the first
		// non-operator
		if (token_is_function(*token_iter)) {
			// Unary operator, consume as many unary operators as possible
			lhs = parse_unary_func_call();
		}

		if (token_iter->type == NEWLINE || token_iter->type == COMMA) {
			return lhs;
		} else if (token_is_function(*token_iter)) {
			// Parse binary operator
			lhs = parse_binary_func_call(std::move(lhs));
		} else {
			throw ParseError {*token_iter};
		}

		return lhs;
	}


	// Declaration
	std::unique_ptr<DeclNode> parse_declaration() {
		switch (token_iter->type) {
			case K_FUNC: {
				return parse_func_definition();
			}

			default: {
				throw ParseError {*token_iter};
			}
		}
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
			throw ParseError {*token_iter};
		}

		// Open bracket
		++token_iter;
		skip_comments_and_newlines();
		if (token_iter->type != LSQUARE)
			throw ParseError {*token_iter};

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
			else
				throw ParseError {*token_iter};

			// Colon
			++token_iter;
			skip_comments_and_newlines();
			if (token_iter->type != COLON)
				throw ParseError {*token_iter};

			// Parameter type
			// TODO: types aren't just names, need to evaluate full type expression here.
			++token_iter;
			skip_comments_and_newlines();
			if (token_iter->type == IDENTIFIER)
				node->parameters.push_back(NameTypePair {name, std::unique_ptr<TypeExprNode>(new TypeExprNode())});
			else
				throw ParseError {*token_iter};

			// Either a comma or closing square bracket
			++token_iter;
			skip_comments_and_newlines();
			if (token_iter->type == COMMA)
				continue;
			else if (token_iter->type == RSQUARE)
				break;
			else
				throw ParseError {*token_iter};
		}

		// ->
		++token_iter;
		skip_comments_and_newlines();
		if (token_iter->type != OPERATOR || token_iter->text != "->")
			throw ParseError {*token_iter};

		// Return type
		// TODO: types aren't just names, need to evaluate full type expression here.
		++token_iter;
		skip_comments_and_newlines();
		if (token_iter->type == IDENTIFIER)
			node->return_type = std::unique_ptr<TypeExprNode>(new TypeExprNode());
		else
			throw ParseError {*token_iter};

		// Push name onto scope stack
		scope_stack.push_symbol(node->name, SymbolType::FUNCTION);

		// Function body
		// TODO
		++token_iter;
		skip_comments_and_newlines();
		if (token_iter->type == LPAREN) {
			node->body = parse_scope();
		}

		std::cout << "\tFunction definition: " << node->name << std::endl;

		return node;
	}


	// Standard function call syntax
	std::unique_ptr<FuncCallNode> parse_standard_func_call() {
		auto node = std::unique_ptr<FuncCallNode>(new FuncCallNode());

		// Get function name
		if (token_iter->type == IDENTIFIER || token_iter->type == OPERATOR) {
			node->name = token_iter->text;
		} else {
			throw ParseError {*token_iter};
		}

		// [
		++token_iter;
		skip_comments();
		if (token_iter->type != LSQUARE)
			throw ParseError {*token_iter};
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
			throw ParseError {*token_iter};
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
				throw ParseError {*token_iter};
			}

			// Identifier or operator
			case IDENTIFIER:
			case OPERATOR: {
				// Check if symbol is in scope
				if (!scope_stack.is_symbol_in_scope(token_iter->text)) {
					throw ParseError {*token_iter};
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
					// TODO
					throw ParseError {*token_iter};
				}
				// Otherwise, error
				else {
					throw ParseError {*token_iter};
				}
				break;
			}

			default: {
				throw ParseError {*token_iter};
			}
		}

		return node;
	}


	// Binary infix function call syntax
	std::unique_ptr<FuncCallNode> parse_binary_func_call(std::unique_ptr<ExprNode> lhs) {
		auto node = std::unique_ptr<FuncCallNode>(new FuncCallNode());
		std::unique_ptr<ExprNode> rhs;

		if (!token_is_function(*token_iter)) {
			throw ParseError {*token_iter};
		}

		// Op info
		node->name = token_iter->text;
		const int my_prec = get_op_prec(token_iter->text);

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
				throw ParseError {*token_iter};
			}

			// Identifier or operator
			case IDENTIFIER:
			case OPERATOR: {
				// Check if symbol is in scope
				if (!scope_stack.is_symbol_in_scope(token_iter->text)) {
					throw ParseError {*token_iter};
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
					// TODO
					throw ParseError {*token_iter};
				}
				// Otherwise, error
				else {
					throw ParseError {*token_iter};
				}
				break;
			}

			default: {
				throw ParseError {*token_iter};
			}
		}

		while (true) {
			if (token_iter->type == NEWLINE || token_iter->type == COMMA || token_iter->type == RPAREN) {
				break;
			} else if (token_is_function(*token_iter)) {
				if (get_op_prec(token_iter->text) > my_prec) {
					rhs = parse_binary_func_call(std::move(rhs));
				} else {
					break;
				}
			} else {
				throw ParseError {*token_iter};
			}
		}

		node->parameters.push_back(std::move(lhs));
		node->parameters.push_back(std::move(rhs));

		return node;
	}


	// Scope
	std::unique_ptr<ScopeNode> parse_scope() {
		auto node = std::unique_ptr<ScopeNode>(new ScopeNode());

		// Open scope
		if (token_iter->type != LPAREN)
			throw ParseError {*token_iter};
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


AST parse_tokens(const std::vector<Token>& tokens)
{
	Parser parser(tokens);
	AST ast;

	try {
		ast = parser.parse();
	} catch (ParseError e) {
		std::cout << "Parse Error: " << "[L" << e.token.line + 1 << ", C" << e.token.column << ", " << e.token.type << "]:\t" << " " << e.token.text << std::endl;
		throw e;
	}

	return ast;
}