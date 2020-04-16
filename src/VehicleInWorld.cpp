#include "util/Timer.h"

#include "renderer/camera/SimpleCamera.h"

#include <imgui/imgui.h>



#include "universe/vehicle/Vehicle.h"
#include "assets/Model.h"

#include "physics/ground/GroundShape.h"
#include "renderer/lighting/SunLight.h"

#include "OSP.h"

#include "universe/vehicle/VehicleLoader.h"
#include "game/scenes/flight/FlightScene.h"
#include "game/scenes/editor/EditorScene.h"

#include <util/fmt/glm.h>

#include <game/input/JoystickDebug.h>

static int iteration = 0;


int main(int argc, char** argv)
{
	OSP osp = OSP();
	osp.init(argc, argv);

	SerializeUtil::read_file_to("udata/saves/debug-save/save.toml", osp.game_state);

	double fps_t = 0.0;
	double dt_avg = 0.0;

	osp.game_state.load_scene(new FlightScene());

	{

		SunLight sun = SunLight();
		osp.renderer->add_light(&sun);

		while (osp.should_loop())
		{
			osp.start_frame();

			osp.update();
		
			osp.game_state.universe.bt_world->debugDrawWorld();

			osp.render();
			osp.finish_frame();

			//osp.dt = osp.game_state.universe.PHYSICS_STEPSIZE;
		}
	}

	osp.finish();
}
