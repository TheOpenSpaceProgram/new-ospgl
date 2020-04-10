#pragma once
#include <cmath>
#include <functional>
#include <vector>
#include <any>

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
