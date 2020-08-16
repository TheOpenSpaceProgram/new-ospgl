#include "GameDatabase.h"
#include <util/Logger.h>


static std::string sanitize_path(const std::string& path, const std::string& pkg)
{
	std::string sane_path = path;
	if(path.find(':') == std::string::npos)
	{
		sane_path.insert(0, pkg);
		sane_path.insert(pkg.size(), ":");
	}
	return sane_path;
}

void GameDatabase::add_part(const std::string& path, const std::string& pkg)
{
	std::string sane_path = sanitize_path(path, pkg);
	logger->debug("Adding part from pkg '{}' with path '{}'", pkg, sane_path);	
	parts.push_back(sane_path);
}

void GameDatabase::add_part_category(const std::string& path, const std::string& pkg)
{
	std::string sane_path = sanitize_path(path, pkg);
	logger->debug("Adding part category from pkg '{}' with path '{}'", pkg, sane_path);	
	part_categories.push_back(sane_path);
}


GameDatabase::GameDatabase()
{
	
}
