#include "SaveDatabase.h"
#include <assets/AssetManager.h>
#include <OSP.h>

std::shared_ptr<cpptoml::table> SaveDatabase::get_toml(const std::string &pkg, const std::string &path)
{
	std::string sane_path = path;
	if(sane_path.find(".toml") != sane_path.size() - 5)
	{
		sane_path = sane_path + ".toml";
	}

	logger->check(osp->assets->is_path_safe(path), "Path {} is unsafe and breaks the sandbox", path);
	logger->check(osp->assets->is_path_safe(pkg), "Pkg {} is unsafe and breaks the sandbox", pkg);

	// Is the file already loaded
	std::string full_path = form_path(pkg, sane_path);
	auto it = cached.find(full_path);
	if(it != cached.end())
	{
		return it->second;
	}
	else
	{
		// Is the file present?
		std::shared_ptr<cpptoml::table> tb;
		if (osp->assets->file_exists(full_path))
		{
			tb = SerializeUtil::load_file(full_path);
		}
		else
		{
			tb = cpptoml::make_table();
		}

		cached[full_path] = tb;
		return tb;
	}

}

void SaveDatabase::flush_all()
{
	int count = 0;
	for(auto pair : cached)
	{
		SerializeUtil::write_to_file(*pair.second, pair.first);
		count++;
	}
	logger->info("Game database flushed {} files", count);
}

std::string SaveDatabase::form_path(const std::string &pkg, const std::string &path)
{
	return osp->assets->udata_path + save_folder + "/" + pkg + "/" + path;
}
