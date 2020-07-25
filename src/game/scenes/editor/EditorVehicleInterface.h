#pragma once
#include "EditorVehicle.h"

#include "interfaces/AttachInterface.h"
#include "interfaces/WireInterface.h"
#include "EditorCamera.h"

// Handles user interaction with the EditorVehicle
class EditorVehicleInterface
{
public:

	struct RaycastResult
	{
		bool has_hit;
		Piece* p;
		glm::dvec3 world_pos;
		glm::dvec3 world_nrm;
	};

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
	WireInterface wire_interface;

	// A convenience pointer
	BaseInterface* current_interface;


	// Return true if mouse should be blocked for the GUI
	bool handle_input(const CameraUniforms& cu, glm::dvec4 viewport, 
		glm::dvec2 real_screen_size, GUIInput* gui_input);

	void update(double dt);
	
	void do_gui(NVGcontext* vg, GUISkin* gui_skin, GUIInput* gui_input, glm::vec4 viewport);

	bool can_change_editor_mode();

	EditorVehicleInterface(EditorVehicle* edveh, EditorCamera* camera);
};
