#include "catch.hpp"

#include <cstdint>
#include "memory_arena.hpp"


struct alignas(32) SomeType {
    int64_t x;
    int64_t y;
    int64_t z;
};


// Make sure that subsequent allocations that should fit into a single
// chunk are actually allocated sequentially.
TEST_CASE("Sequential memory addresses", "[memory_arena]")
{
	MemoryArena<16> arena;

	int32_t* a = arena.alloc<int32_t>();
	int32_t* b = arena.alloc<int32_t>();
	int32_t* c = arena.alloc<int32_t>();
	int32_t* d = arena.alloc<int32_t>();

	REQUIRE((a+1) == b);
	REQUIRE((a+2) == c);
	REQUIRE((a+3) == d);
}


// Make sure that types are allocated with proper memory alignment
TEST_CASE("Memory alignment requirements", "[memory_arena]")
{
	MemoryArena<128> arena;

	arena.alloc<char>();
	uintptr_t a = (uintptr_t)(arena.alloc<SomeType>());
	uintptr_t b = (uintptr_t)(arena.alloc<SomeType>());
	arena.alloc<char>();
	arena.alloc<char>();
	arena.alloc<char>();
	uintptr_t c = (uintptr_t)(arena.alloc<SomeType>());

	REQUIRE((a % alignof(SomeType)) == 0);
	REQUIRE((b % alignof(SomeType)) == 0);
	REQUIRE((c % alignof(SomeType)) == 0);
}