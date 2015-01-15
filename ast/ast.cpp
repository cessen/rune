#include "ast.hpp"
#include "type.hpp"

bool is_node_const_func_decl(ASTNode* node) {
	if (auto const_ptr = dynamic_cast<ConstantDeclNode*>(node)) {
		if (dynamic_cast<Function_T*>(const_ptr->type)) {
			return true;
		}
	}

	return false;
}

bool is_node_variable(ASTNode* node) {
	if (dynamic_cast<VariableDeclNode*>(node)) {
		return true;
	}

	return false;
}

bool is_node_constant(ASTNode* node) {
	if (dynamic_cast<ConstantDeclNode*>(node)) {
		return true;
	}

	return false;
}