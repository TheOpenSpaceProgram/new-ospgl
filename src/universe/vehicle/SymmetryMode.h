#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "part/Part.h"
#include "gui/GUICanvas.h"

class ModifyInterface;
class Vehicle;

// A symmetric distribution handles all symmetric parts in a group
// Behaviour is handled in lua, but this lua code runs in the editor only as during
// flight symmetry modes exist only as metadata to know which parts are symmetric!
class SymmetryMode
{
private:
	sol::environment env;

public:
	// Do not use keys starting with "__" as these are used by the engine
	std::shared_ptr<cpptoml::table> save_toml;

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
	// ONLY CALLED IN THE EDITOR
	void on_dirty(Piece* piece);

	// Calld when any of the mirrored, child pieces, is disconnected,
	// so that mirrored versions are properly deleted
	// ONLY CALLED IN THE EDITOR
	void on_disconnect(Piece* piece);

	// ONLY CALLED IN THE EDITOR
	void init(sol::state* in_state, const std::string& pkg);

	// ONLY CALLED IN THE EDITOR
	void do_gui(std::shared_ptr<GUICanvas> to_canvas, ModifyInterface* mod_int);

};

// This is used only for the prototype and metadata, it, the stuff that's needed for
// the vehicle in flight. Lua stuff is loaded in init(), but ONLY IN THE EDITOR
template<>
class GenericSerializer<SymmetryMode>
{
public:
	static void serialize(const SymmetryMode& what, cpptoml::table& target)
	{
		// We use the prototype and override with save_toml, if prototype is not
		// available, then the symmetry mode will be saved "headless"
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

		// Read everything into save_toml (TODO: override kind of useless overhead?)
		to.save_toml = cpptoml::make_table();
		SerializeUtil::override(*to.save_toml, from);
	}
};


