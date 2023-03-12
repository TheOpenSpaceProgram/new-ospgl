#pragma once
#include "BaseInterface.h"

class EditorVehicleInterface;
class SymmetryMode;

class ModifyInterface : public BaseInterface
{
private:
	EditorVehicleInterface* edveh_int;
	EditorVehicle* edveh;

	bool do_interface_idle(Piece* hovered, GUIInput* ipt);
	bool do_interface_select_symmetry(Piece* hovered, GUIInput* ipt);
	bool do_interface_create_symmetry(Piece* hovered, GUIInput* ipt);
	bool do_interface_modify_symmetry(Piece* hovered, GUIInput* ipt);

	// returns highlighted pieces, not including root
	std::vector<Piece*> highlight_symmetry(Piece* root);

	int cur_attachment_point;
	Piece* prev_hover;
	bool is_attachment_compatible(int id, Piece* p);

public:
	enum State
	{
		// The editor is waiting for the user to initiate an action, highlighting and
		// context menu creation is possible
		IDLE,
		// The editor is waiting for the user to click a piece to create symmetry
		// or is modifying a symmetry if selected_piece != nullptr
		CREATING_SYMMETRY,
		// The editor is waiting for the user to click on a piece with symmetry to modify it
		// or the symmetry has been selected and is being modified if selected_piece != nullptr
		// Slight difference: In creating cancel means the symmetry is deleted, here changes undone!
		SELECTING_SYMMETRY,
		// The editor is waiting for the user to click a piece to remove symmetry (instant action)
		ERASING_SYMMETRY,
		// The editor is waiting for the user to click a piece to re-root (instant action)
		RE_ROOTING,
		// The editor is waiting for the user to click a piece for lua reasons
		// (uses select_piece event)
		SELECTING_PIECE,

	};
	Piece* selected_piece;
	// If true and we are creating symmetry, more pieces may be picked and returned
	// using the select_piece event (used by lua)
	bool pick_another_piece;
	State cur_state;

	void change_state(State st);


	void update(double dt) override;

	bool do_interface(const CameraUniforms& cu, glm::dvec3 ray_start, glm::dvec3 ray_end,
							  glm::dvec4 viewport, NVGcontext* vg, GUIInput* gui_input, GUISkin* gui_skin) override;

	// Called when the interface changes by user input
	void leave() override;

	bool can_leave() override;

	void start_picking_piece();

	ModifyInterface(EditorVehicleInterface* edveh_int);
};
