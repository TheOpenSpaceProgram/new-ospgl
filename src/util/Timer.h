#pragma once
#include <string>

// Uses GLFW time to make relatively precise 
// measurements
// Inspired by the SFML timer, but just returns seconds.
// If you give a string to the constructor it will automatically
// log (INFO) the time every single call to getElapsedTime or restart
class Timer
{
private:
	double t0;
	std::string str;
public:

	double get_elapsed_time();
	double restart();

	Timer(std::string name);
	Timer();
	~Timer();
};

