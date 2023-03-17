#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "part/Part.h"
#include "gui/GUICanvas.h"

class ModifyInterface;
class ModifyPanel;
class EditorVehicleInterface;
class EditorVehicle;
class Vehicle;
class EditorScene;

// A symmetric distribution handles all symmetric parts in a group
// Behaviour is handled in lua, but this lua code runs in the editor only as during
// flight symmetry modes exist only as metadata to know which parts are symmetric!
// Cloning of the CHILD pieces is done in C++, with lua having the responsability of giving
// the root pieces the correct attachment positions, and also handling wiring and plumbing!
class SymmetryMode
{
private:
	sol::environment env;
	sol::state* st;
	EditorVehicle* edveh;
	EditorScene* sc;

public:
	std::vector<Piece*> all_in_symmetry;

	// Do not use keys starting with "__" as these are used by the engine
	std::shared_ptr<cpptoml::table> save_toml;

	// Metadata, available even without init
	std::string ui_name;
	std::string ui_desc;
	std::string script_path;
	bool can_use_stack_attachments;
	bool can_use_radial_attachments;
	AssetHandle<Image> icon;

	// Guaranteed to have associated part, as we don't allow making simetries with
	// orphaned pieces
	Piece* root;
	// Includes the original piece as first vector element (root)
	std::vector<Piece*> clones;
	std::string attachment_used;

	// Called when any of the mirrored pieces is modified in any way, including disconnection
	// ONLY CALLED IN THE EDITOR
	void on_dirty(Piece* piece);

	// Calld when any of the mirrored, child pieces, is disconnected,
	// so that mirrored versions are properly deleted
	// ONLY CALLED IN THE EDITOR
	void on_disconnect(Piece* piece);

	// Returns all root pieces, including the one that the symmetry
	// was created from, BUT NOT CHILDREN!
	std::vector<Piece*> make_clones(int count);

	bool is_piece_in_symmetry(Piece* p);

	// ONLY CALLED IN THE EDITOR
	void init(sol::state* in_state, EditorVehicle* in_vehicle, const std::string& pkg);

	// Called if the back button is clicked in the editor and the can_go_back option is set
	void gui_go_back();

	// Once this function is called, you are free to set the symmetry panel to anything,
	// take piece select events, etc, as the symmetry is being modified
	void take_gui_control(ModifyPanel* panel, ModifyInterface* interface, EditorVehicleInterface* edveh_int);

	// Once this function is called, you must no longer modify the symmetry panel,
	// nor handle any piece select events or similar, as the symmetry is not being modified
	// NOTE: symmetry_panel and modify_interface will be null!
	void leave_gui_control();

};

// This is used only for the prototype and metadata, it, the stuff that's needed for
// the vehicle in flight. Lua stuff is loaded in init(), but ONLY IN THE EDITOR
// If loading the symmetry mode from a saved vehicle, remember to override with the
// prototype toml!
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
		SAFE_TOML_GET(to.can_use_stack_attachments, "__can_use_stack_attachments", bool);
		SAFE_TOML_GET(to.can_use_radial_attachments, "__can_use_radial_attachments", bool);

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


