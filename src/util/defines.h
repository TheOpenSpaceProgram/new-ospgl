#pragma once
#include <cmath>
#include <functional>
#include <vector>
#include <any>
#include <string>

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}


static std::vector<std::string> split(const char *str, char c = ' ')
{
	std::vector<std::string> result;

	do
	{
		const char *begin = str;

		while(*str != c && *str)
		{
			str++;
		}

		result.push_back(std::string(begin, str));
    
	} while (0 != *str++);

    return result;
}

template<typename T>
static bool vector_contains(std::vector<T>& vec, T elem)
{
	auto it = std::find(vec.begin(), vec.end(), elem);
	return it != vec.end();
}

// https://stackoverflow.com/a/20583932
// Modified the vector in place!
template<typename T>
static void vector_remove_indices(std::vector<T>& vector, const std::vector<size_t>& to_remove)
{
	auto vector_base = vector.begin();
	size_t down_by = 0;

	for (auto iter = to_remove.cbegin();
		 iter < to_remove.cend();
		 iter++, down_by++)
	{
		size_t next = (iter + 1 == to_remove.cend()
				? vector.size()
				: *(iter + 1));

		std::move(vector_base + *iter + 1,
				  vector_base + next,
				  vector_base + *iter - down_by);
	}
	vector.resize(vector.size() - to_remove.size());
}
