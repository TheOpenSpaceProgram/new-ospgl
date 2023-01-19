#include "ThreadUtil.h"

#ifdef __linux__
#include <pthread.h>
#endif

#ifdef _WIN32

#endif

void set_this_thread_name(const std::string& str)
{
	std::string sane_str = str;
	if(sane_str.size() >= 15)
	{
		sane_str = sane_str.substr(0, 15);
	}

#ifdef __linux__
	pthread_setname_np(pthread_self(), sane_str.c_str());
#endif

#ifdef _WIN32

#endif

}
