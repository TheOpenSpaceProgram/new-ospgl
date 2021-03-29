#include "Logger.h"
#include <rang.hpp>
#include <iostream>
#include <fstream>
#include <ctime> 
#include <sstream>
#include <iomanip>
#include <string>
#include <chrono>
#include <backward/backward.hpp>

#ifdef _WIN32

#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

size_t get_console_width()
{
#ifdef _WIN32

#else

	struct winsize size;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
	return size.ws_col;

#endif
}

// Pretty prints to the terminal and writes to the file too
void Logger::stacktrace()
{
	using namespace backward;

	size_t con_width = get_console_width();

	StackTrace st; st.load_here();
	TraceResolver tr; tr.load_stacktrace(st);
	std::cout << rang::fg::yellow << "Stacktrace: " << std::endl;
	toFile.push_back("Stacktrace: \n");
	for(size_t i = 5; i < st.size(); i++)
	{
		ResolvedTrace trace = tr.resolve(st[i]);
		std::string file = trace.source.filename;
		auto it = file.find("src");
		if(it != std::string::npos)
		{
			file = file.substr(it);
		}
		else
		{
			it = file.find("dep");
			if(it != std::string::npos)
			{
				file = file.substr(it);
			}
			else
			{
				// External file
				continue;
			}
		}
		std::string path = file + "(" + std::to_string(trace.source.line) + ")";
		std::string fnc = trace.object_function;
		std::string pad = " ";
		if(fnc.size() >= con_width - 64)
		{
			fnc = trace.object_function.substr(0, con_width - 64);
			fnc += "...";
		}
		for(size_t si = path.size(); si < 50; si++)
		{
			pad += " ";
		}
		std::cout << rang::fg::reset << i << "\t" << rang::fg::magenta <<
			path << pad << rang::fg::reset << fnc << std::endl;

		std::string fstring = std::to_string(i);
		fstring += " ";
		fstring += path;
		fstring += " ";
		fstring += trace.object_function;
		fstring += "\n";
		toFile.push_back(fstring);
	}
}

void Logger::log(int level, const char* format, fmt::format_args args)
{
	auto mtx_lock = std::unique_lock<std::mutex>(mtx);

	std::string prefix = "";
	if (level == 0)
	{
		prefix = "DBG";
		std::cout << rang::fgB::black;
	}
	else if (level == 1)
	{
		prefix = "INF";
	}
	else if (level == 2)
	{
		prefix = "WRN";
		std::cout << rang::fg::yellow;
	}
	else if (level == 3)
	{
		prefix = "ERR";
		std::cout << rang::fg::red;
	}
	else
	{
		prefix = "FTL";
		std::cout << rang::fg::red;
	}

	std::string fmated = fmt::vformat(format, args);

	std::string str = fmt::format("[{}] {}\n", prefix, fmated);

	std::cout << str << rang::fg::reset << rang::bg::reset;

	toFile.push_back(str);

	if(level >= 3)
	{
		stacktrace();
	}

	if (level == 4)
	{
		std::cout << "Raising exception" << std::endl;
		flushCounter = 0;
	}

	onLog(level >= 2);

	if (level == 4)
	{
		throw("Fatal error");
	}
}

void Logger::onLog(bool important)
{
	flushCounter--;

	if (flushCounter <= 0 || important)
	{
		std::ofstream outfile;

		outfile.open("output.log", std::ios_base::app);
		for (size_t i = 0; i < toFile.size(); i++)
		{
			outfile << toFile[i];
		}

		toFile.clear();

		flushCounter = wantedFlushCounter;

		outfile.close();
	}
}

Logger::Logger()
{
	wantedFlushCounter = 1000;
	flushCounter = wantedFlushCounter;

	remove("./output.log");

	std::ofstream logfile("output.log");

	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	logfile << "Program started at " << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X") << std::endl;
	logfile << "-------------------------------------------------" << std::endl;

	logfile.close();
}


Logger::~Logger()
{
	flushCounter = 0;
	onLog();
}

Logger* logger;

void create_global_logger()
{
	logger = new Logger();
}

void destroy_global_logger()
{
	delete logger;
}
