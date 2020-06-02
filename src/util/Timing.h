#pragma once
#include <string>
#include <thread>
#include <mutex>

using namespace std;

class Timing
{
private:
	unsigned long long fps,last_time,measure;
	bool running;
	thread *t;
	mutex mtx;

public:
	Timing();
	~Timing();
	void log();
	unsigned long long getctime();
	void start();
	void frameCompleted();
	void stop();
	void startMeasure();
	void endMeasure();
};

