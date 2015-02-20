#ifndef PARSER_HPP
#define PARSER_HPP

#include "builtins.hpp"
#include "tokens.hpp"
#include "ast.hpp"
#include "string_slice.hpp"
#include "scope_stack.hpp"

#include <cassert>
#include <iostream>
#include <exception>
#include <string>
#include <cstring>
#include <sstream>
#include <unordered_map>
#include <vector>




AST parse_tokens(const char* file_path, const std::vector<Token>& tokens);

#define PARSE_ERROR_MESSAGE_MAX_LENGTH 4096

class ParseError: std::exception
{
public:
	Token token;
	char error_message[PARSE_ERROR_MESSAGE_MAX_LENGTH];

	ParseError(Token token): token {token}
	{
		error_message[0] = '\0';
	}

	ParseError(Token token, const std::string& message): token {token}
	{
		init_message(message.data(), message.size());
	}

	ParseError(Token token, const char* message): token {token}
	{
		const auto len = strlen(message);
		init_message(message, len);
	}

// HACK: MSVC 2012/2013 doesn't support `noexcept`
#ifdef _MSC_VER
	virtual const char* what() const
	{
#else
	virtual const char* what() const noexcept
	{
#endif
		return error_message;
	}




private:
	void init_message(const char* message, size_t message_length)
	{
		const size_t len = message_length <= PARSE_ERROR_MESSAGE_MAX_LENGTH ? message_length : PARSE_ERROR_MESSAGE_MAX_LENGTH;

		for (int i = 0; i < len; ++i) {
			error_message[i] = message[i];
		}

		error_message[PARSE_ERROR_MESSAGE_MAX_LENGTH-1] = '\0';
	}
};


class Parser
{
	std::string file_path;

	std::vector<Token>::const_iterator begin;
	std::vector<Token>::const_iterator end;
	std::vector<Token>::const_iterator token_iter;

	ScopeStack scope_stack;

	std::unordered_map<StringSlice, int> op_prec; // Binary operator precidence map
	std::string binary_op_list; // Storage for operator strings, referenced by op_prec

	AST ast;


public:
	Parser(std::string file_path, const std::vector<Token>& tokens);
	AST parse();


private:
	////////////////////////////////////////////////
	// Parser Methods
	//
	// All the parsing methods below should adhere to the following
	// conventions:
	//
	// - When they are called, they assume token_iter is on the first
	//   character for them to consume.
	//
	// - When they return, they leave token_iter on the first character that
	//   they don't consume (as opposed to the last character they do).  In
	//   particular they should not consume trailing whitespace unless it
	//   is actually syntactically meaningful to them.
	//
	// - When calling another parsing method, the call should be done in a
	//   state consistent with the above, and handle things afterwards
	//   assuming a state consistent with the above.
	////////////////////////////////////////////////

	// parser_statements.cpp
	StatementNode* parse_statement();
	ReturnNode* parse_return();

	// parser_scope.cpp
	ScopeNode* parse_scope();

	// parser_expressions.cpp
	ExprNode* parse_expression();
	ExprNode* parse_primary_expression();

	// parser_declarations.cpp
	DeclNode* parse_declaration();
	ConstantDeclNode* parse_constant_decl();
	VariableDeclNode* parse_variable_decl();
	ConstantDeclNode* parse_func_definition();
	NominalTypeDeclNode* parse_nominal_type_decl();

	// parser_calls.cpp
	FuncCallNode* parse_standard_func_call();
	FuncCallNode* parse_unary_func_call();
	ExprNode* parse_binary_func_call(ExprNode* lhs, int lhs_prec);

	// parser_literals.cpp
	LiteralNode* parse_literal();
	FuncLiteralNode* parse_function_literal(bool has_fn = true);
	struct Type* parse_type();
	struct Type* parse_struct();




private:
	////////////////////
	// Helper Methods
	////////////////////

	void skip_docstrings()
	{
		while (token_iter->type == DOC_STRING)
			++token_iter;
	}


	void skip_newlines()
	{
		while (token_iter->type == NEWLINE)
			++token_iter;
	}


	void skip_docstrings_and_newlines()
	{
		while (token_iter->type == DOC_STRING || token_iter->type == NEWLINE)
			++token_iter;
	}


	// Returns whether the token is a function identifier or operator
	bool token_is_const_function(Token t)
	{
		if (t.type == OPERATOR) {
			// TODO
			return true;
		}
		else if (t.type == IDENTIFIER &&
		         scope_stack.is_symbol_in_scope(t.text) &&
		         is_node_const_func_decl(scope_stack[t.text])
		        ) {
			return true;
		}
		else if (t.type == IDENTIFIER && GetBuiltin(t.text) != nullptr) {
			return true;
		}
		else {
			return false;
		}
	}


	// Returns whether the token is a variable identifier
	bool token_is_variable(Token t)
	{
		if (t.type == IDENTIFIER &&
		        scope_stack.is_symbol_in_scope(t.text) &&
		        is_node_variable(scope_stack[t.text]) ||
		        is_node_constant(scope_stack[t.text])
		   ) {
			return true;
		}
		else {
			return false;
		}
	}


	bool token_in_scope(Token t)
	{
		return scope_stack.is_symbol_in_scope(t.text);
	}


	// Throws an error if the given token isn't in scope
	void assert_in_scope(Token t)
	{
		if (!scope_stack.is_symbol_in_scope(t.text) && GetBuiltin(t.text) == nullptr) {
			std::ostringstream msg;
			msg << "No symbol in scope named '" << token_iter->text << "'.";
			parsing_error(*token_iter, msg.str());
		}
	}


	// Returns whether the token is a terminator token, i.e. a token
	// that ends an expression.
	bool token_is_terminator(Token t)
	{
		return (
		           t.type == NEWLINE ||
		           t.type == COMMA ||
		           t.type == RPAREN ||
		           t.type == RSQUARE ||
		           t.type == RCURLY
		       );
	}


	void add_op_prec(const char* op, int prec)
	{
		auto itr = binary_op_list.cend();
		binary_op_list.append(op);
		op_prec.emplace(StringSlice(itr, binary_op_list.cend()), prec);
	}


	int get_op_prec(StringSlice symbol)
	{
		if (op_prec.count(symbol) > 0)
			return op_prec[symbol];
		else
			return 0;
	}


private:
	//////////////////////////////////////////////
	// Error reporting
	//////////////////////////////////////////////

	// Prints an error and then throws it
	void parsing_error(Token t, std::string msg = "")
	{
		std::ostringstream fmt;
		fmt << "\x1b[31;1mParse error:\033[0m \033[1m" << file_path << ":" << t.line + 1 << ":" << t.column << ":\033[0m\n    " << msg;
		std::cout << fmt.str() << "\n\n";
		throw ParseError {t, fmt.str()};
	}
};


#endif // PARSER_HPP
