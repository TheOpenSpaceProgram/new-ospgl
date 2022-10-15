#include "VehicleEntity.h"

#include "../../../renderer/Renderer.h"
#include "../../Universe.h"

void VehicleEntity::init()
{
	this->vehicle->set_world(get_universe()->bt_world);
	this->vehicle->init(get_universe());
}

void VehicleEntity::update(double dt)
{
	vehicle->update(dt);
}

void VehicleEntity::physics_update(double pdt)
{
	vehicle->physics_update(pdt);
}

VehicleEntity::VehicleEntity(Vehicle* vehicle) : debug(this)
{
	this->vehicle = vehicle;
}

VehicleEntity::VehicleEntity(cpptoml::table& toml) : debug(this)
{

}


VehicleEntity::~VehicleEntity()
{
	delete vehicle;
}

void VehicleEntity::enable_bullet(btDynamicsWorld * world)
{
}

void VehicleEntity::disable_bullet(btDynamicsWorld * world)
{
}



void VehicleEntity::deferred_pass(CameraUniforms& camera_uniforms, bool is_env)
{
	for (Piece* p : vehicle->all_pieces)
	{
		glm::dmat4 tform = to_dmat4(p->get_graphics_transform()) * glm::inverse(p->collider_offset);
		p->model_node->draw(camera_uniforms, tform, drawable_uid, true);
	}

}

void VehicleEntity::shadow_pass(ShadowCamera& sh_cam)
{
	for(Piece* p : vehicle->all_pieces)
	{
		glm::dmat4 tform = to_dmat4(p->get_graphics_transform()) * glm::inverse(p->collider_offset);
		p->model_node->draw_shadow(sh_cam, tform, true);
	}
}

glm::dvec3 VehicleEntity::get_visual_origin()
{
	if(vehicle->is_packed())
	{
		//TODO
		return glm::dvec3(0, 0, 0);
	}
	else
	{
		return vehicle->unpacked_veh.get_center_of_mass(true);
	}
}

glm::dvec3 VehicleEntity::get_physics_origin()
{
	if(vehicle->is_packed())
	{
		//TODO
		return glm::dvec3(0, 0, 0);
	}
	else
	{
		return vehicle->unpacked_veh.get_center_of_mass(false);
	}
}
