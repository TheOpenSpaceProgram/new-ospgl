#include "Timing.h"
#include "Logger.h"
#include <sys/time.h>
#include <unistd.h>

Timing *helper;

void starter()
{
	helper->start();
}


Timing::Timing()
{
	fps=0;
	running=true;
	helper=this;
	t=new thread(starter);
	t->detach();
}

unsigned long long Timing::getctime() {
	struct timeval time;
	gettimeofday(&time, NULL);
	return (time.tv_sec * 1000 + time.tv_usec / 1000.0);
}

void Timing::start()
{
	last_time=getctime();
	while(running)
	{
		usleep(980000);
		log();
	}
}

void Timing::frameCompleted()
{
	auto mtx_lock = std::unique_lock<std::mutex>(mtx);
	fps++;
}

void Timing::log()
{
	unsigned long long ctime;
	int current_fps;

	ctime=getctime();
	mtx.lock();
	current_fps=fps*1000/(ctime-last_time);
	fps=0;
	mtx.unlock();
	logger->info("fps: {} ", current_fps);
	last_time=ctime;
}

void Timing::startMeasure()
{
	measure=getctime();
}

void Timing::endMeasure()
{
	logger->info("meauserd: {} ms", measure-getctime());
}

void Timing::stop()
{
	running=false;
}

Timing::~Timing()
{
	stop();
}
