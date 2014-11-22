#ifndef PARSER_HPP
#define PARSER_HPP

#include "tokens.hpp"
#include "ast.hpp"

#include <vector>


class ParseError: std::exception
{
public:
	Token token;
	ParseError(Token token): token {token} {}
	virtual const char* what() const noexcept {
		return "Parse error.";
	}
};


AST parse_tokens(const char* file_path, const std::vector<Token>& tokens);



#endif // PARSER_HPP