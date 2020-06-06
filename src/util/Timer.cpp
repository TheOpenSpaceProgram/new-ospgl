#include "Timer.h"
#include "GLFW/glfw3.h"
#include "Logger.h"
#include <cfloat>

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
}


Timer::~Timer()
{
}
