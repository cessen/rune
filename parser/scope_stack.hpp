#ifndef SCOPE_STACK_HPP
#define SCOPE_STACK_HPP

#include <unordered_map>
#include <vector>

#include "string_slice.hpp"
#include "ast.hpp"


template <typename T>
class ScopeStack
{
	std::unordered_map<StringSlice, T> symbol_table;
	std::vector<std::vector<StringSlice>> symbol_stack;

public:
	ScopeStack()
	{
		push_scope();
	}


	void clear()
	{
		symbol_table.clear();
		symbol_stack.clear();
		push_scope();
	}


	void push_scope()
	{
		symbol_stack.push_back(std::vector<StringSlice>());
	}


	void pop_scope()
	{
		for (auto &name: symbol_stack.back()) {
			symbol_table.erase(name);
		}
		symbol_stack.pop_back();
	}


	bool push_symbol(StringSlice name, T node)
	{
		if (symbol_table.count(name) == 0) {
			symbol_table.emplace(std::make_pair(name, node));
			symbol_stack.back().push_back(name);
			return true;
		}
		else {
			return false;
		}
	}


	bool is_symbol_in_scope(StringSlice name)
	{
		return symbol_table.count(name) > 0;
	}

	T operator[](StringSlice name)
	{
		return symbol_table[name];
	}
};

#endif // SCOPE_STACK_HPP