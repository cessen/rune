#ifndef MEMORY_ARENA_HPP
#define MEMORY_ARENA_HPP

#include <cstdint>
#include <cstdlib>
#include <utility>
#include <vector>
#include "slice.hpp"

#ifdef _MSC_VER
#define alignof(T) __alignof(T)
#define alignas(T) __declspec(align(T))
#endif

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
	T* _alloc(size_t count) {
		// Figure out how much padding we need between elements for proper
		// memory alignment if we put them in an array.
		const auto array_pad = (alignof(T) - (sizeof(T) % alignof(T))) % alignof(T);

		// Total needed bytes for the requested array of data
		const auto needed_bytes = (sizeof(T) * count) + (array_pad * (count - 1));

		// Figure out how much padding we need at the beginning to put the
		// first element in the right place for memory alignment.
		const auto mem_addr = reinterpret_cast<uintptr_t>(chunks.back().data + chunks.back().used);
		const auto begin_pad = (alignof(T) - (mem_addr % alignof(T))) % alignof(T);

		// Get the number of bytes left in the current chunk
		const auto available_bytes = chunks.back().size - chunks.back().used;

		// If we don't have enough space in this chunk, then we need to create
		// a new chunk.
		if ((begin_pad + needed_bytes) > available_bytes) {
			// Calculate the minimum needed bytes to guarantee that we can
			// accommodate properlyaligned data.
			const auto min_needed_bytes = needed_bytes + alignof(T);

			// Make sure we don't get a chunk smaller than MIN_CHUNK_SIZE
			const size_t new_chunk_size = min_needed_bytes > MIN_CHUNK_SIZE ? min_needed_bytes : MIN_CHUNK_SIZE;

			// Create the new chunk, and then recurse for DRY purposes.
			// TODO: if we break things up differently, we could perhaps
			// avoid the redundant work caused by recursing while still
			// being DRY.  Super low priority, though, unless this somehow
			// turns out to be a performance bottleneck.
			add_chunk(new_chunk_size);
			return _alloc<T>(count);
		}

		// Otherwise, proceed in getting the pointer and recording how much
		// of the chunk we used.
		T* ptr = reinterpret_cast<T*>(chunks.back().data + chunks.back().used + begin_pad);
		for (unsigned int i=0; i < count; ++i) {
			new(ptr+i) T();
		}
		chunks.back().used += begin_pad + needed_bytes;

		// Ta da!
		return ptr;
	}


public:
	MemoryArena() {
		// Start with a single chunk of size zero,
		// to simplify the logic in _alloc()
		add_chunk(0);
	}
	~MemoryArena() {
		clear_chunks();
	}


	// No copying, only moving, but...
	// HACK: MSVC is stupid, so we're making
	// the copy-constructors behave like
	// move constructors.
#ifdef _MSC_VER
	MemoryArena(MemoryArena& other) {
		chunks = std::move(other.chunks);
	}
	MemoryArena& operator=(MemoryArena& other) {
		chunks = std::move(other.chunks);
		return *this;
	}
#else
	MemoryArena(MemoryArena& other) = delete;
	MemoryArena& operator=(MemoryArena& other) = delete;
#endif
	MemoryArena(MemoryArena&& other) {
		chunks = std::move(other.chunks);
	}
	MemoryArena& operator=(MemoryArena&& other) {
		chunks = std::move(other.chunks);
		return *this;
	}


	/**
	 * Allocates space for a single element of type T and returns a
	 * raw pointer to that space.
	 */
	template <typename T>
	T* alloc() {
		return _alloc<T>(1);
	}


	/**
	 * Allocates space for a single element of type T, initializes it with
	 * init, and returns a raw pointer to that space.
	 */
	template <typename T>
	T* alloc(const T& init) {
		auto ptr = _alloc<T>(1);
		*ptr = init;
		return ptr;
	}


	/**
	 * Allocates enough space for count elements of type T, and returns
	 * a Slice to that space.
	 */
	template <typename T>
	Slice<T> alloc_array(size_t count) {
		if (count <= 0)
			return Slice<T>();

		return Slice<T>(_alloc<T>(count), count);
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

		auto ptr = _alloc<T>(size);

		for (size_t i = 0; i < size; ++i) {
			ptr[i] = *begin;
			++begin;
		}

		return Slice<T>(ptr, size);
	}
};

#endif // MEMORY_ARENA_HPP
