#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#define DETAILED_TIMING
#define SHOW_EVERY 60
// Uses GLFW time to make relatively precise 
// measurements
// Inspired by the SFML timer, but just returns seconds.
// If you give a string to the constructor it will automatically
// log (INFO) the time every single call to getElapsedTime or restart
#ifdef DETAILED_TIMING
struct runstats
{
	int level,count;
	std::string name,checkpoint_start,checkpoint_end;
	double avg,min,max;
};
#endif

class Timer
{
private:
	double t0;
	std::string str;
#ifdef DETAILED_TIMING
	int compiled;
	std::unordered_map<std::string,double> checkpoints;
	std::vector<struct runstats> results;
#endif
public:

	double get_elapsed_time();
	double restart();

	Timer(std::string name);
	Timer();
#ifdef DETAILED_TIMING
	void checkpoint(std::string gate);
	void init_results();
	void compile_results();
	void show_results();
	void init_result(std::string checkpoint_start,std::string checkpoint_end,std::string name,int level);
	void reset_result(struct runstats &r);
#endif
	~Timer();
};

