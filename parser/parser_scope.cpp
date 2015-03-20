#include "parser.hpp"

// Scope
ScopeNode* Parser::parse_scope()
{
	auto node = ast.store.alloc<ScopeNode>();
	node->code = *token_iter;
	std::vector<StatementNode*> statements;

	// Open scope
	if (token_iter->type != LPAREN) {
		// Error
		std::ostringstream msg;
		msg << "Opening scope with wrong character: '" << token_iter->text << "'.";
		parsing_error(*token_iter, msg.str());
	}
	++token_iter;

	// Push this scope
	fn_scope.push_scope();

	while (true) {
		skip_newlines();

		// Close scope?
		if (token_iter->type == RPAREN) {
			++token_iter;
			break;
		}
		// Should be an expression
		else {
			statements.push_back(parse_statement());
		}
	}

	node->statements = ast.store.alloc_from_iters(statements.begin(), statements.end());

	// Pop this scope
	fn_scope.pop_scope();

	node->code.text.set_end((token_iter - 1)->text.end());
	return node;
}
