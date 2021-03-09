#include "util/Timer.h"

#include "assets/Model.h"
#include "physics/ground/GroundShape.h"
#include "OSP.h"
#include "game/scenes/flight/FlightScene.h"
#include "game/scenes/editor/EditorScene.h"
#include "game/GameState.h"
#include "game/database/GameDatabase.h"
#include <util/Profiler.h>

#include <universe/vehicle/material/FluidTank.h>
#include <renderer/Renderer.h>

int main(int argc, char** argv)
{
	osp = new OSP();
	osp->init(argc, argv);

	SerializeUtil::read_file_to("udata/saves/debug-save/save.toml", *osp->game_state);

	double fps_t = 0.0;
	double dt_avg = 0.0;

	PROFILE_FUNC();

	osp->game_state->load_scene(new EditorScene());

	StoredFluid H2C;
	H2C.gas_mass = 1.0f;
	H2C.liquid_mass = 0.0f;

	FluidTank ft;
	ft.volume = 0.1f;
	ft.temperature = 375.15f;
	ft.contents[AssetHandle<PhysicalMaterial>("core:materials/water.toml")] = H2C;
	ft.ullage_distribution = 1.0f;
	ft.go_to_equilibrium();


	while (osp->should_loop())
	{
		PROFILE_BLOCK("frame");

		osp->start_frame();
		osp->update();
		ft.draw_imgui_debug();
		if(input->key_pressed(GLFW_KEY_SPACE))
		{
			ft.exchange_heat(10000000.0f * osp->dt);
			float atm_mol = (1.0f * ft.get_ullage_volume() * 1e3) / (0.082f * ft.temperature);
			float atm_kg = atm_mol * 18.015e-3f;
			//ft.contents["core:materials/water.toml"].gas_mass = glm::min(atm_kg, ft.contents["core:materials/water.toml"].gas_mass);
		}


		ft.update((float)osp->dt, 1.0f);
		osp->render();
		osp->finish_frame();
	}

	osp->finish();
}
