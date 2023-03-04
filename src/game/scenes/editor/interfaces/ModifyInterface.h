#pragma once
#include "BaseInterface.h"

class EditorVehicleInterface;

class ModifyInterface : public BaseInterface
{
private:
	EditorVehicleInterface* edveh_int;
	EditorVehicle* edveh;

	bool do_interface_idle(Piece* hovered, GUIInput* ipt);
public:
	enum State
	{
		// The editor is waiting for the user to initiate an action, highlighting and
		// context menu creation is possible
		IDLE,
		// The editor is waiting for the user to click a piece to create symmetry
		CREATING_SYMMETRY,
		// The editor is waiting for the user to click a piece to re-root
		RE_ROOTING,
		// The editor is waiting for the user to click a piece for lua reasons
		// (select_piece event)
		SELECTING_PIECE,

	};
	State cur_state;

	void change_state(State st);


	void update(double dt) override;

	bool do_interface(const CameraUniforms& cu, glm::dvec3 ray_start, glm::dvec3 ray_end,
							  glm::dvec4 viewport, NVGcontext* vg, GUIInput* gui_input, GUISkin* gui_skin) override;

	// Called when the interface changes by user input
	void leave() override;

	bool can_leave() override;

	ModifyInterface(EditorVehicleInterface* edveh_int);
};
