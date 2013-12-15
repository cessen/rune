#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>

#include "lexer_utils.hpp"

enum TokenType {
    UNKNOWN,
    COMMENT,
    IDENTIFIER,
    OPERATOR,
    INTEGER_LIT,
    FLOAT_LIT,
    STRING_LIT,
    NEWLINE,
    RESERVED
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