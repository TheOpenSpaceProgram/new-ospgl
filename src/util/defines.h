#pragma once
#include <functional>
#include <vector>
#include <any>

using VectorOfAny = std::vector<std::any>;

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}