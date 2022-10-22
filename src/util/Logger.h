#pragma once
#include <fmt/core.h>
#include <vector>
#include <mutex>

// Comment to disable "debug" logging in Release
#define LOG_DEBUG_ALWAYS
// Comment to disable "check" calls in Release
#define CHECK_ALWAYS

class Logger
{
private:

	int flushCounter;
	std::vector<std::string> toFile;

	int wantedFlushCounter;

public:

	std::vector<std::pair<std::string, int>> saved_log;

	std::mutex mtx;

	void stacktrace();

	template <typename... Args>
	void debug(const char* format, const Args & ... args)
	{
#if defined(_DEBUG) || defined(LOG_DEBUG_ALWAYS)
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
	template <typename... Args>
	void check(bool condition, const char* text = "", const Args& ... args)
	{
		if (!condition)
		{
			std::string formatted = fmt::format(text, args...);
			std::string str = fmt::format("Condition '{}' failed", formatted);
			error(str.c_str());
			// We throw instead of crashing so lua can handle the checks properly
			// without crashing the program
			throw(str);
		}
	}

	void onLog(bool important = false);

	Logger();
	~Logger();
};

extern Logger* logger;

void create_global_logger();
void destroy_global_logger();
