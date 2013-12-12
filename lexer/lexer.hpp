#ifndef LEXER_HPP
#define LEXER_HPP

#include <utility>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "lexer_utils.hpp"

enum TokenType {
    UNKNOWN,
    IDENTIFIER,
    OPERATOR,
    PAREN_OPEN,
    PAREN_CLOSE,
    SQUARE_OPEN,
    SQUARE_CLOSE,
    CURLY_OPEN,
    CURLY_CLOSE
};

struct Token {
	TokenType type;
	std::string str;
};


/**
 * Takes an input string encoded in utf8 and lexes it into a vector of tokens.
 */
std::vector<Token> lex_string(const std::string& input);


#endif // DATASTORE_HPP