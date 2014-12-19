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

	// Documentation string
	DOC_STRING,

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
	AT,
	COMMA,
	PERIOD,
	COLON,
	BACKTICK,

	// Keywords
	K_NAMESPACE,
	K_PUB,
	K_UNSAFE,

	K_CONST,
	K_VAL,
	K_VAR,

	K_MUT,
	K_REF,

	K_FN,

	K_STRUCT,
	K_ENUM,
	K_UNION,

	K_TRAIT,
	K_IS,

	K_IF,
	K_ELSE,
	K_LOOP,
	K_WHILE,
	K_UNTIL,
	K_FOR,
	K_IN,
	K_BREAK,
	K_CONTINUE,
	K_RETURN,

	K_AS,

	K_ALIAS,
	K_TYPE,
};

struct Token {
	TokenType type = UNKNOWN;
	unsigned int line = 0; // Line of the source file the token starts on (newline tokens are considered to be the last token of the line that they end)
	unsigned int column = 0; // Column of the line the token starts on
	StringSlice text; // A reference to the text of the token
};


#endif // TOKENS_HPP