#ifndef TOKENS_HPP
#define TOKENS_HPP

#include <string>

enum TokenType {
    // Catch-all
    UNKNOWN,

    // User-defined symbols
    IDENTIFIER,
    OPERATOR,

    // Literals
    INTEGER_LIT,
    FLOAT_LIT,
    STRING_LIT,

    // Comments
    COMMENT,

    // Catch-all for valid but as-of-yet unused symbols
    RESERVED,

    // Punctuation
    NEWLINE,
    LPAREN,
    RPAREN,
    LSQUARE,
    RSQUARE,
    LCURLY,
    RCURLY,
    COMMA,
    PERIOD,
    COLON,
    BACKTICK,

    // Keywords
    K_FN,
    K_STRUCT,
    K_LET,
    K_RETURN

};

struct Token {
	TokenType type = UNKNOWN;
	unsigned int line = 0; // Line of the source file the token starts on (newline tokens are considered to be the last token of the line that they end)
	unsigned int column = 0; // Column of the line the token starts on
	std::string str; // The text of the token
};


#endif // TOKENS_HPP