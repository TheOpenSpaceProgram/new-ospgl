#include "EditorVehicle.h"
#include <util/InputUtil.h>
#include "EditorScene.h"
#include <physics/glm/BulletGlmCompat.h>
#include <util/fmt/glm.h>
#include <GLFW/glfw3.h>

void EditorVehicle::create_collider(Piece* p)
{
	remove_collider(p);
	// Now create it

	btRigidBody::btRigidBodyConstructionInfo info(0.0, nullptr, p->collider, btVector3(0.0, 0.0, 0.0));
	btRigidBody* rigid_body = new btRigidBody(info);

	btTransform trans = p->get_graphics_transform();
	rigid_body->setWorldTransform(trans);

	RigidBodyUserData* udata = new RigidBodyUserData(p);

	rigid_body->setUserPointer((void*)udata);

	scene->bt_world->addCollisionObject(rigid_body);

	piece_meta[p].collider.rigid = rigid_body;
	piece_meta[p].collider.udata = udata;
}

// Doesn't do anything if not present
void EditorVehicle::remove_collider(Piece* p)
{
	EditorVehiclePiece& mp = piece_meta[p];

	if(mp.collider.rigid != nullptr)
	{
		scene->bt_world->removeCollisionObject(mp.collider.rigid);
		delete mp.collider.rigid;
		delete mp.collider.udata;
		mp.collider.rigid = nullptr;
		mp.collider.udata = nullptr;
	}
}


void EditorVehicle::deferred_pass(CameraUniforms& cu)
{	
	for (Piece* p : veh->all_pieces)
	{
		p->model_node->draw(cu, p->get_graphics_matrix(), drawable_uid, true);
	}
}

void EditorVehicle::forward_pass(CameraUniforms& cu)
{
	// We only draw "receiver" models, that is, attachment
	// points which are set to stack (radial goes anywhere)
	for(Piece* p : veh->all_pieces)
	{
		if(draw_attachments)
		{
			for(std::pair<PieceAttachment, bool>& pair : p->attachments)
			{
				PieceAttachment& attch = pair.first;
				if(!pair.second && attch.stack == true)
				{
					glm::dvec3 pos = p->get_marker_position(attch.marker);
					glm::dquat quat = p->get_marker_rotation(attch.marker);

					glm::dmat4 model = glm::translate(glm::dmat4(1.0), pos);
					model = glm::scale(model, glm::dvec3(attch.size * 0.15));
					model = model * glm::toMat4(quat); 

					model = p->get_graphics_matrix() * model;

					receive_model->draw(cu, model, drawable_uid, true);	
				}
			}
		}

		if(piece_meta[p].highlight)
		{
			draw_highlight(p, glm::vec3(1.0f, 1.0f, 1.0f), cu);
		}
	}
}

void EditorVehicle::shadow_pass(ShadowCamera& cu)
{
	for(Piece* p : veh->all_pieces)
	{
		p->model_node->draw_shadow(cu, p->get_graphics_matrix(), true);
	}
}

void EditorVehicle::update(double dt)
{
	veh->editor_update(dt);

}


void EditorVehicle::init()
{
	// Load all colliders
	for(Piece* p : veh->all_pieces)
	{
		piece_meta[p] = EditorVehiclePiece();
		create_collider(p);
	}
}


EditorVehicle::EditorVehicle() : Drawable()
{
	auto vehicle_toml = SerializeUtil::load_file("udata/vehicles/Test Vehicle.toml");
	veh = VehicleLoader(*vehicle_toml).get_vehicle();
	
	// Load the different models
	std::string model_path = *SerializeUtil::load_file(assets->resolve_path("core:meshes/editor_attachment.toml"))
		->get_as<std::string>("model");

	AssetHandle<Model> model = AssetHandle<Model>(model_path);

	stack_model = GPUModelNodePointer(model.duplicate(), "stack");
	radial_model = GPUModelNodePointer(model.duplicate(), "radial");	
	stack_radial_model = GPUModelNodePointer(model.duplicate(), "stack_radial");
	receive_model = GPUModelNodePointer(model.duplicate(), "receive");

	mat_hover = AssetHandle<Material>("core:mat_hover.toml");

	draw_attachments = false;

}


void EditorVehicle::draw_highlight(Piece* p, glm::vec3 color, CameraUniforms& cu)
{
	// Override the hover color
	MaterialOverride over = MaterialOverride();
	over.uniforms["color"] = Uniform(color);

	p->model_node->draw_override(cu, &(*mat_hover), p->get_graphics_matrix(), drawable_uid, &over, true);
}
