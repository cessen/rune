#ifndef SLICE_HPP
#define SLICE_HPP

#include <cstdlib>
#include <cassert>

/**
 * Basically just a non-owning pointer, but with size information
 * for arrays of data.
 */
template <typename T>
class Slice
{
	T* _data = nullptr;
	size_t _size = 0;

public:
	Slice() {}
	Slice(T* ptr, size_t size): _data {ptr}, _size {size}
	{}

	T& operator*() {
		assert(_data != nullptr);
		return *_data;
	}

	const T& operator*() const {
		assert(_data != nullptr);
		return *_data;
	}

	T& operator[](size_t i) {
		assert(_data != nullptr && _size > i);
		return *(_data+i);
	}

	const T& operator[](size_t i) const {
		assert(_data != nullptr && _size > i);
		return *(_data+i);
	}

	size_t size() const {
		return _size;
	}

	T* begin() {
		return _data;
	}

	T* end() {
		return _data + _size;
	}

	T const * begin() const {
		return _data;
	}

	T const * end() const {
		return _data + _size;
	}
};

#endif // SLICE_HPP