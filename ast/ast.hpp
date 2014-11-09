#ifndef AST_HPP
#define AST_HPP

#include <memory>
#include <vector>
#include "string_slice.hpp"

struct ASTNode;

// An AST root
class AST
{
public:
	std::vector<std::unique_ptr<ASTNode>> roots;
};

/**
 * Base class for nodes in the AST.
 */
struct ASTNode {
	virtual ~ASTNode() {}
};

/**
 * Declaration node base class.
 */
struct DeclNode: ASTNode {
};

/**
 * Expression node base class.
 */
struct ExprNode: ASTNode {
};




struct TypeExpr: ExprNode {
};




struct ExprSequence: ExprNode {
	std::vector<std::unique_ptr<ExprNode>> expressions;
};

struct ScopeGroupExpr: ExprNode {
	std::unique_ptr<ASTNode> expression;
};




struct FuncDefNode: DeclNode {
	StringSlice name;
	std::vector<StringSlice> parameter_names;
	std::vector<TypeExpr> parameter_types;
	TypeExpr return_type;
};

struct FuncCallNode: ExprNode {
	StringSlice name;
	std::vector<std::unique_ptr<ExprNode>> parameters;
};


#endif // AST_HPP