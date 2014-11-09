#include "config.h"

#include <fstream>
#include <iostream>

#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"

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

	std::cout << "Parsing..." << std::endl;
	auto ast = parse_tokens(tokens);

	if (argc > 2) {
		for (auto& t: tokens) {
			std::cout << "[L" << t.line + 1 << ", C" << t.column << ", " << t.type << "]:\t" << " " << t.text << std::endl;
		}
	}

	return 0;
}