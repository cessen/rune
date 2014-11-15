#include "parser.hpp"

#include "ast.hpp"
#include "tokens.hpp"

#include <iostream>
#include <exception>


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

	// TODO: scope stack

	AST ast;

public:
	Parser(const std::vector<Token>& tokens): begin {tokens.cbegin()}, end {tokens.cend()}, token_iter {tokens.cbegin()}
	{}


	void skip_comments() {
		while (token_iter->type == COMMENT || token_iter->type == DOC_COMMENT)
			++token_iter;
	}


	void skip_comments_and_newlines() {
		while (token_iter->type == COMMENT || token_iter->type == DOC_COMMENT || token_iter->type == NEWLINE)
			++token_iter;
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
			// complex case.  Handle after switch statement.
			case IDENTIFIER:
			case OPERATOR: {
				break;
			}

			default: {
				throw ParseError {*token_iter};
			}
		}

		// We know we're on an identifier or operator name, which means this
		// is either going to be a function call or just the value behind the
		// identifier or operator name.
		auto first = token_iter;
		++token_iter;
		skip_comments();

		// Just a singleton?
		if (token_iter->type == NEWLINE || token_iter->type == COMMA) {
			// TODO
			throw ParseError {*token_iter};
		}

		// Standard-style function call
		if (token_iter->type == LSQUARE) {
			token_iter = first;
			return parse_standard_func_call();
		}
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

		while (true) {
			skip_comments_and_newlines();

			// ]?
			if (token_iter->type == RSQUARE) {
				++token_iter;
				break;
			}

			// TODO: parse arguments
			++token_iter;
		}

		return node;
	}


	// Scope
	std::unique_ptr<ScopeNode> parse_scope() {
		auto node = std::unique_ptr<ScopeNode>(new ScopeNode());

		// Open scope
		if (token_iter->type != LPAREN)
			throw ParseError {*token_iter};
		++token_iter;

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