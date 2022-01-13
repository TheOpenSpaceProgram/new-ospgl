#pragma once 
#include <string>
#include <unordered_map>
#include <vector>
#include "defines.h"

#define ENABLE_PROFILER

// Helper macros to generate unique* identifier
// * Limitation: Only one PROFILER_X per line
// TODO: Use __COUNTER__? It's not standard, but supported on most compilers
#define _PROFILER_MERGE(A, B) A##B
#define _PROFILER_LABEL(A) _PROFILER_MERGE(__profiler_, A)


#ifdef ENABLE_PROFILER 
	#define PROFILE_FUNC() auto _PROFILER_LABEL(__LINE__) = profiler->block(__func__);
#else 
	#define PROFILE_FUNC() block;
#endif

#ifdef ENABLE_PROFILER
	#define PROFILE_BLOCK(name) auto _PROFILER_LABEL(__LINE__) = profiler->block(name)
#else 
	#define PROFILE_BLOCK(name) 
#endif 

struct ProfileBlock;

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

	int pos_x = 0, pos_y = 0;

	ProfileBlock block(std::string name);

	void push(std::string gate);
	void pop();
	void show_results();
	void show_imgui();

};

extern Profiler* profiler;

void create_global_profiler();
void destroy_global_profiler();

// Simple RAII class for profiling blocks of code
class ProfileBlock
{
protected:
	friend class Profiler;

	ProfileBlock()
	{	
		invalid = false;
	}

private:
	bool invalid;

	// Make non-copyable
	ProfileBlock(const ProfileBlock& b) = delete;
	ProfileBlock& operator=(const ProfileBlock& b) = delete;

public:
	
	ProfileBlock(ProfileBlock&& b)
	{
		invalid = b.invalid;
		b.invalid = true;
	}

	~ProfileBlock()
	{
		if(!invalid)
		{
			profiler->pop();
		}
	}
};
