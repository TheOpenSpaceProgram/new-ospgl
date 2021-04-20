#pragma once
#include "BaseInterface.h"



class EditorVehicleInterface;


// TODO: Integrate AttachInterface into the other interfaces
// when a piece is tryin to be added OR make it automatically toggle
// to a temporal attach interface that returns to the previous status
// automatically
class AttachInterface : public BaseInterface
{
private:


	void handle_input_hovering(const CameraUniforms& cu, 
		glm::dvec3 ray_start, glm::dvec3 ray_end, GUIInput* gui_input);

	void handle_input_selected(const CameraUniforms& cu, 
		glm::dvec3 ray_start, glm::dvec3 ray_end, GUIInput* gui_input);

	// Re-roots assuming selected piece is 'current', and new root is 'new_root'
	void reroot(Piece* current, Piece* new_root);


	// Called when a selection is started (by the editor)
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

	Piece* try_attach_stack(glm::dvec3 r0, glm::dvec3 r1, glm::dvec3 selected_pos, 
		const std::vector<Piece*>& children, std::string& attach_marker);
	Piece* try_attach_radial(glm::dvec3 r0, glm::dvec3 r1, const std::vector<Piece*>& children, std::string& attach_marker);

public:

	void on_selection_change(double dist = 10.0);
	// The user selects a single piece, but he can use commands to "move"
	// through all connected pieces to said piece.
	// Once a piece is selected, it's separated from root if any connection
	// is present
	Piece* selected;
	// If null the piece has no free attachment points and may only
	// be moved around, not attached
	PieceAttachment* selected_attachment;
	std::string ignore_attachment;
	double selected_distance;
	glm::dquat selected_rotation;
	glm::dvec3 selected_offset;

	// The editor prioritizes stack over radial
	bool allow_radial;
	bool allow_stack;

	std::vector<Piece*> selected_buffer;

	EditorVehicle* edveh;
	EditorScene* scene;
	EditorVehicleInterface* edveh_int;

	bool do_interface(const CameraUniforms& cu, glm::dvec3 ray_start, glm::dvec3 ray_end,
			glm::dvec4 viewport, NVGcontext* vg, GUIInput* gui_input, GUISkin* gui_skin) override;
	
	void update(double dt) override;

	void leave() override;

	bool can_leave() override;
	
	AttachInterface(EditorVehicleInterface* edveh_int);

};
