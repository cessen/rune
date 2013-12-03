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

	std::ifstream f {argv[1]};

	Lexer l {&f};

	std::string s;

	s = l.next();
	while (s != std::string("")) {
		std::cout << s << std::endl;
		s = l.next();
	}

	return 0;
}