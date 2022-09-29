#include "util/Timer.h"

#include "assets/Model.h"
#include "physics/ground/GroundShape.h"
#include "OSP.h"
#include "game/scenes/flight/FlightScene.h"
#include "game/scenes/editor/EditorScene.h"
#include "game/GameState.h"
#include <util/Profiler.h>

#include <renderer/Renderer.h>
#include <universe/vehicle/plumbing/VehiclePlumbing.h>

#include <assets/AudioClip.h>
#include <audio/AudioSource.h>
#include <audio/AudioEngine.h>

int main(int argc, char** argv)
{
	osp = new OSP();
	osp->init(argc, argv);

	SerializeUtil::read_file_to("udata/saves/debug-save/save.toml", *osp->game_state);

	double fps_t = 0.0;
	double dt_avg = 0.0;

	PROFILE_FUNC();

	osp->game_state->bootstrap();


	while (osp->should_loop())
	{
		PROFILE_BLOCK("frame");

		osp->start_frame();
		osp->update();
		osp->render();
		osp->finish_frame();

	}

	osp->finish();
}
