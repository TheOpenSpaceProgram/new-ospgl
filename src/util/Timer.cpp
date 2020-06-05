#include "Timer.h"
#include "GLFW/glfw3.h"
#include "Logger.h"
#include <cfloat>

#ifdef DETAILED_TIMING

void Timer::checkpoint(std::string gate)
{
	checkpoints[gate]=glfwGetTime();
}

void Timer::reset_result(struct runstats &r)
{
	r.avg=0.0;
	r.count=0;
	r.max=0.0;
	r.min=DBL_MAX;
}

void Timer::init_result(std::string checkpoint_start,std::string checkpoint_end,std::string name,int level)
{
	struct runstats r;

	r.level=level;
	r.checkpoint_start=checkpoint_start;
	r.checkpoint_end=checkpoint_end;
	r.name=name;
	reset_result(r);
	results.push_back(r);
}

void Timer::init_results()
{
	compiled=0;
	init_result("start","update","start_frame",0);
	init_result("update","render","update",0);
	init_result("render","finish","render",0);
	init_result("render","render_gui","render_main",1);
	init_result("render_gui","render_finish","render_gui",1);
	init_result("render_finish","finish","render_finish",1);
	init_result("finish","end","finish",0);
}

void Timer::compile_results()
{
	std::vector<struct runstats>::iterator it;
	double d;
	struct runstats r;

	for(it=results.begin();it!=results.end();++it)
	{
		r=*it;
		d=checkpoints[it->checkpoint_end]-checkpoints[it->checkpoint_start];
		r.count=it->count+1;
		r.avg=(it->avg*it->count+d)/r.count;
		if (r.max<d) r.max=d;
		if (r.min>d) r.min=d;
		*it=r;
	}
	if (compiled>SHOW_EVERY)
	{
		show_results();
		compiled=0;
	}else compiled++;
}

void Timer::show_results()
{
	std::vector<struct runstats>::iterator it;
	std::string s;

	logger->info("---");
	for(it=results.begin();it!=results.end();++it)
	{
		s="";
		if (it->level>0)
		{
			for(int i=1;i<it->level;i++) s+='-';
			s+="> ";
		}
		logger->info("{0}{1}: max: {3:.4f}ms min: {4:.4f}ms avg: {2:.4f}ms", s,it->name,it->avg*1000.0,it->max*1000.0,it->min*1000.0);
		reset_result(*it);
	}
}

#endif


double Timer::get_elapsed_time()
{
	double now = glfwGetTime();
	double diff = now - t0;

	if (!str.empty())
	{
		logger->info("['{}'] {} seconds", str, diff);
	}

	return diff;
}

double Timer::restart()
{
	double now = glfwGetTime();
	double diff = now - t0;
	
	if (!str.empty())
	{
		logger->info("['{}' (Restart)] {} seconds", str, diff);
	}

	t0 = glfwGetTime();
	return diff;
}

Timer::Timer(std::string name)
{
	str = name;
	t0 = glfwGetTime();
}

Timer::Timer()
{
	str = "";
	t0 = glfwGetTime();
#ifdef DETAILED_TIMING
	init_results();
#endif
}


Timer::~Timer()
{
}
