#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>

#include "lexer_utils.hpp"
#include "tokens.hpp"


/**
 * Takes an input string encoded in utf8 and lexes it into a vector of tokens.
 */
std::vector<Token> lex_string(const std::string& input);


#endif // LEXER_HPP