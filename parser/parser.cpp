#include "parser.hpp"
#include "string_slice.hpp"
#include "type.hpp"


AST parse_tokens(const char* file_path, const std::vector<Token>& tokens)
{
	Parser parser(file_path, tokens);

	return parser.parse();
}


Parser::Parser(std::string file_path, const std::vector<Token>& tokens): file_path {file_path}, begin {tokens.cbegin()}, end {tokens.cend()}, token_iter {tokens.cbegin()}
{
	// Build operator precidence map
	// Note that this is only for function-like binary operators.
	// Non-function-like operators such as . have their own rules.
	// Unary operators always bind more tightly than binary operators.
	binary_op_list.reserve(256); // Make sure we have enough space to avoid iterator invalidation
	binary_op_list.append(" "); // To get it started, so that add_op_prec()'s logic works

	add_op_prec("*", 100); // Multiply
	add_op_prec("/", 100); // Divide
	add_op_prec("//", 100); // Modulus/remainder

	add_op_prec("+", 90); // Add
	add_op_prec("-", 90); // Subtract

	add_op_prec("<<", 80); // Bit shift left
	add_op_prec(">>", 80); // Bit shift right

	add_op_prec("<", 70); // Less than
	add_op_prec(">", 70); // Greater than
	add_op_prec("<=", 70); // Less than or equal
	add_op_prec(">=", 70); // Greater than or equal

	add_op_prec("==", 60); // Equal
	add_op_prec("!=", 60); // Not equal

	add_op_prec("&", 50); // Bit-wise and

	add_op_prec("^", 40); // Bit-wise xor

	add_op_prec("|", 30); // Bit-wise or

	add_op_prec("and", 20); // Logical and

	add_op_prec("or", 10); // Logical or

	add_op_prec("=", -10); // Assignment
}


AST Parser::parse()
{
	ast.root = ast.store.alloc<NamespaceNode>();
	ast.root->code = *begin;
	ast.root->code.text.set_end(end->text.end());

	std::vector<NamespaceNode*> namespaces;
	std::vector<DeclNode*> declarations;

	// Iterate over the tokens and collect all top-level
	// declarations and namespaces
	while (token_iter < end) {
		skip_docstrings_and_newlines();

		// Call the appropriate parsing function for the token type
		switch (token_iter->type) {
			// Declarations
			case K_CONST:
			case K_VAL:
			case K_VAR:
			case K_FN:
			case K_STRUCT:
			case K_TYPE: {
				declarations.push_back(parse_declaration());
				break;
			}

			case K_NAMESPACE:
				// TODO
				parsing_error(*token_iter, "TODO: namespaces not yet implemented.");
				break;

			case LEX_EOF:
				goto done;

			// Something else, not allowed at this level
			default: {
				// Error
				parsing_error(*token_iter, "Only declarations are allowed at the namespace level");
			}
		}

	}

done:

	// Move lists of declarations and namespaces into root
	ast.root->namespaces = ast.store.alloc_from_iters(namespaces.begin(), namespaces.end());
	ast.root->declarations = ast.store.alloc_from_iters(declarations.begin(), declarations.end());

	// Return the AST
	return std::move(ast);
}
