#ifndef AST_HPP
#define AST_HPP

#include <memory>
#include <vector>
#include "string_slice.hpp"

// We use std::unique_ptr's a lot in this code, so make it shorter
template <typename T>
using uptr = std::unique_ptr<T>;

// Same with std::vector's of std::unique_ptr's
template <typename T>
using uptr_vec = std::vector<std::unique_ptr<T>>;



struct NamespaceNode;

// An AST root
class AST
{
public:
	uptr_vec<NamespaceNode> root;
};




////////////////////////////////////////////////////////////////
// Basic building blocks and base classes
////////////////////////////////////////////////////////////////

/**
 * Base class for nodes in the AST.  Basically just exists to ensure
 * virtual destructor.
 */
struct ASTNode {
	virtual ~ASTNode() {}
};


/**
 * Type expression node base class.
 */
struct TypeExprNode: ASTNode {
};


/**
 * Expression node base class.
 */
struct ExprNode: ASTNode {
	uptr<TypeExprNode> eval_type;  // Type that the expression evaluates to
};


/**
 * Declaration node base class.
 */
struct DeclNode: ExprNode {
};


/**
 * Namespace node.
 */
struct NamespaceNode: ASTNode {
	uptr_vec<NamespaceNode> namespaces;
	uptr_vec<DeclNode> declarations;
};


/**
 * Scope node.
 */
struct ScopeNode: ExprNode {
	uptr_vec<ExprNode> expressions;
};


/**
 * Literal node base class.
 */
struct LiteralNode: ExprNode {
};




////////////////////////////////////////////////////////////////
// Helper classes
////////////////////////////////////////////////////////////////

struct NameTypePair {
	StringSlice name;
	uptr<TypeExprNode> type;
};




////////////////////////////////////////////////////////////////
// Declarations
////////////////////////////////////////////////////////////////

struct VariableDeclNode: DeclNode {
	StringSlice name;
	uptr<TypeExprNode> type;
	uptr<ExprNode> initializer;
};

struct FuncDeclNode: DeclNode {
	StringSlice name;
	std::vector<NameTypePair> parameters;
	uptr<TypeExprNode> return_type;
	uptr<ScopeNode> body;
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
};

struct FuncNode: ExprNode {
	StringSlice name;
};

struct FuncCallNode: ExprNode {
	StringSlice name;
	uptr_vec<ExprNode> parameters;
};




#endif // AST_HPP