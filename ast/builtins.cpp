#include <map>
#include <memory>
#include <string>

#include "ast.hpp"
#include "slice.hpp"
#include "builtins.hpp"

std::map<std::string, std::unique_ptr<ConstantDeclNode>> builtins_map;

Void_T void_t;
Byte_T byte;
UInt64_T u64;
Type* u64_t_ptr_array[1];
Type* byte_t_ptr_array[1];
Pointer_T ptr_byte;
Function_T cmalloc_T;
Function_T cfree_T;

void InitBuiltins()
{
	ptr_byte.type = &byte;
	u64_t_ptr_array[0] = &u64;
	cmalloc_T.return_t = &ptr_byte;
	cmalloc_T.parameter_ts = Slice<Type*>(u64_t_ptr_array, 1);

	byte_t_ptr_array[0] = &byte;
	cfree_T.return_t = &void_t;
	cfree_T.parameter_ts = Slice<Type*>(byte_t_ptr_array, 1);

	auto node = new ConstantDeclNode();
	node->name = "cmalloc";
	node->type = &cmalloc_T;
	builtins_map.insert(std::make_pair("cmalloc", std::unique_ptr<ConstantDeclNode>(node)));

	node = new ConstantDeclNode();
	node->name = "cfree";
	node->type = &cfree_T;
	builtins_map.insert(std::make_pair("cfree", std::unique_ptr<ConstantDeclNode>(node)));
}

ConstantDeclNode* GetBuiltin(StringSlice name)
{
	auto it = builtins_map.find(name.to_string());

	if (it == builtins_map.end())
		return nullptr;

	return it->second.get();
}