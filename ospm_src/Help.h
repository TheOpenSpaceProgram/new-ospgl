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
		std::cout << rang::fgB::green << title << rang::fg::reset << ":" << std::endl;
	}

	static void show_menu() 
	{
		std::cout << "OSPM - Open Space Program Manager" << std::endl;
		std::cout << "Version: " << VERSION_STR << std::endl;
		std::cout << std::endl;

		show_title("Global flags");

		std::cout << rang::fgB::blue << "-norefresh" << rang::fg::reset << " : Will not refresh the connected repository this session" << std::endl;
		std::cout << rang::fgB::blue << "-res path/to/res" << rang::fg::reset << " : Resource directory to use. OSPM metadata and packages are stored there. Default is ./res" << std::endl;

		std::cout << std::endl;

		show_title("Connect to a repository");
		std::cout << "Once you are connected to a repository, OSPM will automatically refresh it on every invocation, unless \
it's called with the -norefresh flag" << std::endl;
		std::cout << std::endl;
		show_command("connect", "url-to-ospm-repository [-forget]",
			"Connects OSPM to a repository. OSPM will automatically remember it unless the forget flag is set");

		std::cout << std::endl;

		show_title("Repository operations");
		std::cout << std::endl;
		show_command("search", "[-name x] [-osp_version x] [-tags x,y,z] [-author x] [-folder x]",
			"Searches for packages which satisfy the search query");
		show_command("install", "pkg-id [version] [-overwrite ([prompt]/yes/no)] [-nodeps]",
			"Installs a package given its id and an optional version. If the package is already present, overwrite behaviour \
is controlled by the overwrite setting. If you install a specific version, update will ignore that package until it's manually updated");
		show_command("remove", "pkg-id [-deps]",
			"Removes a package given its id, it will optionally remove all dependencies which are not required by other packages");
		show_command("update", "[pkg-id]",
			"Updates all updateable packages, or optionally only the given package. Packages installed with a specific version will not be updated \
unless you manually indicate their pkg-id");
		show_command("noupdate", "[pkg-id]", 
			"Disables updating for a package, locking it on its current version. To re-enable updating, manually update the package");

		std::cout << std::endl;

		show_title("Direct installation from URLs");
		std::cout << "These functions will not check any meta-data before downloading the whole package!" << std::endl;
		std::cout << rang::fg::reset;
		std::cout << std::endl;

		show_command("fetch", "url-of-pkg-zip [-overwrite ([prompt]/yes/no)]",
			"Installs a package directly from URL. Used internally by install, but can be used to install off-repo packages");


		std::cout << std::endl;
		std::cout << "Press any key to close the program" << std::endl;
		std::cin.get();
	}
};
