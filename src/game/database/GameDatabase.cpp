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
	logger->debug("[DB] Adding part with path '{}'", sane_path);
	parts.push_back(sane_path);
}

void GameDatabase::add_part_category(const std::string& path, const std::string& pkg)
{
	std::string sane_path = sanitize_path(path, pkg);
	logger->debug("[DB] Adding part category with path '{}'", sane_path);
	part_categories.push_back(sane_path);
}

void GameDatabase::add_material(const std::string& path, const std::string& pkg)
{
	std::string sane_path = sanitize_path(path, pkg);
	logger->debug("[DB] Adding material with path '{}'", sane_path);
	PhysicalMaterial pmat;
	SerializeUtil::read_file_to(sane_path, pmat);
	materials[sane_path] = pmat;
}

GameDatabase::GameDatabase()
{
	
}

