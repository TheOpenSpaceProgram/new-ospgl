#pragma once
#include <string>
#include <tuple>
#include <fmt/format.h>

// Class used for displaying the actual time
// in a human-readable format
// Year is given relative to 2000 (so 1 is 2001)
// Counts start at 1, except for hour minutes and seconds!
struct Date
{
	enum Month
	{
		INVALID,
		JANUARY,
		FEBRUARY,
		MARCH,
		APRIL,
		MAY,
		JUNE,
		JULY,
		AUGUST,
		SEPTEMBER,
		OCTOBER,
		NOVEMBER,
		DECEMBER
	};

	int year;
	int month;
	int day;
	double day_decimal;

	std::string to_string();

	double to_seconds();

	std::tuple<int, int, int> get_hours_minutes_seconds();

	// Seconds are given relative to J2000
	Date(double seconds);
	Date(int year, int month, int day, double day_decimal = 0.0);
};

