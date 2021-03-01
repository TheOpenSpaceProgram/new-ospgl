#include "Logger.h"
#include <rang.hpp>
#include <iostream>
#include <fstream>
#include <ctime> 
#include <sstream>
#include <iomanip>
#include <string>
#include <chrono>


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
