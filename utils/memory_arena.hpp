#ifndef MEMORY_ARENA_HPP
#define MEMORY_ARENA_HPP

#include <cstdlib>
#include <utility>
#include <vector>


/**
 * A memory arena.
 */
template <size_t MIN_CHUNK_SIZE=4096>
class MemoryArena {
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


public:
    MemoryArena() {
        add_chunk(0);
    }
    ~MemoryArena() {
        clear_chunks();
    }
    
    
    // No copying, only moving
    MemoryArena(const MemoryArena& other) = delete;
    MemoryArena(MemoryArena&& other) {
        chunks = std::move(other.chunks);
    }

    
    /**
     * Allocates enough contiguous space for count items of type T, and
     * returns a pointer to the front of that space.
     */
    template <typename T>
    T* alloc(size_t count=1) {
        const auto needed_bytes = sizeof(T) * count;
        const auto available_bytes = chunks.back().size - chunks.back().used;
        
        if (needed_bytes > available_bytes) {
            const size_t new_chunk_size = needed_bytes > MIN_CHUNK_SIZE ? needed_bytes : MIN_CHUNK_SIZE;
            add_chunk(new_chunk_size);
        }
        
        T* ptr = reinterpret_cast<T*>(chunks.back().data + chunks.back().used);
        chunks.back().used += needed_bytes;
        return ptr;
    }
};

#endif // MEMORY_ARENA_HPP