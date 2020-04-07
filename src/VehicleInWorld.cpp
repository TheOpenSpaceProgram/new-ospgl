#include "util/Timer.h"

#include "renderer/camera/SimpleCamera.h"

#include <imgui/imgui.h>



#include "universe/vehicle/Vehicle.h"
#include "assets/Model.h"

#include "physics/ground/GroundShape.h"
#include "renderer/lighting/SunLight.h"
#include "renderer/lighting/PointLight.h"

#include "game/Save.h"

#include "universe/vehicle/VehicleLoader.h"
#include "universe/entity/entities/VehicleEntity.h" 
#include "universe/entity/entities/BuildingEntity.h"


int main(int argc, char** argv)
{
	OSP osp = OSP();
	osp.init(argc, argv);


	SimpleCamera* camera = new SimpleCamera();
	osp.renderer->cam = camera;
	camera->speed = 40.0;
	glm::dvec3 cam_offset = glm::dvec3(-10.0, 0.0f, 0.0f);
	camera->fw = glm::normalize(glm::dvec3(1.0f, 0.0f, 0.0));
	camera->pos = cam_offset;
	camera->fov = 60.0f;

	Save save = Save(&osp);
	SerializeUtil::read_file_to("udata/saves/debug-save/save.toml", save);

	debug_drawer->debug_enabled = true;

	SunLight sun = SunLight();
	osp.renderer->add_light(&sun);

	std::vector<PointLight> lights;

	// Create a vehicle
	Vehicle* n_vehicle = nullptr;

	bool first_frame = true;

	while (osp.should_loop())
	{
		osp.start_frame();



		camera->update(osp.dt);
		save.universe.update(osp.dt);


		if (first_frame)
		{
			auto vehicle_toml = SerializeUtil::load_file("udata/vehicles/Test Vehicle.toml");
			n_vehicle = VehicleLoader::load_vehicle(*vehicle_toml);

			save.universe.create_entity<VehicleEntity>(n_vehicle);

			WorldState st = WorldState();
			st.rotation = glm::dquat(0.0, 0.707, 0.0, 0.707);


			BuildingEntity* lpad_ent = (BuildingEntity*)save.universe.entities[0];
			WorldState stt = lpad_ent->traj.get_state(0.0, true);


			st.cartesian.pos = stt.cartesian.pos;
			st.cartesian.vel = stt.cartesian.vel;
			st.rotation = stt.rotation;

			st.cartesian.pos += stt.rotation * glm::dvec3(0, 0, 1) * 20.0;
			st.angular_velocity = glm::dvec3(0, 0.0, 0);
			n_vehicle->packed_veh.set_world_state(st);
			n_vehicle->unpack();

			first_frame = false;
		}

		if (glfwGetKey(osp.renderer->window, GLFW_KEY_K))
		{
			for (Piece* p : n_vehicle->all_pieces)
			{
				p->welded = false; 
			}

			n_vehicle->unpacked_veh.dirty = true;
		}

		if (glfwGetKey(osp.renderer->window, GLFW_KEY_J))
		{
			for (Piece* p : n_vehicle->all_pieces)
			{
				p->welded = true;
			}

			n_vehicle->unpacked_veh.dirty = true;
		}


		camera->center = n_vehicle->unpacked_veh.get_center_of_mass(true);
		
		osp.renderer->render(&save.universe.system);

		osp.finish_frame(save.universe.MAX_PHYSICS_STEPS * save.universe.PHYSICS_STEPSIZE);

	}

	osp.finish();
}
