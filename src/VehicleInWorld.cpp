#include "util/Timer.h"

#include "renderer/camera/SimpleCamera.h"

#include <imgui/imgui.h>

#include "universe/Date.h"

#include "vehicle/Vehicle.h"
#include "assets/Model.h"

#include "physics/ground/GroundShape.h"
#include "renderer/lighting/SunLight.h"
#include "renderer/lighting/PointLight.h"

#include "universe/Universe.h"
#include "OSP.h"

#include "vehicle/VehicleLoader.h"
#include "universe/entity/vehicle/VehicleEntity.h" 



int main(int argc, char** argv)
{
	OSP osp = OSP();
	osp.init(argc, argv);


	SimpleCamera* camera = new SimpleCamera();
	osp.renderer->cam = camera;
	camera->speed = 10.0;
	glm::dvec3 cam_offset = glm::dvec3(-10.0, 0.0f, 0.0f);
	camera->fw = glm::normalize(glm::dvec3(1.0f, 0.0f, 0.0));
	camera->pos = cam_offset;
	camera->fov = 60.0f;

	Universe universe = Universe(osp.renderer);
	assets->get_from_path<Config>("debug_system:systems/system.toml")->read_to(universe.system);

	debug_drawer->debug_enabled = true;

	//Date start_date = Date(2000, Date::MAY, 31);
	Date start_date = Date(2030, Date::AUGUST, 21);


	universe.system.t = start_date.to_seconds();
	logger->info("Starting at: {}", start_date.to_string());

	universe.system.init(universe.bt_world);

	universe.system.update(0.0, universe.bt_world, false);
	universe.system.update(0.0, universe.bt_world, true);

	SunLight sun = SunLight();
	osp.renderer->add_light(&sun);

	// Create a vehicle
	auto vehicle_toml = SerializeUtil::load_file("udata/vehicles/Test Vehicle.toml");
	Vehicle* n_vehicle = VehicleLoader::load_vehicle(*vehicle_toml);

	universe.create_entity<VehicleEntity>(n_vehicle);	

	WorldState st = WorldState();
	st.rotation = glm::dquat(1.0, 0.0, 0.0, 0.0f);
	st.cartesian.pos = glm::dvec3(0.0, 0.0, 0.0);
	st.cartesian.vel = glm::dvec3(0.0, 0.0, 0.0);
	st.angular_velocity = glm::dvec3(1.0, 0.0, 0.0);


	n_vehicle->packed_veh.set_world_state(st);
	n_vehicle->unpack();
	

	while (osp.should_loop())
	{
		osp.start_frame();

		if(glfwGetKey(osp.renderer->window, GLFW_KEY_K))
		{
			for(Piece* p : n_vehicle->all_pieces)
			{
				p->welded = false;
			}

			n_vehicle->unpacked_veh.dirty = true;
		}

		if(glfwGetKey(osp.renderer->window, GLFW_KEY_J))
		{
			for(Piece* p : n_vehicle->all_pieces)
			{
				p->welded = true;
			}

			n_vehicle->unpacked_veh.dirty = true;
		}

		for(Piece* p : n_vehicle->all_pieces)
		{
			glm::dvec3 pos = to_dvec3(p->get_global_transform().getOrigin());
			glm::dvec3 tvel = to_dvec3(p->get_tangential_velocity());
			glm::dvec3 avel = to_dvec3(p->get_angular_velocity());
			glm::dvec3 lvel = to_dvec3(p->get_linear_velocity());
			glm::dvec3 rpos = to_dvec3(p->get_relative_position());

			debug_drawer->add_line(pos, pos + tvel * 10.0, glm::vec3(1.0, 0.0, 1.0));
			debug_drawer->add_line(pos, pos + avel * 10.0, glm::vec3(0.0, 0.0, 1.0));	
			debug_drawer->add_line(pos, pos + lvel * 10.0, glm::vec3(1.0, 1.0, 0.0));
			if(p->id == 2)
			{
				debug_drawer->add_point(rpos, glm::vec3(1.0, 1.0, 1.0));
			}
		}

		camera->update(osp.dt);
		universe.update(osp.dt);

		camera->center = n_vehicle->unpacked_veh.get_center_of_mass();
		
		osp.renderer->render();

		osp.finish_frame(universe.MAX_PHYSICS_STEPS * universe.PHYSICS_STEPSIZE);
	}

	osp.finish();
}
