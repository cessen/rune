#ifndef MEMORY_ARENA_HPP
#define MEMORY_ARENA_HPP

#include <cstdlib>
#include <utility>
#include <vector>
#include "slice.hpp"

/**
 * A memory arena.
 */
template <size_t MIN_CHUNK_SIZE=4096>
class MemoryArena
{
	struct Chunk {
		size_t size = 0;
		size_t used = 0;
		char* data = nullptr;
	};

	std::vector<Chunk> chunks;


	void add_chunk(size_t size) {
		Chunk c;
		c.size = size;
		if (size > 0) {
			c.data = new char[size];
		}
		chunks.push_back(c);
	}

	void clear_chunks() {
		for (auto& c: chunks) {
			if (c.data != nullptr)
				delete[] c.data;
		}
		chunks.clear();
	}

	/**
	 * Allocates enough contiguous space for count items of type T, and
	 * returns a pointer to the front of that space.
	 */
	template <typename T>
	T* alloc(size_t count) {
		const auto needed_bytes = sizeof(T) * count;
		const auto available_bytes = chunks.back().size - chunks.back().used;

		if (needed_bytes > available_bytes) {
			const size_t new_chunk_size = needed_bytes > MIN_CHUNK_SIZE ? needed_bytes : MIN_CHUNK_SIZE;
			add_chunk(new_chunk_size);
		}

		T* ptr = reinterpret_cast<T*>(chunks.back().data + chunks.back().used);
		for (int i=0; i < count; ++i) {
			new(ptr+i) T();
		}
		chunks.back().used += needed_bytes;
		return ptr;
	}


public:
	MemoryArena() {
		// Start with a single chunk of size zero,
		// to simplify the logic in alloc()
		add_chunk(0);
	}
	~MemoryArena() {
		clear_chunks();
	}


	// No copying, only moving
	MemoryArena(const MemoryArena& other) = delete;
	MemoryArena& operator=(MemoryArena& other) = delete;
	MemoryArena(MemoryArena&& other) = default;
	MemoryArena& operator=(MemoryArena&& other) = default;


	/**
	 * Allocates a single item of type T.
	 */
	template <typename T>
	T* alloc_single() {
		return alloc(1);
	}


	/**
	 * Allocates space to hold the contents of the iters, and copys the
	 * iter's contents over.  Returns a Slice to that memory.
	 */
	template <typename ITER, typename T=typename ITER::value_type>
	Slice<T> alloc_from_iters(ITER begin, ITER end) {
		const size_t size = std::distance(begin, end);
		if (size <= 0)
			return Slice<T>();

		auto ptr = alloc<T>(size);

		for (size_t i = 0; i < size; ++i) {
			ptr[i] = *begin;
			++begin;
		}

		return Slice<T>(ptr, size);
	}
};

#endif // MEMORY_ARENA_HPP