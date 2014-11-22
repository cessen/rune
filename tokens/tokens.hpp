#ifndef TOKENS_HPP
#define TOKENS_HPP


#include "string_slice.hpp"

enum TokenType {
    // Catch-all
    UNKNOWN,

    // EOF
    LEX_EOF,

    // User-defined symbols
    IDENTIFIER,
    OPERATOR,

    // Literals
    INTEGER_LIT,
    FLOAT_LIT,
    STRING_LIT,
    RAW_STRING_LIT,

    // Comments
    COMMENT,
    DOC_COMMENT,

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
    LGENERIC,
    RGENERIC,
    COMMA,
    PERIOD,
    COLON,
    BACKTICK,

    // Keywords
    K_CONST,
    K_LET,
    K_VAR,
    K_FUNC,
    K_STRUCT,
    K_RETURN

};

struct Token {
	TokenType type = UNKNOWN;
	unsigned int line = 0; // Line of the source file the token starts on (newline tokens are considered to be the last token of the line that they end)
	unsigned int column = 0; // Column of the line the token starts on
	StringSlice text; // A reference to the text of the token
};


#endif // TOKENS_HPP