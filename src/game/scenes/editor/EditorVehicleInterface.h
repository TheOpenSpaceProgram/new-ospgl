#pragma once
#include "EditorVehicle.h"

// Handles user interaction with the EditorVehicle
class EditorVehicleInterface
{
public:

	EditorVehicle* edveh;
	EditorScene* scene;

	// The user selects a single piece, but he can use commands to "move"
	// through all connected pieces to said piece.
	// Once a piece is selected, it's separated from root if any connection
	// is present
	Piece* selected;
	std::string selected_attachment;
	double selected_distance;
	glm::dquat selected_rotation;

	// Returns true if GUI should be blocked (we have something selected)
	// Call the function ONLY if key and mouse input is free to use
	// Clicking ANYWHERE outside viewport results in deletion of the selected
	// part
	// Viewport is renderer style, (x0, y0, w, h)
	bool handle_input(const CameraUniforms& cu, glm::dvec4 viewport, glm::dvec2 real_screen_size);
	
	// Called when a selection is started
	void on_selection_change(const CameraUniforms& cu);
	// Called when cycling through attachments in the same piece
	void on_attachment_change();
	// Called when cycling through pieces 
	void on_piece_change();

	void cycle_attachments(int dir);

	void update(double dt);

	EditorVehicleInterface();
};
