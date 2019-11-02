#include "Date.h"

std::string Date::to_string()
{
	auto[hour, minute, second] = get_hours_minutes_seconds();

	return fmt::format("{}/{}/{} {}:{}:{}", day, month, year, hour, minute, second);
}

// https://archive.org/details/Practical_Astronomy_with_your_Calculator_or_Spreadsheet_4th_edition_by_Peter_Duf
// Page 9 (Time, 4)
// Algorithm implemented and then adjusted for J2000
double Date::to_seconds()
{

	double y = year;
	double m = month;
	double d = day;

	double yp = y;
	double mp = m;

	if (m == 1 || m == 2)
	{
		yp = y - 1;
		mp = m + 12;
	}

	// Dates ARE ALWAYS LATER THAN 1582, so
	double A = trunc(yp / 100.0);
	double B = 2.0 - A + trunc(A / 4.0);

	// yp won't be negative either, dates are also later than 0
	double C = trunc(365.25 * yp);
	double D = trunc(30.6001 * (mp + 1));

	double JD = B + C + D + d + 1720994.5;

	// Offset so we start at J2000
	JD -= 2451545.0;

	JD += day_decimal;

	return JD * 24.0 * 60.0 * 60.0;
}

std::tuple<int, int, int> Date::get_hours_minutes_seconds()
{
	double day_decimal_part = day_decimal;

	double hour_decimal = day_decimal_part * 24.0;

	int hour = (int)floor(hour_decimal);

	double minute_decimal = (hour_decimal - hour) * 60.0;

	int minute = (int)floor(minute_decimal);

	int second = (int)((minute_decimal - minute) * 60.0);

	return std::make_tuple(hour, minute, second);
}

// https://archive.org/details/Practical_Astronomy_with_your_Calculator_or_Spreadsheet_4th_edition_by_Peter_Duf
// Page 11 (Time, 5)
// Algorithm implemented and then adjusted for J2000
Date::Date(double seconds)
{
	double minutes = seconds / 60;
	double hours = minutes / 60;
	double julian_day = hours / 24.0;

	// Offset so we start at julian 0
	julian_day += 2451545.0;

	// We now implement the algorithm in the reference in C++
	julian_day += 0.5;
	double I = floor(julian_day);
	double F = julian_day - I;
	
	double B = I;

	if (I > 2299160)
	{
		double A = trunc((I - 1867216.25) / 36524.25);
		B = I + A - trunc(A / 4.0) + 1;
	}

	double C = B + 1524;
	double D = trunc((C - 122.1) / 365.25);
	double E = trunc(365.25 * D);
	double G = trunc((C - E) / 30.6001);

	day_decimal = C - E + F - trunc(30.6001 * G);

	double m;

	if (G < 13.5)
	{
		m = G - 1;
	}
	else
	{
		m = G - 13;
	}

	month = (int)floor(m);

	if (m > 2.5)
	{
		year = (int)floor(D - 4716);
	}
	else
	{
		year = (int)floor(D - 4715);
	}
	
	day = (int)floor(day_decimal);

	day_decimal -= day;

}

Date::Date(int year, int month, int day, double day_decimal)
{
	this->year = year;
	this->month = month;
	this->day = day;
	this->day_decimal = day_decimal;
}
