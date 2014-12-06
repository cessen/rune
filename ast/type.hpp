#ifndef TYPE_HPP
#define TYPE_HPP

#include <cstdint>

#include "slice.hpp"
#include "string_slice.hpp"


enum struct TypeClass {
	Void,

	Atom, // One of the fundamental types, like int, float, etc.

	Pointer,
	Slice,

	Array,
	Tuple,
	Struct,
	Enum,
	Union,

	Function,
};


/**
 * Base class for all other types.
 */
struct Type {
	virtual TypeClass type_class() = 0;
	virtual void print(int indent) {
		for (int i = 0; i < indent; ++i) {
			std::cout << "\t";
		}
	};
};


/**
 * Void type.  Used to represent the eval type of statements that don't
 * evaluate to anything and the return type of functions that don't
 * return anything.
 */
struct Void_T: Type {
	virtual TypeClass type_class() {
		return TypeClass::Void;
	}
	virtual void print(int indent) {
		Type::print(indent);
		std::cout << "Void";
	}
};


///////////////////////////////////////////////////////////
// The atom types.
// These are the fundamental building-block types.
///////////////////////////////////////////////////////////

// Base class
struct Atom_T: Type {
	StringSlice value;

	virtual TypeClass type_class() {
		return TypeClass::Atom;
	}
};


// An 8-bit byte
struct Byte_T : Atom_T {
	virtual void print(int indent) {
		Type::print(indent);
		std::cout << "Byte";
	}
};


// An 8-bit integer
struct Int8_T : Atom_T {
	virtual void print(int indent) {
		Type::print(indent);
		std::cout << "Int8";
	}
};

// An 16-bit integer
struct Int16_T : Atom_T {
	virtual void print(int indent) {
		Type::print(indent);
		std::cout << "Int16";
	}
};

// An 32-bit integer
struct Int32_T : Atom_T {
	virtual void print(int indent) {
		Type::print(indent);
		std::cout << "Int32";
	}
};

// An 64-bit integer
struct Int64_T : Atom_T {
	virtual void print(int indent) {
		Type::print(indent);
		std::cout << "Int64";
	}
};


// An unsigned 8-bit integer
struct UInt8_T : Atom_T {
	virtual void print(int indent) {
		Type::print(indent);
		std::cout << "UInt8";
	}
};

// An unsigned 16-bit integer
struct UInt16_T : Atom_T {
	virtual void print(int indent) {
		Type::print(indent);
		std::cout << "UInt16";
	}
};

// An unsigned 32-bit integer
struct UInt32_T: Atom_T {
	virtual void print(int indent) {
		Type::print(indent);
		std::cout << "UInt32";
	}
};

// An unsigned 64-bit integer
struct UInt64_T : Atom_T {
	virtual void print(int indent) {
		Type::print(indent);
		std::cout << "UInt64";
	}
};


// A 16-bit float
struct Float16_T : Atom_T {
	virtual void print(int indent) {
		Type::print(indent);
		std::cout << "Float16";
	}
};

// A 32-bit float
struct Float32_T : Atom_T {
	virtual void print(int indent) {
		Type::print(indent);
		std::cout << "Float32";
	}
};

// A 64-bit float
struct Float64_T : Atom_T {
	virtual void print(int indent) {
		Type::print(indent);
		std::cout << "Float64";
	}
};


// Unicode code point, 32 bits
struct CodePoint_T : Atom_T {
	virtual void print(int indent) {
		Type::print(indent);
		std::cout << "Code Point";
	}
};


///////////////////////////////////////////////////////////
// The pointer types.
// These are the types that point at data stored in
// another memory location.
///////////////////////////////////////////////////////////

// A raw pointer.  Points at a single piece of data in memory.
struct Pointer_T: Type {
	virtual TypeClass type_class() {
		return TypeClass::Pointer;
	}

	Type* t;  // The type that the pointer points at
};


// A raw slice.  Points at an array of data in memory.
struct Slice_T: Type {
	virtual TypeClass type_class() {
		return TypeClass::Slice;
	}

	Type* t;  // The type that the slice points at
};


///////////////////////////////////////////////////////////
// The composite types.
// These are the types that build new types out of other
// types.
///////////////////////////////////////////////////////////

// An array.  Contains a fixed number of elements of the same type.
struct Array_T: Type {
	virtual TypeClass type_class() {
		return TypeClass::Array;
	}

	Type* t;  // The type that the array contains
	size_t size;  // It's size in elements
};


// A tuple.  Contains a fixed number of elements of varying types.
struct Tuple_T: Type {
	virtual TypeClass type_class() {
		return TypeClass::Tuple;
	}

	Slice<Type*> ts;  // The types that the tuple contains
};


// A struct.  A named type that Contains a fixed number of elements of
// varying types in named fields.
struct Struct_T: Type {
	virtual TypeClass type_class() {
		return TypeClass::Struct;
	}

	StringSlice name;
	Slice<Type*> field_ts;  // The types of the fields of the struct
	Slice<StringSlice> field_names;  // The names of the fields of the struct
};


///////////////////////////////////////////////////////////
// The function type.
// This represents the type signature of a function/
///////////////////////////////////////////////////////////

struct Function_T: Type {
	virtual TypeClass type_class() {
		return TypeClass::Function;
	}

	Slice<Type*> parameter_ts;  // The types of the parameters of the function
	Type* return_t;  // The return type of the function
};

#endif // TYPE_HPP