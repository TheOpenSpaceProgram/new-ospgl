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
	auto n_vehicles = vehicle->physics_update(pdt);
	for(Vehicle* n_vehicle : n_vehicles)
	{
		get_universe()->create_entity<VehicleEntity>(n_vehicle);
	}
}

VehicleEntity::VehicleEntity(Vehicle* vehicle)
{
	this->vehicle = vehicle;
}

VehicleEntity::VehicleEntity(cpptoml::table& toml)
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

void VehicleEntity::deferred_pass(CameraUniforms & camera_uniforms)
{
	for (Piece* p : vehicle->all_pieces)
	{
		glm::dmat4 tform = /* glm::inverse(p->collider_offset) */ to_dmat4(p->get_graphics_transform());
		p->model_node->draw(camera_uniforms, tform, true);
	}
}

void VehicleEntity::shadow_pass(ShadowCamera& sh_cam)
{
	for(Piece* p : vehicle->all_pieces)
	{
		glm::dmat4 tform = glm::inverse(p->collider_offset) * to_dmat4(p->get_graphics_transform());
		p->model_node->draw_shadow(sh_cam, tform, true);
	}
}
