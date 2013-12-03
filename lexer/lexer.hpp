#ifndef LEXER_HPP
#define LEXER_HPP

#include <utility>
#include <fstream>
#include <iostream>
#include <string>

enum TokenType {
    IDENTIFIER,
    OPERATOR,
    PAREN_OPEN,
    PAREN_CLOSE
};

std::string next_utf8(std::istream &in);

class Lexer
{
public:
	// TODO: change to use move semantics once gcc implements them for streams.
	// This current implementation of the constructor is delicate
	Lexer(std::istream* input): in {*input} {
		cur_c = next_utf8(in);
	}

	std::string next();

	//std::string token();

	//TokenType token_type();


private:
	std::istream& in;
	std::string cur_c;
	std::string current_token;
	TokenType current_token_type;
};

#endif // DATASTORE_HPP