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


	AST parse() {
		// Iterate over the tokens
		for (; token_iter < end; ++token_iter) {
			skip_comments_and_newlines();

			// Call the appropriate parsing function for the token type
			switch (token_iter->type) {
					// Function declaration
				case K_FUNC: {
					auto subtree = func_definition();
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


	FuncDefNode func_definition() {
		FuncDefNode node;

		// Function name
		++token_iter;
		skip_comments_and_newlines();
		if (token_iter->type == IDENTIFIER || token_iter->type == OPERATOR) {
			node.name = token_iter->text;
		} else {
			throw ParseError {*token_iter};
		}

		// Open bracker
		++token_iter;
		skip_comments_and_newlines();
		if (token_iter->type != LSQUARE)
			throw ParseError {*token_iter};

		// Parameters
		while (true) {
			// Parameter name
			++token_iter;
			skip_comments_and_newlines();
			if (token_iter->type == IDENTIFIER)
				node.parameter_names.push_back(token_iter->text);

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
				node.parameter_types.push_back(TypeExpr {});
			else
				throw ParseError {*token_iter};

			// Should see either a comma or closing square bracket
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
			node.return_type = TypeExpr {};
		else
			throw ParseError {*token_iter};

		// Function body
		// TODO

		std::cout << "\tFunction definition: " << node.name << std::endl;

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