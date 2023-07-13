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

class SymmetryMode;

struct SymmetryInstance
{
	Piece* p;
	SymmetryMode* mode;
};

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
	EditorScene* sc;

	// If you point to all_in_symmetry[0] = clones[0], clones[clone_depth] = clones[1]
	size_t clone_depth;

	void remove_non_root(EditorVehicle* edveh);

	// Warning: these don't update all_in_symmetry! They leave nullptr marker
	// that must be removed with cleanup()
	void remove_piece(EditorVehicle* edveh, Piece* p);
	void remove_piece_from_symmetry(EditorVehicle* edveh, Piece* p);
	void remove_piece_and_children(EditorVehicle* edveh, Piece* p);
	void remove_piece_and_children_from_symmetry(EditorVehicle* edveh, Piece* p);
	// Removes nullptrs in all_in_symmetry
	void cleanup();
	void update_clone_depth();

	// <-1, -1> = not present in symmetry, otherwise:
	// all_in_symmetry[first * clone_depth + second] = p
	std::pair<int, int> get_piece_sub_index(Piece* p);

public:
	// This is sorted in the same way as the vehicle!
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
	// So that pieces have a place to go on removal of symmetry group
	// attachments are not restored!
	glm::dvec3 original_root_pos;
	glm::dquat original_root_rot;

	// Guaranteed to have associated part, as we don't allow making simetries with
	// orphaned pieces
	Piece* root;
	// Includes the original piece as first vector element. Only includes roots!
	std::vector<Piece*> clones;
	std::string attachment_used;

	// Call when a piece is attached to one belonging to the symmetry
	// (attached_to and transforms must be set!)
	void on_attach(EditorVehicle* edveh, Piece* piece, int sym_depth);

	// Called when any of the mirrored pieces is modified in any way
	// ONLY CALLED IN THE EDITOR
	void on_dirty(Piece* piece);

	// Call to remove given piece from the symmetry. Returns true if it was a root, and then
	// the whole symmetry must be removed. Piece and children will never be removed!
	// ONLY CALLED IN THE EDITOR
	bool disconnect(EditorVehicle* edveh, Piece* piece);

	// Returns all root pieces, including the one that the symmetry
	// was created from, BUT NOT CHILDREN!
	std::vector<Piece*> make_clones(EditorVehicle* edveh, int count);

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

	void remove(EditorVehicle* edveh);
	// Used while dettaching in the editor, removes all pieces belonging to the symmetry
	// except given and children
	// Make sure to destroy symmetry (remove it from array) afterwards as it's no longer symmetry
	void remove_all_but(EditorVehicle* edveh, Piece* p);
	// Finds the cloned father of a given piece, or returns nullptr if not found
	Piece* find_father_clone(Piece* p);
	// Returns all clones, optionally includes p as the first array element
	// If p is not in the symmetry group, the empty vector is returned
	std::vector<SymmetryInstance> find_clones(Piece* p, bool include_p);


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


