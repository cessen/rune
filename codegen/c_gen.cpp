#include "c_gen.hpp"

#include "ast.hpp"
#include "type.hpp"


static void gen_c_decl(const DeclNode* decl, std::ostream& f);


void gen_c_code(const AST& ast, std::ostream& f)
{
	f << "#include <stdint.h>\n\n";

	// Declarations
	for (const auto& decl: ast.root->declarations) {
		gen_c_decl(decl, f);
		f << ";\n";
	}
}


static void gen_c_type(const Type* t, std::ostream& f)
{
	switch (t->type_class()) {
		case TypeClass::Void: {
			f << "void";
		}

		case TypeClass::Atom: {
			// Byte
			if (dynamic_cast<const Byte_T*>(t)) {
				f << "uint8_t";
			}
			// Int8
			else if (dynamic_cast<const Int8_T*>(t)) {
				f << "int8_t";
			}
			// Int16
			else if (dynamic_cast<const Int16_T*>(t)) {
				f << "int16_t";
			}
			// Int32
			else if (dynamic_cast<const Int32_T*>(t)) {
				f << "int32_t";
			}
			// Int64
			else if (dynamic_cast<const Int64_T*>(t)) {
				f << "int64_t";
			}
			// UInt8
			else if (dynamic_cast<const UInt8_T*>(t)) {
				f << "uint8_t";
			}
			// UInt16
			else if (dynamic_cast<const UInt16_T*>(t)) {
				f << "uint16_t";
			}
			// UInt32
			else if (dynamic_cast<const UInt32_T*>(t)) {
				f << "uint32_t";
			}
			// UInt64
			else if (dynamic_cast<const UInt64_T*>(t)) {
				f << "uint64_t";
			}
			// Float16
			else if (dynamic_cast<const Float16_T*>(t)) {
				// TODO: we don't have an f16 type implementation
				f << "uint16_t";
			}
			// Float32
			else if (dynamic_cast<const Float32_T*>(t)) {
				f << "float";
			}
			// Float64
			else if (dynamic_cast<const Float64_T*>(t)) {
				f << "double";
			}
			break;
		}

		default:
			break;
	}
}

static void gen_c_literal(const LiteralNode* literal, std::ostream& f)
{
	if (const IntegerLiteralNode* node = dynamic_cast<const IntegerLiteralNode*>(literal)) {
		f << node->text;
	}
}

static void gen_c_expression(const ExprNode* expression, std::ostream& f)
{
	if (const LiteralNode* node = dynamic_cast<const LiteralNode*>(expression)) {
		gen_c_literal(node, f);
	}
	else if (const VariableNode* node = dynamic_cast<const VariableNode*>(expression)) {
		f << node->name;
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
				gen_c_type(p.type, f);
				// Name
				f << " " << p.name;
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
}
