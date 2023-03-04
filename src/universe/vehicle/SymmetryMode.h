#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "part/Part.h"

class Vehicle;

// A symmetric distribution handles all symmetric parts in a group
// Behaviour is handled in lua
class SymmetryMode
{
private:
	// Do not use keys starting with "__" as these are used by the engine
	std::shared_ptr<cpptoml::table> save_toml;

public:
	// Metadata, available even without init
	std::string ui_name;
	std::string ui_desc;
	std::string script_path;
	AssetHandle<Image> icon;

	// Only root pieces are included here. Set on creation.
	// To obtain children, use vehicle functions
	// TODO: If this causes lags, store child pieces too and update the array appropiately!
	std::vector<Piece*> symmetry_roots;

	// Called when any of the mirrored pieces is modified
	void on_dirty(Piece* piece);

	// Calld when any of the mirrored, child pieces, is disconnected,
	// so that mirrored versions are properly deleted
	void on_disconnect(Piece* piece);

	// Before init, the symmetry mode is non functional but contains meta-data
	void init(std::shared_ptr<cpptoml::table> init_toml, const std::string& pkg);

};

// This is used ONLY for the prototype, lua stuff is handled by the SymmetryMode itself as usual
template<>
class GenericSerializer<SymmetryMode>
{
public:
	static void serialize(const SymmetryMode& what, cpptoml::table& target)
	{
		logger->fatal("NOT IMPLEMENTED");
	}

	static void deserialize(SymmetryMode& to, const cpptoml::table& from)
	{
		std::string name, desc, icon, script;
		SAFE_TOML_GET(name, "__name", std::string);
		SAFE_TOML_GET(desc, "__description", std::string);
		SAFE_TOML_GET(icon, "__icon", std::string);
		SAFE_TOML_GET(script, "__script", std::string);
		auto pkg = osp->assets->get_current_package();
		to.script_path = osp->assets->get_package_and_name(script, pkg).second;
		to.ui_name = osp->game_database->get_string(name);
		to.ui_desc = osp->game_database->get_string(desc);
		icon = osp->assets->get_package_and_name(icon, pkg).second;
		to.icon = AssetHandle<Image>(icon);
	}
};


