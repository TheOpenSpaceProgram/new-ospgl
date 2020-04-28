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

	scene->bt_world->addCollisionObject(rigid_body);

	colliders[p] = EditorVehicleCollider();
	colliders[p].rigid = rigid_body;
}

// Doesn't do anything if not present
void EditorVehicle::remove_collider(Piece* p)
{
	auto it = colliders.find(p);
	if(it != colliders.end())
	{
		scene->bt_world->removeCollisionObject(it->second.rigid);
		delete it->second.rigid;
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
	if(draw_attachments || selected != nullptr)
	{
		// We only draw "receiver" models, that is, attachment
		// points which are set to stack
		for(Piece* p : veh->all_pieces)
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

					receive_model->draw(cu, model, drawable_uid, true);	
				}
			}
		}
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

	// This is in the near plane
	glm::dvec4 ray_start_ndc = glm::dvec4(in_subscreen, -1.0, 1.0);
	// This is in the far plane, potentially very very far away 
	glm::dvec4 ray_end_ndc = glm::dvec4(in_subscreen, 0.0, 1.0);

	glm::dmat4 inv_tform = glm::inverse(cu.proj_view * cu.c_model);
	glm::dvec4 ray_start4 = inv_tform * ray_start_ndc; ray_start4 /= ray_start4.w;
	glm::dvec4 ray_end4 = inv_tform * ray_end_ndc; ray_end4 /= ray_end4.w;
	glm::dvec3 ray_start = ray_start4, ray_rend = ray_end4;
	glm::dvec3 direction = glm::normalize(ray_rend - ray_start);
	glm::dvec3 ray_end = ray_start + direction * 100.0;

	logger->info("({} => {})", ray_start_ndc, ray_end_ndc);
	logger->info("{} -> {}", ray_start, ray_end);
	
	logger->info("Cam pos: {}", cu.cam_pos);

	btCollisionWorld::ClosestRayResultCallback callback(to_btVector3(ray_start), to_btVector3(ray_end));
	// We cast the ray using bullet
	scene->bt_world->rayTest(to_btVector3(ray_start), to_btVector3(ray_end), callback);

	debug_drawer->add_arrow(ray_start, ray_end, glm::vec3(1.0, 0.0, 1.0));

	if(callback.hasHit())
	{
		logger->info("YEAH BOY!");
		glm::dvec3 p = to_dvec3(callback.m_hitPointWorld);
		glm::dvec3 n = to_dvec3(callback.m_hitNormalWorld);
		debug_drawer->add_point(p, glm::vec3(1.0, 1.0, 1.0));
		debug_drawer->add_line(p, p + n * 2.0, glm::vec3(1.0, 0.0, 0.0));
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
	veh = VehicleLoader::load_vehicle(*vehicle_toml);
	
	// Load the different models
	std::string model_path = *SerializeUtil::load_file(assets->resolve_path("core:meshes/editor_attachment.toml"))
		->get_as<std::string>("model");

	AssetHandle<Model> model = AssetHandle<Model>(model_path);

	stack_model = GPUModelNodePointer(model.duplicate(), "stack");
	radial_model = GPUModelNodePointer(model.duplicate(), "radial");	
	stack_radial_model = GPUModelNodePointer(model.duplicate(), "stack_radial");
	receive_model = GPUModelNodePointer(model.duplicate(), "receive");

	selected = nullptr;
	draw_attachments = false;

}


