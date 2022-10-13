#include "LuaRenderer.h"
#include "renderer/Renderer.h"
#include "game/GameState.h"
#include "universe/Universe.h"

void LuaRenderer::load_to(sol::table& table)
{
	table.new_usertype<Renderer>("renderer",
		  "add_drawable", sol::resolve<void(Drawable*)>(&Renderer::add_drawable),
		  "remove_drawable", &Renderer::remove_drawable,
		  "add_light", &Renderer::add_light,
		  "remove_light", &Renderer::remove_light,
		  "remove_all_drawables", &Renderer::remove_all_drawables,
		  "remove_all_lights", &Renderer::remove_all_lights,
		  "render", [](Renderer* rnd)
			 {
				// TODO: Maybe give the chance of changing rendered system?
				// Probably not a good idea
				rnd->render(&osp->game_state->universe.system);
			 },
		  "clear", &Renderer::clear);

	table.new_usertype<Drawable>("drawable");
	table.new_usertype<CameraUniforms>("camera_uniforms",
			"proj", &CameraUniforms::proj,
			"view", &CameraUniforms::view,
			"c_model", &CameraUniforms::c_model,
			"tform", &CameraUniforms::tform,
			"proj_view", &CameraUniforms::proj_view,
			"far_plane", &CameraUniforms::far_plane,
			"cam_pos", &CameraUniforms::cam_pos);

}
