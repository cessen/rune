#ifndef STRING_SLICE_HPP
#define STRING_SLICE_HPP


#include <iostream>
#include <string>
#include <cstring>

/**
 * A non-owning view into part of a std::string.
 */
struct StringSlice {
	std::string::const_iterator iter; // Iterator to the beginning of the string slice
	std::string::const_iterator end {iter}; // Iterator to the end of the string slice

	StringSlice()
	{}

	StringSlice(std::string::const_iterator begin, std::string::const_iterator end): iter {begin}, end {end}
	{}

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


	bool operator!=(const StringSlice &other) const {
		return !(*this == other);
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


	bool operator!=(const std::string &other) const {
		return !(*this == other);
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


	bool operator!=(const char* const str) const {
		return !(*this == str);
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