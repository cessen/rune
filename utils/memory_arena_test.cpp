#include "catch.hpp"

#include <cstdint>
#include "memory_arena.hpp"

TEST_CASE("Sequential memory addresses", "[memory_arena]")
{
	// Make sure that subsequent allocations that should fit into a single
	// chunk are actually allocated sequentially.
	MemoryArena<16> arena;

	int32_t* a = arena.alloc<int32_t>();
	int32_t* b = arena.alloc<int32_t>();
	int32_t* c = arena.alloc<int32_t>();
	int32_t* d = arena.alloc<int32_t>();

	REQUIRE((a+1) == b);
	REQUIRE((a+2) == c);
	REQUIRE((a+3) == d);
}