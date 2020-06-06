#pragma once 
#include <string>
#include <unordered_map>
#include <vector>
#include "defines.h"

#define ENABLE_PROFILER

#ifdef ENABLE_PROFILER 
	#define PROFILE(name, block) profiler->push(name); block; profiler->pop();
#else 
	#define PROFILE(name, block) block;
#endif

class Profiler
{
private:

	struct RunStats
	{
		uint64_t count;
		double avg, min, max, last;

		RunStats()
		{
			count = 0; avg = 0; min = 99999999.0; max = -1.0; last = 0.0;
		}
	};

	struct VectorHasher
	{
		std::size_t operator()(const std::vector<std::string>& vec) const
		{
			size_t seed = 0;
			for(const std::string& str : vec)
			{
				hash_combine(seed, str);
			}
			return seed;
		}	
	};

	int compiled;
	std::unordered_map<std::vector<std::string>, double, VectorHasher> checkpoints;
	std::unordered_map<std::vector<std::string>, RunStats, VectorHasher> results;

	std::vector<std::string> stack;

public:

	void push(std::string gate);
	void pop();
	void show_results();
	void show_imgui();

};

extern Profiler* profiler;

void create_global_profiler();
void destroy_global_profiler();
