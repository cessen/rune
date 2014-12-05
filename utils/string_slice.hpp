#ifndef STRING_SLICE_HPP
#define STRING_SLICE_HPP


#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>

/**
 * A non-owning view into part of a std::string.
 */
struct StringSlice {
	std::string::const_iterator iter; // Iterator to the beginning of the string slice
	size_t length = 0; // Iterator to the end of the string slice

	StringSlice()
	{}

	StringSlice(std::string::const_iterator begin, std::string::const_iterator end): iter {begin}, length {static_cast<size_t>(std::distance(begin, end))}
	{}

	// iter must be set before calling this
	void set_end(std::string::const_iterator end) {
		length = std::distance(iter, end);
	}

	std::string to_string() const {
		if (length != 0)
			return std::string(iter, iter+length);
		else
			return std::string("");
	}


	bool operator==(const StringSlice &other) const {
		if (length != other.length)
			return false;

		for (int i = 0; i < length; ++i) {
			if (iter[i] != other.iter[i])
				return false;
		}

		return true;
	}


	bool operator!=(const StringSlice &other) const {
		return !(*this == other);
	}


	bool operator==(const std::string &other) const {
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


	bool operator!=(const std::string &other) const {
		return !(*this == other);
	}


	bool operator==(const char* const str) const {
		int i = 0;
		for (; i < length; ++i) {
			if (iter[i] != str[i] || str[i] == '\0')
				return false;
		}

		if (str[i] != '\0')
			return false;

		return true;
	}


	bool operator!=(const char* const str) const {
		return !(*this == str);
	}
};


// Allows StringSlice to be used with iostreams
static std::ostream& operator<< (std::ostream& out, const StringSlice& strslc)
{
	if (strslc.length == 0)
		return out;

	for (size_t i = 0; i < strslc.length; ++i)
		out << strslc.iter[i];

	return out;
}


// Allows string slice to be hashed, for use in e.g. unordered_map
namespace std
{
template<>
struct hash<StringSlice> {
	typedef StringSlice argument_type;
	typedef std::size_t result_type;

	result_type operator()(argument_type const& s) const {
		// TODO: something more efficient that avoids allocating
		// a std::string.
		return std::hash<std::string>()(s.to_string());
	}
};
}


#endif // STRING_SLICE_HPP