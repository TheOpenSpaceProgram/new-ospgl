#include "EditorVehicle.h"
#include <util/InputUtil.h>
#include "EditorScene.h"
#include <physics/glm/BulletGlmCompat.h>
#include <util/fmt/glm.h>

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

	colliders[p] = EditorVehicleCollider();
	colliders[p].rigid = rigid_body;
	colliders[p].udata = udata;
}

// Doesn't do anything if not present
void EditorVehicle::remove_collider(Piece* p)
{
	auto it = colliders.find(p);
	if(it != colliders.end())
	{
		scene->bt_world->removeCollisionObject(it->second.rigid);
		delete it->second.rigid;
		delete it->second.udata;
		colliders.erase(it);
	}
}


void EditorVehicle::deferred_pass(CameraUniforms& cu)
{	
	for (Piece* p : veh->all_pieces)
	{
		glm::dmat4 tform = to_dmat4(p->get_graphics_transform()) * glm::inverse(p->collider_offset);
		p->model_node->draw(cu, tform, drawable_uid, true);
	}
}

void EditorVehicle::forward_pass(CameraUniforms& cu)
{
	if(draw_attachments || selected != nullptr || true)
	{
		// We only draw "receiver" models, that is, attachment
		// points which are set to stack (radial goes anywhere)
		for(Piece* p : veh->all_pieces)
		{
			for(std::pair<PieceAttachment, bool>& pair : p->attachments)
			{
				PieceAttachment& attch = pair.first;
				//if(!pair.second && attch.stack == true)
				//{
					glm::dvec3 pos = p->get_marker_position(attch.marker);
					glm::dquat quat = p->get_marker_rotation(attch.marker);

					glm::dmat4 model = glm::translate(glm::dmat4(1.0), pos);
					model = glm::scale(model, glm::dvec3(attch.size * 0.15));
					model = model * glm::toMat4(quat); 

					glm::dmat4 piece_model = to_dmat4(p->get_global_transform());
					model = model * piece_model;

					receive_model->draw(cu, model, drawable_uid, true);	
				//}
			}
		}
	}

	// Draw an overlay over the hovered piece if any
	if(hovered != nullptr)
	{		
		draw_highlight(hovered, glm::vec3(1.0f, 1.0f, 1.0f), cu);
	}

}

void EditorVehicle::shadow_pass(ShadowCamera& cu)
{
	for(Piece* p : veh->all_pieces)
	{
		glm::dmat4 tform = to_dmat4(p->get_graphics_transform()) * glm::inverse(p->collider_offset);
		p->model_node->draw_shadow(cu, tform, true);
	}
}

void EditorVehicle::update(double dt)
{
	hovered = nullptr;
	veh->editor_update(dt);
}

bool EditorVehicle::handle_input(const CameraUniforms& cu, glm::dvec4 viewport, glm::dvec2 screen_size)
{
	// We are only called if input is free
	
	glm::dvec2 subscreen_size = screen_size * glm::dvec2(viewport.z - viewport.x, viewport.w - viewport.y);
	glm::dvec2 subscreen_pos = screen_size * glm::dvec2(viewport.x, viewport.y);
	// This goes from -1 to 1
	glm::dvec2 in_subscreen = (((input->mouse_pos - subscreen_pos) / subscreen_size) - 0.5) * 2.0; 
	in_subscreen.y = -in_subscreen.y;

	auto[ray_start, ray_end] = MathUtil::screen_raycast(in_subscreen, glm::inverse(cu.tform), 1000.0);

	btCollisionWorld::ClosestRayResultCallback callback(to_btVector3(ray_start), to_btVector3(ray_end));
	// We cast the ray using bullet
	scene->bt_world->rayTest(to_btVector3(ray_start), to_btVector3(ray_end), callback);

	if(callback.hasHit())
	{
		RigidBodyUserData* udata = (RigidBodyUserData*)callback.m_collisionObject->getUserPointer();
		logger->check_important(udata != nullptr, "A rigidbody did not have an user data attached");

		// We only care about PIECE colliders, so ignore everything else 
		if(udata->type == RigidBodyType::PIECE)
		{
			hovered = udata->as_piece;
		}
	}


	return false;
}


void EditorVehicle::init()
{
	// Load all colliders
	for(Piece* p : veh->all_pieces)
	{
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

	selected = nullptr;
	draw_attachments = false;

}


void EditorVehicle::draw_highlight(Piece* p, glm::vec3 color, CameraUniforms& cu)
{
	glm::dmat4 tform = to_dmat4(hovered->get_graphics_transform()) * glm::inverse(hovered->collider_offset);

	// Override the hover color
	MaterialOverride over = MaterialOverride();
	over.uniforms["color"] = Uniform(color);

	hovered->model_node->draw_override(cu, &(*mat_hover), tform, drawable_uid, &over, true);
}
