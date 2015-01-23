#include "ast.hpp"
#include "type.hpp"

bool is_node_const_func_decl(ASTNode* node)
{
	if (auto const_ptr = dynamic_cast<ConstantDeclNode*>(node)) {
		if (dynamic_cast<Function_T*>(const_ptr->type)) {
			return true;
		}
	}

	return false;
}

bool is_node_variable(ASTNode* node)
{
	if (dynamic_cast<VariableDeclNode*>(node)) {
		return true;
	}

	return false;
}

bool is_node_constant(ASTNode* node)
{
	if (dynamic_cast<ConstantDeclNode*>(node)) {
		return true;
	}

	return false;
}

static void _report_type_error(ASTNode* node_a, ASTNode* node_b)
{
	std::cout << "ERROR(" << node_a->code.line + 1 << ", " << node_a->code.column + 1 << ") Type mismatch between \"" << node_a->code.text << "\" and \"" << node_b->code.text << "\"" << std::endl;
}

static bool _check_types_helper(ASTNode *_node)
{
	if (_node == nullptr)
		return true;

	if (auto node = dynamic_cast<NamespaceNode*>(_node)) {
		for (auto i : node->namespaces) {
			if (!_check_types_helper(i))
				return false;
		}
		for (auto i : node->declarations) {
			if (!_check_types_helper(i))
				return false;
		}
	}
	else if (auto node = dynamic_cast<DeclNode*>(_node)) {
		if (!_check_types_helper(node->initializer))
			return false;

		//TODO Handle this better
		if (node->initializer->eval_type == nullptr)
			return true;

		if (*node->type != *node->initializer->eval_type) {
			_report_type_error(node, node->initializer);
			return false;
		}
	}
	else if (auto node = dynamic_cast<ScopeNode*>(_node)) {
		for (auto i : node->statements) {
			if (!_check_types_helper(i))
				return false;
		}
	}
	else if (auto node = dynamic_cast<ReturnNode*>(_node)) {
		if (!_check_types_helper(node->expression))
			return false;
	}
	else if (auto node = dynamic_cast<VariableNode*>(_node)) {
		//TODO
		node->eval_type = node->declaration->type; // Propigate type from declaration
	}
	else if (auto node = dynamic_cast<ConstantNode*>(_node)) {
		//TODO
		node->eval_type = node->declaration->type; // Propigate type from declaration
	}
	else if (auto node = dynamic_cast<FuncLiteralNode*>(_node)) {
		if (!_check_types_helper(node->body))
			return false;
	}
	else if (auto node = dynamic_cast<FuncCallNode*>(_node)) {
		//TODO
	}
	else if (auto node = dynamic_cast<AssignmentNode*>(_node)) {
		if (*node->lhs->eval_type != *node->rhs->eval_type) {
			_report_type_error(node->lhs, node->rhs);
			return false;
		}
	}

	return true;
}

bool AST::check_types()
{
	return _check_types_helper(this->root);
}