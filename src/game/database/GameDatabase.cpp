#include "GameDatabase.h"
#include <util/Logger.h>

void GameDatabase::add_part(const std::string& path, const std::string& pkg)
{
	// Sanitize the path, adding the pkg if neccesary
	std::string sane_path = path;
	if(path.find(':') == std::string::npos)
	{
		sane_path.insert(0, pkg);
		sane_path.insert(pkg.size(), ":");
	}
	logger->debug("Adding part from pkg '{}' with path '{}'", pkg, sane_path);	
	parts.push_back(sane_path);

}

GameDatabase::GameDatabase()
{
	
}
