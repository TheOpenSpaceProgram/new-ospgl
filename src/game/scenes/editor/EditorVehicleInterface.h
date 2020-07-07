#pragma once
#include "EditorVehicle.h"

// Handles user interaction with the EditorVehicle
class EditorVehicleInterface
{
private:

	struct RaycastResult
	{
		bool has_hit;
		Piece* p;
		glm::dvec3 world_pos;
		glm::dvec3 world_nrm;
	};

	RaycastResult raycast(glm::dvec3 start, glm::dvec3 end,	bool ignore_non_radial,
		   	std::vector<Piece*> ignore = std::vector<Piece*>());

	void handle_input_hovering(const CameraUniforms& cu, 
		glm::dvec3 ray_start, glm::dvec3 ray_end);

	void handle_input_selected(const CameraUniforms& cu, 
		glm::dvec3 ray_start, glm::dvec3 ray_end);

	// Re-roots assuming selected piece is 'current', and new root is 'new_root'
	void reroot(Piece* current, Piece* new_root);

	// Called when a selection is started
	void on_selection_change(const CameraUniforms& cu);
	// Called when the attachment point changes
	void on_attachment_change();

	// Attaches current selected, using the attachment port
	void attach(Piece* target, std::string target_port = "");

	glm::dmat4 get_current_tform(glm::dvec3 mpos);
	// Obtains a good (user-wise) default rotation which makes parts face up
	glm::dquat get_fixed_rotation();

	void use_attachment_port(Piece* target, std::string port);

	PieceAttachment* find_free_attachment();

	void cycle_attachments(int dir);
	void cycle_pieces(int dir);

	Piece* try_attach_stack(glm::dvec3 cpos, const std::vector<Piece*>& children, std::string& attach_marker);
	Piece* try_attach_radial(glm::dvec3 r0, glm::dvec3 r1, const std::vector<Piece*>& children, std::string& attach_marker);

public:

	EditorVehicle* edveh;
	EditorScene* scene;

	// The user selects a single piece, but he can use commands to "move"
	// through all connected pieces to said piece.
	// Once a piece is selected, it's separated from root if any connection
	// is present
	Piece* selected;
	PieceAttachment* selected_attachment;
	std::string ignore_attachment;
	double selected_distance;
	glm::dquat selected_rotation;
	glm::dvec3 selected_offset;

	// The editor prioritizes stack over radial
	bool allow_radial;
	bool allow_stack;

	std::vector<Piece*> selected_buffer;

	// Returns true if GUI should be blocked (we have something selected)
	// Call the function ONLY if key and mouse input is free to use
	// Clicking ANYWHERE outside viewport results in deletion of the selected
	// part
	// Viewport is renderer style, (x0, y0, w, h)
	bool handle_input(const CameraUniforms& cu, glm::dvec4 viewport, glm::dvec2 real_screen_size);
	


	void update(double dt);


	EditorVehicleInterface();
};
