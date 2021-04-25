#include "EditorVehicleInterface.h"
#include "EditorScene.h"
#include <util/InputUtil.h>

EditorVehicleInterface::EditorVehicleInterface(EditorVehicle* v, EditorCamera* cam)
	: edveh(v), scene(v->scene), camera(cam),
	attach_interface(this), wire_interface(this), plumbing_interface(this)
{
}

void EditorVehicleInterface::middle_click_focus(glm::dvec3 ray_start, glm::dvec3 ray_end)
{
	// We first handle middle click (or CTRL+left-click) which centers the camera
	// around a piece
	if(input->mouse_down(GLFW_MOUSE_BUTTON_3) || 
		(input->mouse_down(GLFW_MOUSE_BUTTON_1) && input->key_pressed(GLFW_KEY_LEFT_CONTROL)))
	{
		RaycastResult res = raycast(ray_start, ray_end, false, ignore_center);
		if(res.has_hit)
		{
			camera->center = to_dvec3(res.p->packed_tform.getOrigin());
		}
	}

}

void EditorVehicleInterface::update(double dt) 
{
	current_interface->update(dt);
}

bool EditorVehicleInterface::do_interface(const CameraUniforms& cu, glm::dvec4 viewport, glm::dvec4 gui_viewport,
		glm::dvec2 screen_size, NVGcontext* vg, GUIInput* gui_input, GUISkin* gui_skin)
{
	// We are only called if input is free
	glm::dvec2 subscreen_size = screen_size * glm::dvec2(viewport.z - viewport.x, viewport.w - viewport.y);
	glm::dvec2 subscreen_pos = screen_size * glm::dvec2(viewport.x, viewport.y);
	// This goes from -1 to 1
	glm::dvec2 in_subscreen = (((input->mouse_pos - subscreen_pos) / subscreen_size) - 0.5) * 2.0; 
	in_subscreen.y = -in_subscreen.y;
	auto[ray_start, ray_end] = MathUtil::screen_raycast(in_subscreen, glm::inverse(cu.tform), 1000.0);

	return current_interface->do_interface(cu, ray_start, ray_end, gui_viewport, vg, gui_input, gui_skin); 
}

bool EditorVehicleInterface::can_change_editor_mode() 
{
	return current_interface->can_leave();
}

EditorVehicleInterface::RaycastResult EditorVehicleInterface::raycast(
	glm::dvec3 ray_start, glm::dvec3 ray_end, bool ignore_non_radial,
	std::vector<Piece*> ignore)
{
	RaycastResult res;
	res.has_hit = false;
	res.p = nullptr;

	btCollisionWorld::AllHitsRayResultCallback callback(to_btVector3(ray_start), to_btVector3(ray_end));
	// We cast the ray using bullet
	scene->bt_world->rayTest(to_btVector3(ray_start), to_btVector3(ray_end), callback);

	if(callback.hasHit())
	{
		// We could choose any member array, we just need the length
		size_t hit_count = callback.m_hitPointWorld.size();

		double closest_dist = 99999999.0;
		int closest_index = -1;

		// Find the closest one that isn't a selected part
		for(size_t i = 0; i < hit_count; i++)
		{
			double dist = glm::length(to_dvec3(callback.m_hitPointWorld[i]) - ray_start);
			RigidBodyUserData* udata = (RigidBodyUserData*)callback.m_collisionObjects[i]->getUserPointer();

			logger->check(udata != nullptr, "A rigidbody did not have an user data attached");

			
			if(udata->type == RigidBodyType::PIECE)
			{
				bool is_selected = false;
				// We can't simply check against selected, we need to check
				// all children
				for(Piece* ign : ignore)
				{
					if(ign == udata->as_piece)
					{
						is_selected = true;
						break;
					}
				}

				if(dist <= closest_dist && !is_selected && 
					(udata->as_piece->piece_prototype->allows_radial || !ignore_non_radial))
				{
					closest_index = int(i);
					closest_dist = dist;

					res.has_hit = true;
					res.p = udata->as_piece;
					res.world_pos = to_dvec3(callback.m_hitPointWorld[i]);
					res.world_nrm = to_dvec3(callback.m_hitNormalWorld[i]);
				}
			}
		}

	}

	return res;
}

