#ifndef SLICE_HPP
#define SLICE_HPP

#include <cassert>

/**
 * Basically just a non-owning pointer, but with size information
 * for arrays of data.
 */
template <typename T>
class Slice
{
	T* data = nullptr;
	size_t size = 0;

public:
	Slice() {}
	Slice(T* ptr, size_t size): data {ptr}, size {size}
	{}

	T& operator*() {
		assert(data != nullptr);
		return *data;
	}

	T& operator[](size_t i) {
		assert(data != nullptr && size > i);
		return *(data+i);
	}

	T* begin() {
		return data;
	}

	T* end() {
		return data + size;
	}
};

#endif // SLICE_HPP