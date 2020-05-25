#pragma once
#include <universe/vehicle/Vehicle.h>
#include <renderer/Drawable.h>
#include <universe/vehicle/VehicleLoader.h>
#include <gui/GUIInput.h>
#include <physics/RigidBodyUserData.h>

class EditorScene;

struct EditorVehicleCollider
{
	btRigidBody* rigid;
	RigidBodyUserData* udata;
};


// Only what's needed of a vehicle for the editor, rendering, vehicle data
// and also the part colliders, but without building welded groups, links,
// or anything like that (they are used for picking)
class EditorVehicle : public Drawable
{
private:

	AssetHandle<Material> mat_hover;
	// The little attachment "handles"
	// These are drawn in a forward pass as they require transparency
	GPUModelNodePointer stack_model, radial_model, stack_radial_model, receive_model;	
	
	void create_collider(Piece* p);
	void remove_collider(Piece* p);

public:

	EditorScene* scene;


	Vehicle* veh;
	std::unordered_map<Piece*, EditorVehicleCollider> colliders; 

	// This allows toggling on all "receive" attachments even if nothing is selected
	bool draw_attachments;

	// The user selects a single piece, but he can use commands to "move"
	// through all connected pieces to said piece.
	// Once a piece is selected, it's separated from root if any connection
	// is present
	Piece* selected;
	std::string selected_attachment;
	double selected_distance;

	Piece* hovered;

	virtual void deferred_pass(CameraUniforms& cu) override;
	virtual void forward_pass(CameraUniforms& cu) override;
	virtual void shadow_pass(ShadowCamera& cu) override;

	virtual bool needs_deferred_pass() override { return true; }
	virtual bool needs_forward_pass() override { return true; }
	virtual bool needs_shadow_pass() override { return true; }

	void init();

	void update(double dt);
	// Returns true if GUI should be blocked (we have something selected)
	// Call the function ONLY if key and mouse input is free to use
	// Clicking ANYWHERE outside viewport results in deletion of the selected
	// part
	// Viewport is renderer style, (x0, y0, w, h)
	bool handle_input(const CameraUniforms& cu, glm::dvec4 viewport, glm::dvec2 real_screen_size);

	void draw_highlight(Piece* p, glm::vec3 color, CameraUniforms& cu);

	void on_selection_change(const CameraUniforms& cu);

	EditorVehicle();

};
