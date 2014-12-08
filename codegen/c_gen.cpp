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
	}

	f << "int main(int argc, char** argv) { return 0; }\n";
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

			f << "}\n\n";
		}
		// Variable
		else {
		}
	}
}