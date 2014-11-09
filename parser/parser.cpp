#include "parser.hpp"

#include "ast.hpp"

#include <iostream>


class Parser
{
	std::vector<Token>::const_iterator begin;
	std::vector<Token>::const_iterator end;

	AST ast;

public:
	Parser(const std::vector<Token>& tokens): begin {tokens.cbegin()}, end {tokens.cend()}
	{}

	AST parse() {
		return std::move(ast);
	}
};


AST parse_tokens(const std::vector<Token>& tokens)
{
	for (auto& t: tokens) {
		std::cout << "[L" << t.line + 1 << ", C" << t.column << ", " << t.type << "]:\t" << " " << t.text << std::endl;
	}

	Parser parser(tokens);
	AST ast = parser.parse();

	return ast;
}