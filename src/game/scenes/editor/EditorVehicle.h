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

struct EditorVehiclePiece
{
	EditorVehicleCollider collider;
	bool highlight;
	bool draw_all_attachments;

	EditorVehiclePiece()
	{
		collider.rigid = nullptr;
		collider.udata = nullptr;
		highlight = false;
		draw_all_attachments = false;
	}
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
	std::unordered_map<Piece*, EditorVehiclePiece> piece_meta; 

	// This allows toggling on all "receive" attachments even if nothing is selected
	bool draw_attachments;


	virtual void deferred_pass(CameraUniforms& cu) override;
	virtual void forward_pass(CameraUniforms& cu) override;
	virtual void shadow_pass(ShadowCamera& cu) override;

	virtual bool needs_deferred_pass() override { return true; }
	virtual bool needs_forward_pass() override { return true; }
	virtual bool needs_shadow_pass() override { return true; }

	void init();

	void update(double dt);

	void draw_highlight(Piece* p, glm::vec3 color, CameraUniforms& cu);


	EditorVehicle();

};
