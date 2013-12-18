#include "parser.hpp"

#include "ast.hpp"

#include <iostream>

ExprAST parse_tokens(const std::vector<Token>& tokens)
{
	ExprAST ast;

	for (auto& t: tokens) {
		std::cout << "Token: " << t.type << " " << t.str << std::endl;
	}

	return ast;
}