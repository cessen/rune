#include "catch.hpp"

#include <cstdint>
#include <vector>
#include <list>
#include "slice.hpp"

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


// Make sure alloc() initializes things properly if given a value
TEST_CASE("alloc() init", "[memory_arena]")
{
	MemoryArena<> arena;

	int32_t* a = arena.alloc<int32_t>(42);
	int32_t* b = arena.alloc<int32_t>(64);

	REQUIRE((*a) == 42);
	REQUIRE((*b) == 64);
}


// Make sure alloc_array() creates a slice of the appropriate length
TEST_CASE("alloc_from_array() length", "[memory_arena]")
{
	MemoryArena<> arena;

	Slice<int32_t> s = arena.alloc_array<int32_t>(123);

	REQUIRE(s.size() == 123);
}


// Make sure alloc_from_iters() initializes things properly
TEST_CASE("alloc_from_iters() init", "[memory_arena]")
{
	MemoryArena<64> arena;

	std::vector<int32_t> v {1, 0, 2, 9, 3, 8, 4, 7, 5, 6};
	std::list<int32_t> l {1, 0, 2, 9, 3, 8, 4, 7, 5, 6};

	Slice<int32_t> s1 = arena.alloc_from_iters(v.begin(), v.end());
	Slice<int32_t> s2 = arena.alloc_from_iters(l.begin(), l.end());

	REQUIRE(v.size() == s1.size());
	REQUIRE(l.size() == s2.size());

	auto i1 = v.begin();
	auto i2 = l.begin();
	auto i3 = s1.begin();
	auto i4 = s2.begin();

	for (size_t i = 0; i < s1.size(); ++i) {
		REQUIRE((*i1) == (*i3));
		REQUIRE((*i2) == (*i4));

		++i1;
		++i2;
		++i3;
		++i4;
	}
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