#include "util/Timer.h"

#include "assets/Model.h"
#include "physics/ground/GroundShape.h"
#include "OSP.h"
#include "game/scenes/flight/FlightScene.h"
#include "game/scenes/editor/EditorScene.h"
#include "game/GameState.h"
#include <util/Profiler.h>

#include <universe/vehicle/material/FluidTank.h>

static int iteration = 0;

int main(int argc, char** argv)
{
	osp = new OSP();
	osp->init(argc, argv);

	SerializeUtil::read_file_to("udata/saves/debug-save/save.toml", *osp->game_state);

	double fps_t = 0.0;
	double dt_avg = 0.0;

	PROFILE_FUNC();

	osp->game_state->load_scene(new EditorScene());

	PhysicalMaterial H2;
	PhysicalMaterial O2;
	SerializeUtil::read_file_to(osp->assets->resolve_path("core:materials/hydrogen.toml"), H2);
	SerializeUtil::read_file_to(osp->assets->resolve_path("core:materials/oxygen.toml"), O2);
	StoredFluid H2C = StoredFluid();
	StoredFluid O2C = StoredFluid();
	H2C.gas_mass = 2.0f;
	O2C.gas_mass = 5.0f;

	FluidTank ft;
	ft.volume = 100.0f;
	ft.temperature = 20.28f;
	ft.contents["core:materials/hydrogen.toml"] = H2C;
	ft.contents["core:materials/oxygen.toml"] = O2C;
	ft.ullage_distribution = 1.0f;



	while (osp->should_loop())
	{
		PROFILE_BLOCK("frame");

		osp->start_frame();
		osp->update();
		ft.draw_imgui_debug();
		ft.update(osp->dt, 1.0f);
		if(input->key_pressed(GLFW_KEY_SPACE))
		{
			ft.contents[0].liquid_mass -= 0.01f;
		}
		osp->render();
		osp->finish_frame();
	}

	osp->finish();
}
