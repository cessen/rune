#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include "memory_arena.hpp"
#include "scope_stack.hpp"
#include "string_slice.hpp"
#include "tokens.hpp"
#include "type.hpp"

static void print_indent(int indent)
{
	for (int i = 0; i < indent; ++i) {
		std::cout << "\t";
	}
}

struct CodeSlice {
	unsigned int line = 0;
	unsigned int column = 0;
	StringSlice text;

	CodeSlice& operator=(const Token& token)
	{
		line = token.line;
		column = token.column;
		text = token.text;

		return *this;
	}
};

////////////////////////////////////////////////////////////////
// Basic building blocks and base classes
////////////////////////////////////////////////////////////////

/**
 * Base class for nodes in the AST.  Basically just exists to ensure
 * virtual destructor.
 */
struct ASTNode {
	CodeSlice code;

	virtual ~ASTNode() {}

	virtual void print(int indent)
	{
		print_indent(indent);
		std::cout << "EMPTY_ASTNode";
	}


};


/**
 * Base class for Expression and Declaration nodes.
 */
struct StatementNode: ASTNode {
	virtual void print(int indent)
	{
		print_indent(indent);
		std::cout << "EMPTY_Statement";
	}
};


/**
 * Expression node base class.
 */
struct ExprNode: StatementNode {
	Type* eval_type = nullptr;  // Type that the expression evaluates to

	virtual void print(int indent) = 0;
};


/**
 * Declaration node base class.
 */
struct DeclNode : StatementNode {
	StringSlice name;
	Type* type;
	ExprNode* initializer = nullptr;

	DeclNode() {}
	DeclNode(StringSlice name, Type* type, ExprNode* init) : name { name }, type { type }, initializer { init } {}

	virtual void print(int indent)
	{
		print_indent(indent);
		std::cout << "EMPTY_Decl";
	}
};


/**
 * Namespace node.
 */
struct NamespaceNode: ASTNode {
	StringSlice name;
	Slice<NamespaceNode*> namespaces;
	Slice<DeclNode*> declarations;

	virtual void print(int indent)
	{
		print_indent(indent);
		std::cout << "namespace " << name << " {" << std::endl;
		for (auto& n: namespaces) {
			n->print(indent+1);
			std::cout << std::endl;
		}
		for (auto& d: declarations) {
			d->print(indent+1);
			std::cout << std::endl;
		}

		std::cout << "}" << std::endl;
	}
};


/**
 * Scope node.
 */
struct ScopeNode: ExprNode {
	Slice<StatementNode*> statements;

	virtual void print(int indent)
	{
		print_indent(indent);
		std::cout << "(\n";
		for (auto &e: statements) {
			e->print(indent+1);
			std::cout << std::endl;
		}
		print_indent(indent);
		std::cout << ")";
	}
};


/**
 * Literal node base class.
 */
struct LiteralNode: ExprNode {
	virtual void print(int indent)
	{
		print_indent(indent);
		std::cout << "EMPTY_Literal";
	}
};




////////////////////////////////////////////////////////////////
// Keywords
////////////////////////////////////////////////////////////////

struct ReturnNode: StatementNode {
	ExprNode* expression;

	virtual void print(int indent)
	{
		print_indent(indent);
		std::cout << "RETURN" << std::endl;
		expression->print(indent + 1);
	}
};



////////////////////////////////////////////////////////////////
// Declarations
////////////////////////////////////////////////////////////////

struct ConstantDeclNode: DeclNode {
	virtual void print(int indent)
	{
		// Name
		print_indent(indent);
		std::cout << "CONSTANT_DECL " << name << std::endl;

		// Type
		print_indent(indent+1);
		std::cout << "TYPE" << std::endl;
		type->print(indent+2);
		std::cout << std::endl;

		// Initializer
		print_indent(indent+1);
		std::cout << "INIT" << std::endl;
		initializer->print(indent+2);
		std::cout << std::endl;
	}
};

struct VariableDeclNode : DeclNode {
	bool mut;

	VariableDeclNode() {}
	VariableDeclNode(StringSlice name, Type* type, ExprNode* init, bool mut) : DeclNode(name, type, init), mut { mut } {}

	virtual void print(int indent)
	{
		// Name
		print_indent(indent);
		std::cout << "VARIABLE_DECL " << name;
		if (mut)
			std::cout << " (mutable)";
		std::cout << std::endl;

		// Type
		print_indent(indent+1);
		std::cout << "TYPE" << std::endl;
		type->print(indent+2);
		std::cout << std::endl;

		// Initializer
		print_indent(indent+1);
		std::cout << "INIT" << std::endl;
		initializer->print(indent+2);
		std::cout << std::endl;
	}
};

