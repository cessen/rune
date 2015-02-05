#include "string_slice.hpp"

struct ConstantDeclNode;

void InitBuiltins();
ConstantDeclNode* GetBuiltin(StringSlice name);