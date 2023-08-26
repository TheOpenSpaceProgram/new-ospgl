#include "GameDatabase.h"
#include <util/Logger.h>
#include <assets/AssetManager.h>

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

// Plumbing machines are machines which may exists as plumbing only
// (but they may also be normal machines!)
void GameDatabase::add_plumbing_machine(const std::string& path, const std::string& pkg)
{
	std::string sane_path = sanitize_path(path, pkg);
	logger->debug("[DB] Adding plumbing machine with path '{}'", sane_path);
	plumbing_machines.push_back(sane_path);
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
	logger->debug("[DB] Adding physical material with path '{}'", sane_path);
	materials.push_back(sane_path);
}

GameDatabase::GameDatabase()
{
}

GameDatabase::~GameDatabase()
{

}

// TODO: This could be optimized! We concatenate strings on every call!
// Ideally, mod developers should only use get_string once during the mod load and store all their strings
// in lua variables, which is the preferred method
const std::string& GameDatabase::get_string(const std::string &id, const std::string &def_pkg)
{
	auto[pkg, name] = osp->assets->get_package_and_name(id, def_pkg);
	std::string final_id = pkg + ":" + name;

	auto it = current_locale.find(final_id);
	if(it != current_locale.end())
	{
		return it->second;
	}
	else
	{
		logger->error("Could not find localised string '{}'", final_id);
		// We return a default string to avoid crashing the game over missing text
		const static std::string UNLOCALISED = "NOT_LOCALE_STR";
		return UNLOCALISED;
	}
}

void GameDatabase::load_locale(const sol::table &locale, const std::string &in_pkg)
{
	// We seek the current locale loaded by OSP, if it's present, we load it, otherwise we load the default
	// and report a warning
	// (NOTE THAT LUA ARRAYS START AT 1 IN THE CODE BELOW!)
	std::string osp_locale = osp->current_locale;

	int wanted_locale = -1;
	sol::table locales = locale[1].get<sol::table>();
	for(size_t i = 1; i <= locales.size(); i++)
	{
		if(locales[i].get<std::string>() == osp_locale)
		{
			wanted_locale = (int)i;
			break;
		}
	}

	if(wanted_locale == -1)
	{
		logger->warn("Could not find locale '{}', using default", osp_locale);
		wanted_locale = 0;
	}

	for(size_t i = 2; i <= locale.size(); i++)
	{
		std::string toml_id = locale[i].get<sol::table>()[1].get<std::string>();
		auto[pkg, name] = osp->assets->get_package_and_name(toml_id, in_pkg);
		std::string id = pkg + ":" + name;
		current_locale[id] = locale[i][wanted_locale + 1].get<std::string>();
	}

}

void GameDatabase::add_reaction(const std::string &path, const std::string &pkg)
{
	std::string sane_path = sanitize_path(path, pkg);
	logger->debug("[DB] Adding chemical reaction with path '{}'", sane_path);
	reactions.push_back(sane_path);
}

void GameDatabase::finish_loading()
{
	// Build the reaction map
	for(const std::string& react : reactions)
	{
		ChemicalReaction reaction;
		std::string reaction_path = osp->assets->resolve_path(react);
		SerializeUtil::read_file_to(reaction_path, reaction);

		// Find the reactants / products (remember equilibria!)
		for(StechiometricMaterial& mat : reaction.reactants)
		{
			material_to_reactions.insert({mat.reactant, reaction});
		}
	}

}

void GameDatabase::add_editor_script(const std::string &path, const std::string &pkg)
{
	std::string sane_path = sanitize_path(path, pkg);
	logger->debug("[DB] Adding editor script with path '{}'", sane_path);
	editor_scripts.push_back(sane_path);
}

void GameDatabase::add_symmetry_mode(const std::string &path, const std::string &pkg)
{
	std::string sane_path = sanitize_path(path, pkg);
	logger->debug("[DB] Adding symmetry mode with path '{}'", sane_path);
	symmetry_modes.push_back(sane_path);
}

void GameDatabase::add_logical_group(const std::string &id, const std::string &display_string)
{
	logical_groups.emplace_back(id, display_string);
}

