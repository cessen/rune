#ifndef STRING_SLICE_HPP
#define STRING_SLICE_HPP


#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

/**
 * A non-owning view into part of a std::string.
 */
struct StringSlice {
	char const * iter = nullptr; // Iterator to the beginning of the string slice
	char const * iter_end = nullptr; // Iterator to the end of the string slice


	// Constructors
	StringSlice()
	{}

	StringSlice(char const * string_lit) : iter { string_lit }, iter_end { string_lit + std::strlen(string_lit) } {}

	StringSlice(char const * iter, char const * iter_end): iter {iter}, iter_end {iter_end}
	{}

	StringSlice(std::string::const_iterator iter, std::string::const_iterator iter_end): iter {&(*iter)}, iter_end {&(*iter_end)}
	{}

	char const* begin() const
	{
		return iter;
	}

	char const* end() const
	{
		return iter_end;
	}

	// Methods for setting the begin/end of the string slice
	void set_begin(char const * begin)
	{
		iter = begin;
	}

	void set_begin(std::string::const_iterator begin)
	{
		iter = &(*begin);
	}

	void set_end(char const * end)
	{
		iter_end = end;
	}

	void set_end(std::string::const_iterator end)
	{
		iter_end = &(*end);
	}


	// Returns the length of the string slice
	size_t length() const
	{
		return std::distance(iter, iter_end);
	}


	// Creates and returns a std::string from the string slice
	std::string to_string() const
	{
		if (iter != iter_end)
			return std::string(iter, iter_end);
		else
			return std::string("");
	}


	// Comparing string slices to various string representations
	bool operator==(const StringSlice &other) const
	{
		const auto len = length();
		const auto other_len = other.length();
		if (len != other_len)
			return false;

		for (unsigned int i = 0; i < len; ++i) {
			if (iter[i] != other.iter[i])
				return false;
		}

		return true;
	}

	bool operator!=(const StringSlice &other) const
	{
		return !(*this == other);
	}


	bool operator==(const std::string &other) const
	{
		const auto len = length();
		const auto other_len = other.length();
		if (len != other_len)
			return false;

		auto other_iter = other.begin();
		for (unsigned int i = 0; i < len; ++i) {
			if (iter[i] != other_iter[i])
				return false;
		}

		return true;
	}

	bool operator!=(const std::string &other) const
	{
		return !(*this == other);
	}


	bool operator==(const char* const str) const
	{
		const auto len = length();

		unsigned int i = 0;
		for (; i < len; ++i) {
			if (iter[i] != str[i] || str[i] == '\0')
				return false;
		}

		if (str[i] != '\0')
			return false;

		return true;
	}

	bool operator!=(const char* const str) const
	{
		return !(*this == str);
	}
};


// Allows StringSlice to be used with iostreams
static std::ostream& operator<< (std::ostream& out, const StringSlice& strslc)
{
	const auto length = strslc.length();

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

	result_type operator()(argument_type const& s) const
	{
		// TODO: something more efficient that avoids allocating
		// a std::string.
		return std::hash<std::string>()(s.to_string());
	}
};
}


#endif // STRING_SLICE_HPP
