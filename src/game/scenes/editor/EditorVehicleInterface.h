#pragma once
#include "EditorVehicle.h"

#include "interfaces/AttachInterface.h"
#include "interfaces/ModifyInterface.h"
#include "interfaces/WireInterface.h"
#include "interfaces/PlumbingInterface.h"
#include "EditorCamera.h"

// Handles user interaction with the EditorVehicle
// Emits events:
// - on_piece_click(piece_id: int64) - Only emitted if the interface does, if id < 0 then empty space clicked
// - on_select_piece(piece_id: int64) - Emmited by the modify interface, used while creating symmetries or otherwise
//           interacting with the vehicle in modify mode (but not creating context menus)
// - close_context_menus() - Emmited when interface changes must close all non-cut context menus
class EditorVehicleInterface : public EventEmitter
{
public:

	struct RaycastResult
	{
		bool has_hit;
		Piece* p;
		glm::dvec3 world_pos;
		glm::dvec3 world_nrm;
	};

	// This functionality is common to many interfaces, so it's here
	void middle_click_focus(glm::dvec3 ray_start, glm::dvec3 ray_end);

	RaycastResult raycast(glm::dvec3 start, glm::dvec3 end,	bool ignore_non_radial,
		   	std::vector<Piece*> ignore = std::vector<Piece*>());

	EditorVehicle* edveh;
	EditorScene* scene;
	// We are able to center the camera
	EditorCamera* camera;

	// To be ignored by the auto-center action
	std::vector<Piece*> ignore_center;

	// Note: This must be below these two for initializing order!
	AttachInterface attach_interface;
	ModifyInterface modify_interface;
	WireInterface wire_interface;
	PlumbingInterface plumbing_interface;

	// A convenience pointer
	BaseInterface* current_interface;


	void update(double dt);
	
	bool do_interface(const CameraUniforms& cu, glm::dvec4 viewport, glm::dvec4 gui_viewport, 
			glm::dvec2 screen_size, NVGcontext* vg, GUIInput* gui_input, GUISkin* gui_skin);

	bool can_change_editor_mode();

	EditorVehicleInterface(EditorVehicle* edveh, EditorCamera* camera);
};
