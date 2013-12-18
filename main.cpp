#include "config.h"

#include <fstream>
#include <iostream>

#include "lexer.hpp"
#include "parser.hpp"

int main(int argc, char** argv)
{
	std::cout << "NLang v" << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << "\n";

	if (argc < 2) {
		std::cout << "You must specify a file to compile.\n";
		return 0;
	}


	std::cout << "Reading file..." << std::endl;

	// Read the file into a string
	std::ifstream f(argv[1], std::ios::in | std::ios::binary);
	std::string contents;
	if (f) {
		f.seekg(0, std::ios::end);
		contents.resize(f.tellg());
		f.seekg(0, std::ios::beg);
		f.read(&contents[0], contents.size());
		f.close();
	}

	std::cout << "Lexing..." << std::endl;

	auto tokens = lex_string(contents);

	if (argc > 2) {
		parse_tokens(tokens);
	}

	return 0;
}