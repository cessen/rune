#ifndef STRING_SLICE_HPP
#define STRING_SLICE_HPP


#include <iostream>
#include <string>
#include <cstring>

/**
 * A non-owning view into part of a std::string.
 */
struct StringSlice {
	std::string::const_iterator iter {nullptr}; // Iterator to the beginning of the string slice
	std::string::const_iterator end {nullptr}; // Iterator to the end of the string slice

	std::string to_string() const {
		if (iter != end)
			return std::string(iter, end);
		else
			return std::string("");
	}

	bool operator==(const StringSlice &other) const {
		const auto length = std::distance(iter, end);
		const auto other_length = std::distance(other.iter, other.end);
		if (length != other_length)
			return false;

		for (int i = 0; i < length; ++i) {
			if (iter[i] != other.iter[i])
				return false;
		}

		return true;
	}

	bool operator==(const std::string &other) const {
		const auto length = std::distance(iter, end);
		const auto other_length = other.length();
		if (length != other_length)
			return false;

		auto other_iter = other.begin();
		for (int i = 0; i < length; ++i) {
			if (iter[i] != other_iter[i])
				return false;
		}

		return true;
	}

	bool operator==(const char* const str) const {
		const auto length = std::distance(iter, end);

		int i = 0;
		for (; i < length; ++i) {
			if (iter[i] != str[i] || str[i] == '\0')
				return false;
		}

		if (str[i] != '\0')
			return false;

		return true;
	}
};


// Allows StringSlice to be used with iostreams
static std::ostream& operator<< (std::ostream& out, const StringSlice& strslc)
{
	auto length = std::distance(strslc.iter, strslc.end);
	if (length == 0)
		return out;

	for (size_t i = 0; i < length; ++i)
		out << strslc.iter[i];

	return out;
}


#endif // STRING_SLICE_HPP