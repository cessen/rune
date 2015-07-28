#include <exception>

#include "ast.hpp"
#include "scope_stack.hpp"
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

void AST::_link_refs_helper(ASTNode **node_ref, ScopeStack<DeclNode*> *scope_stack)
{
	ASTNode *_node = *node_ref;
	if (_node == nullptr) {
		// TODO: is this even remotely right????
		// (Ha ha!  Screw you, future us!)
		return;
	}

	if (auto node = dynamic_cast<NamespaceNode*>(_node)) {
		scope_stack->push_scope();

		for (auto &ns : node->namespaces) {
			_link_refs_helper(reinterpret_cast<ASTNode**>(&ns), scope_stack);
		}

		for (auto &decl : node->declarations) {
			_link_refs_helper(reinterpret_cast<ASTNode**>(&decl), scope_stack);
		}

		scope_stack->pop_scope();
	}
	else if (auto node = dynamic_cast<ScopeNode*>(_node)) {
		scope_stack->push_scope();
		for (auto &statement : node->statements) {
			_link_refs_helper(reinterpret_cast<ASTNode**>(&statement), scope_stack);
		}
		scope_stack->pop_scope();
	}
	else if (auto node = dynamic_cast<FuncLiteralNode*>(_node)) {
		scope_stack->push_scope();

		// Push parameters onto the scope stack
		for (auto &param : node->parameters) {
			_link_refs_helper(reinterpret_cast<ASTNode**>(&param), scope_stack);
		}

		// Recurse into body
		_link_refs_helper(reinterpret_cast<ASTNode**>(&node->body), scope_stack);

		scope_stack->pop_scope();
	}

	//////////////////////////////////
	// Declarations
	else if (auto node = dynamic_cast<ConstantDeclNode*>(_node)) {
		_link_refs_helper(reinterpret_cast<ASTNode**>(&node->initializer), scope_stack);

		// Hook up nominal types
		if (node->type->type_class() == TypeClass::Unknown) {
			if (scope_stack->is_symbol_in_scope(node->type->name)) {
				node->type = (*scope_stack)[node->type->name]->type;
			}
			else {
				// TODO proper error reporting
				throw std::exception();
			}
		}
		scope_stack->push_symbol(node->name, node);
	}
	else if (auto node = dynamic_cast<VariableDeclNode*>(_node)) {
		_link_refs_helper(reinterpret_cast<ASTNode**>(&node->initializer), scope_stack);

		// Hook up nominal types
		if (node->type->type_class() == TypeClass::Unknown) {
			if (scope_stack->is_symbol_in_scope(node->type->name)) {
				node->type = (*scope_stack)[node->type->name]->type;
			}
			else {
				// TODO proper error reporting
				throw std::exception();
			}
		}
		scope_stack->push_symbol(node->name, node);
	}
	else if (auto node = dynamic_cast<NominalTypeDeclNode*>(_node)) {
		scope_stack->push_symbol(node->name, node);
	}

	//////////////////////////////////
	// Expressions
	else if (auto node = dynamic_cast<AddressOfNode*>(_node)) {
		_link_refs_helper(reinterpret_cast<ASTNode**>(&node->expr), scope_stack);
	}
	else if (auto node = dynamic_cast<DerefNode*>(_node)) {
		_link_refs_helper(reinterpret_cast<ASTNode**>(&node->expr), scope_stack);
	}
	else if (auto node = dynamic_cast<UnknownIdentifierNode*>(_node)) {
		if (scope_stack->is_symbol_in_scope(node->code.text)) {
			DeclNode* entry = (*scope_stack)[node->code.text];
			if (dynamic_cast<VariableDeclNode*>(entry))
				*node_ref = this->store.alloc<VariableNode>();
			else if (dynamic_cast<ConstantDeclNode*>(entry))
				*node_ref = this->store.alloc<ConstantDeclNode>();

			(*node_ref)->code = _node->code;
			_link_refs_helper(node_ref, scope_stack);
		}
		else {
			// TODO proper error reporting
			throw std::exception();
		}
	}
	else if (auto node = dynamic_cast<VariableNode*>(_node)) {
		if (scope_stack->is_symbol_in_scope(node->code.text)) {
			if (auto decl = dynamic_cast<VariableDeclNode*>((*scope_stack)[node->code.text])) {
				node->declaration = decl;
			}
			else {
				// TODO proper error reporting
				throw std::exception();
			}
		}
		else {
			// TODO proper error reporting
			throw std::exception();
		}
	}
	else if (auto node = dynamic_cast<ConstantNode*>(_node)) {
		if (scope_stack->is_symbol_in_scope(node->code.text)) {
			if (auto decl = dynamic_cast<ConstantDeclNode*>((*scope_stack)[node->code.text])) {
				node->declaration = decl;
			}
			else {
				// TODO proper error reporting
				throw std::exception();
			}
		}
		else {
			// TODO proper error reporting
			throw std::exception();
		}
	}
	else if (auto node = dynamic_cast<FuncCallNode*>(_node)) {
		for (auto &param : node->parameters) {
			_link_refs_helper(reinterpret_cast<ASTNode**>(&param), scope_stack);
		}
	}
	else if (auto node = dynamic_cast<AssignmentNode*>(_node)) {
		_link_refs_helper(reinterpret_cast<ASTNode**>(&node->lhs), scope_stack);
		_link_refs_helper(reinterpret_cast<ASTNode**>(&node->rhs), scope_stack);
	}
	else if (auto node = dynamic_cast<ReturnNode*>(_node)) {
		_link_refs_helper(reinterpret_cast<ASTNode**>(&node->expression), scope_stack);
	}
	else if (dynamic_cast<LiteralNode*>(_node) ||
	         dynamic_cast<EmptyExprNode*>(_node)) {
		return;
	}
	else {
		// TODO proper error reporting
		throw std::exception();
	}
}

void AST::link_references()
{
	ScopeStack<DeclNode*> scope_stack;
	_link_refs_helper(reinterpret_cast<ASTNode**>(&this->root), &scope_stack);
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
		if (node->initializer == nullptr || node->initializer->eval_type == nullptr)
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