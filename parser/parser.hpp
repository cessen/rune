#ifndef PARSER_HPP
#define PARSER_HPP

#include "tokens.hpp"
#include "ast.hpp"

#include <vector>


AST parse_tokens(const std::vector<Token>& tokens);



#endif // PARSER_HPP