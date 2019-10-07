#pragma once
#include <fmt/core.h>
#include <vector>


class Logger
{
private:

	int flushCounter;
	std::vector<std::string> toFile;

	int wantedFlushCounter;

public:

	template <typename... Args>
	void debug(const char* format, const Args & ... args)
	{
#ifdef _DEBUG
		log(0, format, fmt::make_format_args(args...));
#endif
	}

	template <typename... Args>
	void info(const char*  format, const Args & ... args)
	{
		log(1, format, fmt::make_format_args(args...));
	}

	template <typename... Args>
	void warn(const char*  format, const Args & ... args)
	{
		log(2, format, fmt::make_format_args(args...));
	}

	template <typename... Args>
	void error(const char*  format, const Args & ... args)
	{
		log(3, format, fmt::make_format_args(args...));
	}

	template <typename... Args>
	void fatal(const char*  format, const Args & ... args)
	{
		log(4, format, fmt::make_format_args(args...));
	}

	void log(int level, const char*, fmt::format_args args);

	// Condition MUST be true
	void check(bool condition, const char* text = "", bool fatal = true);

	void onLog();

	Logger();
	~Logger();
};

extern Logger* logger;

void createGlobalLogger();
void destroyGlobalLogger();
