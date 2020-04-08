#pragma once


#define VERSION_STR "0.1"

#define COUT_ERROR std::cout << rang::fgB::red << "Error: " << rang::fg::reset
#define COUT_WARNING std::cout << rang::fgB::yellow << "Warning: " << rang::fg::reset

extern std::string res_folder;

static bool yn_prompt()
{
	std::cout << "(y/n - Default: n) ";
	
	std::string input;
	std::cin >> input;

	bool value = false;

	if (input == "y" || input == "Y")
	{
		value = true;
	}

	return value;
}