struct NominalTypeDeclNode : DeclNode {
	// This Node is for identification and uses "type" from DeclNode
};


////////////////////////////////////////////////////////////////
// Literals
////////////////////////////////////////////////////////////////

struct IntegerLiteralNode: LiteralNode {
	StringSlice text;
	virtual void print(int indent)
	{
		print_indent(indent);
		std::cout << text.to_string();
	}
};

struct FloatLiteralNode: LiteralNode {
	StringSlice text;
};


struct FuncLiteralNode: LiteralNode {
	Slice<VariableDeclNode*> parameters;
	Type* return_type;
	ScopeNode* body;

	virtual void print(int indent)
	{
		// Function
		print_indent(indent);
		std::cout << "FUNCTION" << std::endl;

		// Parameters
		print_indent(indent+1);
		std::cout << "PARAMETERS" << std::endl;
		for (auto& p: parameters) {
			p->print(indent+2);
			std::cout << std::endl;
		}

		// Return type
		print_indent(indent+1);
		std::cout << "RETURN_TYPE" << std::endl;
		return_type->print(indent+2);
		std::cout << std::endl;

		// Body
		print_indent(indent+1);
		std::cout << "BODY" << std::endl;
		body->print(indent+1);
		std::cout << std::endl;
	}
};


////////////////////////////////////////////////////////////////
// Expressions
////////////////////////////////////////////////////////////////

struct EmptyExprNode : ExprNode {
	virtual void print(int indent)
	{
		print_indent(indent);
		std::cout << "EMPTY_Expr";
	}
};

struct AddressOfNode : ExprNode {
	ExprNode* expr;
	virtual void print(int indent)
	{
		print_indent(indent);
		std::cout << "ADDRESS_OF" << std::endl;
		expr->print(indent + 1);
	}
};

struct DerefNode : ExprNode {
	ExprNode* expr;
	virtual void print(int indent)
	{
		print_indent(indent);
		std::cout << "DEREF" << std::endl;
		expr->print(indent + 1);
	}
};

struct UnknownIdentifierNode : ExprNode {
	virtual void print(int indent)
	{
		print_indent(indent);
		std::cout << "Unknown Identifier \"" << code.text << "\"";
	}
};

struct VariableNode: ExprNode {
	VariableDeclNode* declaration;

	VariableNode() {}
	VariableNode(VariableDeclNode* decl) : declaration { decl }
	{
		assert(declaration != nullptr);

		eval_type = nullptr;
	}

	virtual void print(int indent)
	{
		// Name
		print_indent(indent);
		std::cout << declaration->name;
	}
};

struct ConstantNode : ExprNode {
	ConstantDeclNode* declaration;

	ConstantNode() {}
	ConstantNode(ConstantDeclNode* decl) : declaration { decl }
	{
		assert(declaration != nullptr);

		eval_type = nullptr;
	}

	virtual void print(int indent)
	{
		// Name
		print_indent(indent);
		std::cout << declaration->name;
	}
};

struct FuncCallNode: ExprNode {
	StringSlice name; //TODO change to declaration pointer
	Slice<ExprNode*> parameters;

	virtual void print(int indent)
	{
		// Name
		print_indent(indent);
		std::cout << "CALL " << name;

		// Parameters
		for (auto& p: parameters) {
			std::cout << std::endl;
			p->print(indent+1);
		}
	}
};

struct AssignmentNode: ExprNode {
	ExprNode* lhs;
	ExprNode* rhs;

	virtual void print(int indent)
	{
		print_indent(indent);
		std::cout << "ASSIGNMENT\n";
		lhs->print(indent+1);
		std::cout << std::endl;
		rhs->print(indent+1);
		std::cout << std::endl;
	}
};


////////////////////////////////////////////////////////////////
// An AST root
////////////////////////////////////////////////////////////////
class AST
{
public:
	NamespaceNode* root;
	MemoryArena<> store; // Memory store for nodes

	void print()
	{
		root->print(0);
	}

	void link_references();
	bool check_types();

private:
	void AST::_link_refs_helper(ASTNode **node_ref, ScopeStack<DeclNode*> *scope_stack);
};




////////////////////////////////////////////////////////////////
// Identification convenience functions
////////////////////////////////////////////////////////////////
bool is_node_const_func_decl(ASTNode* node);
bool is_node_variable(ASTNode* node);
bool is_node_constant(ASTNode* node);

#endif // AST_HPP
