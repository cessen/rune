#include <stdexcept>

#include "c_gen.hpp"

#include "ast.hpp"
#include "builtins.hpp"
#include "type.hpp"

class UnreachableException : public std::logic_error
{
public:
	UnreachableException(): std::logic_error("") {}
	virtual char const * what() const
	{
		return "Function not yet implemented.";
	}
};


static void gen_c_decl(const DeclNode* decl, std::ostream& f);


void gen_c_code(const AST& ast, std::ostream& f)
{
	f << "#include <stdint.h>\n";
	f << "#include <stdlib.h>\n\n";

	// Declarations
	for (const auto& decl: ast.root->declarations) {
		gen_c_decl(decl, f);
		f << ";\n";
	}
}


static void gen_c_type(const Type* t, std::ostream& f)
{
	switch (t->type_class()) {
		case TypeClass::Pointer: {
			auto ptr = dynamic_cast<const Pointer_T*>(t);
			gen_c_type(ptr->type, f);
			f << "*";
			break;
		}
		case TypeClass::Void: {
			f << "void";
			break;
		}

		case TypeClass::Atom_Byte: {
			f << "uint8_t";
			break;
		}

		case TypeClass::Atom_Int8: {
			f << "int8_t";
			break;
		}

		case TypeClass::Atom_Int16: {
			f << "int16_t";
			break;
		}

		case TypeClass::Atom_Int32: {
			f << "int32_t";
			break;
		}

		case TypeClass::Atom_Int64: {
			f << "int64_t";
			break;
		}

		case TypeClass::Atom_UInt8: {
			f << "uint8_t";
			break;
		}

		case TypeClass::Atom_UInt16: {
			f << "uint16_t";
			break;
		}

		case TypeClass::Atom_UInt32: {
			f << "uint32_t";
			break;
		}

		case TypeClass::Atom_UInt64: {
			f << "uint64_t";
			break;
		}

		case TypeClass::Atom_Float16: {
			// TODO: we don't have an f16 type implementation
			f << "uint16_t";
			break;
		}

		case TypeClass::Atom_Float32: {
			f << "float";
			break;
		}

		case TypeClass::Atom_Float64: {
			f << "double";
			break;
		}

		case TypeClass::Atom_CodePoint: {
			f << "uint32_t";
			break;
		}

		default:
			throw UnreachableException();
	}
}

static void gen_c_literal(const LiteralNode* literal, std::ostream& f)
{
	if (const IntegerLiteralNode* node = dynamic_cast<const IntegerLiteralNode*>(literal)) {
		f << node->text;
	}
	else {
		throw UnreachableException();
	}
}

static void gen_c_expression(const ExprNode* expression, std::ostream& f)
{
	if (const LiteralNode* node = dynamic_cast<const LiteralNode*>(expression)) {
		gen_c_literal(node, f);
	}
	else if (const DerefNode* node = dynamic_cast<const DerefNode*>(expression)) {
		f << "*";
		gen_c_expression(node->expr, f);
	}
	else if (const AddressOfNode* node = dynamic_cast<const AddressOfNode*>(expression)) {
		f << "&";
		gen_c_expression(node->expr, f);
	}
	else if (const VariableNode* node = dynamic_cast<const VariableNode*>(expression)) {
		f << node->declaration->name;
	}
	else if (const ConstantNode* node = dynamic_cast<const ConstantNode*>(expression)) {
		f << node->declaration->name;
	}
	else if (const AssignmentNode* node = dynamic_cast<const AssignmentNode*>(expression)) {
		gen_c_expression(node->lhs, f);
		f << " = ";
		gen_c_expression(node->rhs, f);
	}
	else if (const FuncCallNode* node = dynamic_cast<const FuncCallNode*>(expression)) {
		if (node->name == "+" && node->parameters.size() == 2) {
			f << "(";
			gen_c_expression(node->parameters[0], f);
			f << " + ";
			gen_c_expression(node->parameters[1], f);
			f << ")";
		}
		else if (GetBuiltin(node->name) != nullptr) {
			if (node->name == "cmalloc") {
				f << "malloc(";
				gen_c_expression(node->parameters[0], f);
				f << ")";
			}
		}
		else {
			f << node->name << "(";
			bool first = true;
			for (auto s : node->parameters) {
				if (first)
					first = false;
				else
					f << ", ";
				gen_c_expression(s, f);
			}
			f << ")";
		}
	}
	else {
		throw UnreachableException();
	}
}

static void gen_c_statement(const StatementNode* statement, std::ostream& f)
{
	if (const ReturnNode* node = dynamic_cast<const ReturnNode*>(statement)) {
		f << "return ";
		gen_c_expression(node->expression, f);
	}
	else if (auto node = dynamic_cast<const DeclNode*>(statement)) {
		gen_c_decl(node, f);
	}
	else if (auto node = dynamic_cast<const ExprNode*>(statement)) {
		gen_c_expression(node, f);
	}
	else {
		throw UnreachableException();
	}

	f<< ";\n";
}

static void gen_c_decl(const DeclNode* decl, std::ostream& f)
{
	// Function
	if (dynamic_cast<const FuncLiteralNode*>(decl->initializer)) {
		const auto fn = dynamic_cast<const FuncLiteralNode*>(decl->initializer);

		// Constant
		if (dynamic_cast<const ConstantDeclNode*>(decl)) {

			// Return Type
			gen_c_type(fn->return_type, f);
			f << " ";

			// Name
			f << decl->name << " ";

			// Parameters
			f << "(";
			bool first = true;
			for (const auto& p: fn->parameters) {
				if (!first) {
					f << ", ";
				}
				else {
					first = false;
				}
				// Type
				gen_c_type(p->type, f);
				// Name
				f << " " << p->name;
			}
			f << ")";

			// Body
			f << " {\n";
			for (auto& s: fn->body->statements)
				gen_c_statement(s, f);
			f << "}";
		}
		// Variable
		else {
		}
	}
	else if (const VariableDeclNode* node = dynamic_cast<const VariableDeclNode*>(decl)) {
		gen_c_type(node->type, f);
		f << " " << node->name;
		if (!dynamic_cast<const EmptyExprNode*>(node->initializer)) {
			f << " = ";
			gen_c_expression(node->initializer, f);
		}
	}
	else if (const ConstantDeclNode* node = dynamic_cast<const ConstantDeclNode*>(decl)) {
		f << "const ";
		gen_c_type(node->type, f);
		f << " " << node->name;
		f << " = ";
		gen_c_expression(node->initializer, f);
	}
	else {
		throw UnreachableException();
	}
}
