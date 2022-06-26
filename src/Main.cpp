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


	auto* audio = new AssetHandle<AudioClip>("debug_system:test_audio.wav");
	std::weak_ptr<AudioSource> src2 = osp->audio_engine->create_audio_source(1);
	src2.lock()->set_position(glm::dvec3(1.0, 0.0, 2.0));
	src2.lock()->set_source_clip(*audio);
	src2.lock()->set_looping(true);
	src2.lock()->set_3d_source(true);
	src2.lock()->set_playing(false);

	osp->audio_engine->set_listener(glm::dvec3(0.0, 0.0, 0.0),
									glm::dvec3(1.0, 0.0, 0.0),
									glm::dvec3(0.0, 1.0, 0.0),
									glm::dvec3(0.0, 0.0, 0.0),
									360.0);

	osp->game_state->load_scene(new EditorScene());

	double t = 0.0;


	while (osp->should_loop())
	{
		PROFILE_BLOCK("frame");

		osp->start_frame();
		osp->update();
		osp->render();
		osp->finish_frame();

		src2.lock()->set_position(glm::dvec3(cos(t), 0.0, sin(t)));

		t+=osp->game_dt*0.4f;

	}

	delete audio;

	osp->finish();
}
