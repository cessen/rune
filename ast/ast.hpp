#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <memory>
#include <vector>
#include "string_slice.hpp"

// We use std::unique_ptr's a lot in this code, so make it shorter
template <typename T>
using uptr = std::unique_ptr<T>;

// Same with std::vector's of std::unique_ptr's
template <typename T>
using uptr_vec = std::vector<std::unique_ptr<T>>;



static void print_indent(int indent)
{
	for (int i = 0; i < indent; ++i) {
		std::cout << "\t";
	}
}

////////////////////////////////////////////////////////////////
// Basic building blocks and base classes
////////////////////////////////////////////////////////////////

/**
 * Base class for nodes in the AST.  Basically just exists to ensure
 * virtual destructor.
 */
struct ASTNode {
	virtual void print(int indent) {}
	virtual ~ASTNode() {}
};


/**
 * Type expression node base class.
 */
struct TypeExprNode: ASTNode {
	virtual void print(int indent) {
		print_indent(indent);
		std::cout << "EMPTY_TypeExpr";
	}
};


/**
 * Expression node base class.
 */
struct ExprNode: ASTNode {
	uptr<TypeExprNode> eval_type;  // Type that the expression evaluates to

	virtual void print(int indent) {
		print_indent(indent);
		std::cout << "EMPTY_Expr";
	}
};


/**
 * Declaration node base class.
 */
struct DeclNode: ExprNode {
	virtual void print(int indent) {
		print_indent(indent);
		std::cout << "EMPTY_Decl";
	}
};


/**
 * Namespace node.
 */
struct NamespaceNode: ASTNode {
	StringSlice name;
	uptr_vec<NamespaceNode> namespaces;
	uptr_vec<DeclNode> declarations;

	virtual void print(int indent) {
		print_indent(indent);
		std::cout << "namespace " << name << " {" << std::endl;
		for (const auto &n: namespaces) {
			n->print(indent+1);
			std::cout << std::endl;
		}
		for (const auto &d: declarations) {
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
	uptr_vec<ExprNode> expressions;

	virtual void print(int indent) {
		print_indent(indent);
		std::cout << "(\n";
		for (auto &e: expressions) {
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
	virtual void print(int indent) {
		print_indent(indent);
		std::cout << "EMPTY_Literal";
	}
};




////////////////////////////////////////////////////////////////
// Helper classes
////////////////////////////////////////////////////////////////

struct NameTypePair {
	StringSlice name;
	uptr<TypeExprNode> type;

	void print(int indent) {
		print_indent(indent);
		std::cout << name << ":\n";
		type->print(indent+1);
	}
};




////////////////////////////////////////////////////////////////
// Declarations
////////////////////////////////////////////////////////////////

struct VariableDeclNode: DeclNode {
	StringSlice name;
	uptr<TypeExprNode> type;
	uptr<ExprNode> initializer;

	virtual void print(int indent) {
		// Name
		print_indent(indent);
		std::cout << "VARIABLE " << name << std::endl;

		// Return type
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

struct FuncDeclNode: DeclNode {
	StringSlice name;
	std::vector<NameTypePair> parameters;
	uptr<TypeExprNode> return_type;
	uptr<ScopeNode> body;

	virtual void print(int indent) {
		// Name
		print_indent(indent);
		std::cout << "FUNCTION " << name << std::endl;

		// Parameters
		print_indent(indent+1);
		std::cout << "PARAMETERS" << std::endl;
		for (auto& p: parameters) {
			p.print(indent+2);
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

struct StructDeclNode: DeclNode {
	StringSlice name;
	std::vector<NameTypePair> fields;
};

struct EnumDeclNode: DeclNode {
	StringSlice name;
	std::vector<NameTypePair> variants;
};


////////////////////////////////////////////////////////////////
// Literals
////////////////////////////////////////////////////////////////

struct IntegerLiteralNode: LiteralNode {
	StringSlice text;
};

struct FloatLiteralNode: LiteralNode {
	StringSlice text;
};




////////////////////////////////////////////////////////////////
// Expressions
////////////////////////////////////////////////////////////////

struct VariableNode: ExprNode {
	StringSlice name;

	VariableNode(StringSlice name): name {name}
	{}

	virtual void print(int indent) {
		// Name
		print_indent(indent);
		std::cout << name;
	}
};

struct FuncNode: ExprNode {
	StringSlice name;
};

struct FuncCallNode: ExprNode {
	StringSlice name;
	uptr_vec<ExprNode> parameters;

	virtual void print(int indent) {
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


////////////////////////////////////////////////////////////////
// An AST root
////////////////////////////////////////////////////////////////
class AST
{
public:
	uptr<NamespaceNode> root;

	void print() {
		root->print(0);
	}
};


#endif // AST_HPP