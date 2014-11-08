#ifndef TOKENS_HPP
#define TOKENS_HPP

#include <string>

struct StringSlice {
	std::string::const_iterator iter {nullptr}; // Iterator to the beginning of the string slice
	std::string::const_iterator end {nullptr}; // Iterator to the end of the string slice

	std::string as_string() const {
		if (iter != end)
			return std::string(iter, end);
		else
			return std::string("");
	}

	bool operator==(const StringSlice &other) const {
		const auto length = std::distance(iter, end);
		const auto other_length = std::distance(other.iter, other.end);
		if (length != other_length)
			return false;

		for (int i = 0; i < length; ++i) {
			if (iter[i] != other.iter[i])
				return false;
		}

		return true;
	}

	bool operator==(const char* str) const {
		const auto length = std::distance(iter, end);

		int i = 0;
		for (; i < length; ++i) {
			if (iter[i] != str[i] || str[i] == '\0')
				return false;
		}

		if (str[i+1] != '\0')
			return false;

		return true;
	}
};

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
	StringSlice text; // A reference to the text of the token
};


#endif // TOKENS_HPP