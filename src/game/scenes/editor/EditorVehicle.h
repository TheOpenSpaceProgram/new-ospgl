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


// Not a particularly efficient strategy, but works well
struct EditorVehiclePiece
{
	EditorVehicleCollider collider;
	glm::vec3 highlight;
	bool draw_out_attachments;
	bool draw_in_attachments;

	std::unordered_map<std::string, glm::vec4> attachment_color;

	EditorVehiclePiece()
	{
		collider.rigid = nullptr;
		collider.udata = nullptr;
		highlight = glm::vec3(0.0f);
		draw_out_attachments = false;
		draw_in_attachments = true;
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
	

public:

	void update_collider(Piece* p);
	// Does not delete the piece_meta
	void remove_collider(Piece* p);

	EditorScene* scene;

	Vehicle* veh;
	std::unordered_map<Piece*, EditorVehiclePiece> piece_meta; 

	void clear_meta();

	bool draw_attachments;

	virtual void deferred_pass(CameraUniforms& cu, bool is_env_map = false) override;
	virtual void forward_pass(CameraUniforms& cu, bool is_env_map = false) override;
	virtual void shadow_pass(ShadowCamera& cu) override;

	virtual bool needs_deferred_pass() override { return true; }
	virtual bool needs_forward_pass() override { return true; }
	virtual bool needs_shadow_pass() override { return true; }

	void init(sol::state* lua_state);

	void update(double dt);

	void draw_highlight(Piece* p, glm::vec3 color, CameraUniforms& cu);

	void deserialize(cpptoml::table& from);


	EditorVehicle(EditorScene* scene);

};

template<>
class GenericSerializer<EditorVehicle>
{
public:

	static void serialize(const EditorVehicle& what, cpptoml::table& target);
	static void deserialize(EditorVehicle& to, const cpptoml::table& from);
};