#ifndef AST_HPP
#define AST_HPP

#include <memory>
#include <vector>
#include "string_slice.hpp"

struct NamespaceNode;

// An AST root
class AST
{
public:
	std::vector<std::unique_ptr<NamespaceNode>> root;
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
 * Expression node base class.
 */
struct ExprNode: ASTNode {
};


/**
 * Type expression node base class.
 */
struct TypeExprNode: ASTNode {
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
	std::vector<std::unique_ptr<NamespaceNode>> namespaces;
	std::vector<std::unique_ptr<DeclNode>> declarations;
};


/**
 * Scope node.
 */
struct ScopeNode: ExprNode {
	std::vector<std::unique_ptr<ExprNode>> expressions;
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
	std::unique_ptr<TypeExprNode> type;
};




////////////////////////////////////////////////////////////////
// Declarations
////////////////////////////////////////////////////////////////

struct VariableDeclNode: DeclNode {
	StringSlice name;
	std::unique_ptr<TypeExprNode> type;
	std::unique_ptr<ExprNode> initializer;
};

struct FuncDeclNode: DeclNode {
	StringSlice name;
	std::vector<NameTypePair> parameters;
	std::unique_ptr<TypeExprNode> return_type;
	std::unique_ptr<ScopeNode> body;
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

struct FuncCallNode: ExprNode {
	StringSlice name;
	std::vector<std::unique_ptr<ExprNode>> parameters;
};


#endif // AST_HPP