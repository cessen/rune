#ifndef C_GEN_HPP
#define C_GEN_HPP

#include <iostream>

#include "ast.hpp"

void gen_c_code(const AST& ast, std::ostream& f);

#endif // C_GEN_HPP