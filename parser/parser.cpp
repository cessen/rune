#include "parser.hpp"

#include "ast.hpp"

#include <iostream>

ExprAST parse_tokens(const std::vector<Token>& tokens)
{
	ExprAST ast;

	for (auto& t: tokens) {
		std::cout << "[L" << t.line + 1 << ", C" << t.column << ", " << t.type << "]:\t" << " " << t.text.as_string() << std::endl;
	}

	return ast;
}