#include "config.h"

#include <fstream>
#include <iostream>

#include "lexer.hpp"

int main(int argc, char** argv)
{
	std::cout << "NLang v" << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << "\n";

	if (argc < 2) {
		std::cout << "You must specify a file to compile.\n";
		return 0;
	}


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


	auto tokens = lex_string(contents);

	for (auto& t: tokens) {
		std::cout << "Token: " << t.type << " " << t.str << std::endl;
	}

	return 0;
}