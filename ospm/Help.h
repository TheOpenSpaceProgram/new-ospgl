#pragma once

#include <rang.hpp>
#include "define.h"

class Help
{
public:

	static void show_command(std::string cmd, std::string args, std::string help)
	{
		std::cout << "- " << rang::fgB::yellow << cmd << " " << rang::fgB::blue << args << rang::fg::reset << std::endl;
		std::cout << help << std::endl;
	}

	static void show_title(std::string title)
	{
		std::cout << rang::fgB::gray << title << rang::fg::reset << ":" << std::endl;
	}

	static void show_menu() 
	{
		std::cout << "OSPM - Open Space Program Manager" << std::endl;
		std::cout << "Version: " << VERSION_STR << std::endl;
		std::cout << std::endl;

		show_title("Connect to a repository");
		show_command("connect", "url-to-ospm-repository [-forget]",
			"Connects OSPM to a repository. OSPM will automatically remember it unless the forget flag is set");
		
		std::cout << std::endl;

		show_title("Repository operations");
		show_command("search", "[-name x] [-osp_version x] [-tags x,y,z] [-author x] [-folder x]",
			"Searches for packages which satisfy the search query");
		show_command("install", "pkg-id [-overwrite (prompt/yes/no)]",
			"Installs a package given its id. It will also install all dependencies (from the same repository).");
		show_command("remove", "pkg-id",
			"Removes a package given its id");

		std::cout << std::endl;

		show_title("Direct installation from URLs");
		show_command("fetch", "url-of-pkg-zip [-overwrite (prompt/yes/no)]",
			"Same as install but from the direct URL to a package, or the path to it on disk. It will only check dependencies.");


		std::cout << std::endl;
		std::cout << "Press any key to close the program" << std::endl;
		std::cin.get();
	}
};