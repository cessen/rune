#ifndef TOKENS_HPP
#define TOKENS_HPP

#include <string>

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


#endif // TOKENS_HPP