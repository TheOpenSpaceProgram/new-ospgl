#include "LuaRenderer.h"

#include <utility>
#include "renderer/Renderer.h"
#include "game/GameState.h"
#include "universe/Universe.h"

#include "renderer/lighting/EnvMap.h"
#include "renderer/lighting/PartIconLight.h"
#include "renderer/lighting/PointLight.h"
#include "renderer/lighting/SunLight.h"
#include "renderer/util/Skybox.h"

#include "LuaAssets.h"

void LuaRenderer::load_to(sol::table& table)
{
	table.new_usertype<Renderer>("renderer",
		  "add_drawable", sol::resolve<void(Drawable*)>(&Renderer::add_drawable),
		  "add_table_as_drawable", [](Renderer* renderer, sol::table table)
		  {
			auto drawable = new LuaDrawable(std::move(table));
			renderer->add_drawable(drawable);
			// Return the pointer to allow manual removal
			return (Drawable*)drawable;
		  },
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
		  "clear", &Renderer::clear,
		  "quality", &Renderer::quality,
		  "env_sample_pos", &Renderer::env_sample_pos,
		  "disable_env_sampling", [](Renderer* rnd)
		  {
			rnd->set_ibl_source(AssetHandle<Cubemap>());
		  },
		  "set_ibl_source", [](Renderer* rnd, LuaAssetHandle<Cubemap>& cmap)
		  {
			rnd->set_ibl_source(cmap.get_asset_handle());
		  },
		  "enable_env_sampling", &Renderer::enable_env_sampling,
		  "get_size", sol::overload([](Renderer* rnd){
		  	auto size = rnd->get_size(false);
		  	return std::make_tuple(size.x, size.y);
		  },
		  [](Renderer* rnd, bool gui)
		  {
			auto size = rnd->get_size(gui);
		  	return std::make_tuple(size.x, size.y);
		  }));

	// TODO: maybe set to sol::readonly?
	table.new_usertype<RendererQuality>("renderer_quality", sol::no_constructor,
		    "sun_shadow_size", &RendererQuality::sun_shadow_size,
			"sun_terrain_shadow_size", &RendererQuality::sun_terrain_shadow_size,
			"secondary_shadow_size", &RendererQuality::secondary_shadow_size,
			"env_map_size", &RendererQuality::env_map_size,
			"use_planet_detail_map", &RendererQuality::use_planet_detail_map,
			"use_planet_detail_normal", &RendererQuality::use_planet_detail_normal,
			"sun_shadow_size", &RendererQuality::sun_shadow_size,
			"pbr", &RendererQuality::pbr,
			"atmosphere", &RendererQuality::atmosphere
			 );

	table.new_usertype<RendererQuality::PBR>("renderer_quality_pbr", sol::no_constructor,
		    "quality", &RendererQuality::PBR::quality,
		    "faces_per_sample", &RendererQuality::PBR::faces_per_sample,
		    "frames_per_sample", &RendererQuality::PBR::frames_per_sample,
		    "distance_per_sample", &RendererQuality::PBR::distance_per_sample,
		    "time_per_sample", &RendererQuality::PBR::time_per_sample,
		    "planet_distance_parameter", &RendererQuality::PBR::planet_distance_parameter,
		    "simple_sampling", &RendererQuality::PBR::simple_sampling
			 );

	table.new_usertype<RendererQuality::Atmosphere>("renderer_quality_atmosphere", sol::no_constructor,
			 "low_end", &RendererQuality::Atmosphere::low_end,
			 "iterations", &RendererQuality::Atmosphere::iterations,
			 "sub_iterations", &RendererQuality::Atmosphere::sub_iterations
	);

	table.new_enum("renderer_quality_pbr_quality",
		   "full", RendererQuality::PBR::Quality::FULL,
		   "simple_envmap", RendererQuality::PBR::Quality::SIMPLE_ENVMAP,
		   "simple", RendererQuality::PBR::Quality::SIMPLE);

	table.new_usertype<Drawable>("drawable");
	table.new_usertype<CameraUniforms>("camera_uniforms",
			"proj", &CameraUniforms::proj,
			"view", &CameraUniforms::view,
			"c_model", &CameraUniforms::c_model,
			"tform", &CameraUniforms::tform,
			"proj_view", &CameraUniforms::proj_view,
			"far_plane", &CameraUniforms::far_plane,
			"cam_pos", &CameraUniforms::cam_pos,
			"screen_size", sol::property([](CameraUniforms* self)
			 {
				return glm::dvec2(self->screen_size.x, self->screen_size.y);
			 },
			 [](CameraUniforms* self, glm::dvec2 size)
			 {
				self->screen_size = glm::dvec2(size.x, size.y);
				self->iscreen_size = glm::ivec2((int)size.x, (int)size.y);
			 }));

	table.new_usertype<Light>("light",
		   "is_added_to_renderer", &Light::is_added_to_renderer,
		   "get_type", &Light::get_type);

	table.new_usertype<EnvMap>("env_map", sol::base_classes, sol::bases<Light>(),
	        "get_type", &EnvMap::get_type);

	table.new_usertype<PartIconLight>("part_icon_light", sol::base_classes, sol::bases<Light>(),
			"get_type", &PartIconLight::get_type,
			"sun_dir", &PartIconLight::sun_dir,
			"color", &PartIconLight::color);

	table.new_usertype<PointLight>("point_light", sol::base_classes, sol::bases<Light>(),
			  "get_type", &PointLight::get_type,
			  "pos", &PointLight::pos,
			  "color", &PointLight::color,
			  "spec_color", &PointLight::spec_color,
			  "set_parameters", &PointLight::set_parameters,
			  "get_constant", &PointLight::get_constant,
			  "get_linear", &PointLight::get_linear,
			  "get_quadratic", &PointLight::get_quadratic,
			  "get_radius", &PointLight::get_radius);

	table.new_usertype<SunLight>("sun_light", sol::base_classes, sol::bases<Light>(),
			  "get_type", &SunLight::get_type,
			  "pos", &SunLight::position,
			  "color", &SunLight::color,
			  "spec_color", &SunLight::spec_color,
			  "ambient_color", &SunLight::ambient_color,
			  "new", sol::constructors<SunLight(int, int)>());

	table.new_usertype<Skybox>("skybox", sol::base_classes, sol::bases<Drawable>(),
	        "intensity", &Skybox::intensity,
	        "new", [](LuaAssetHandle<Cubemap>& cubemap)
		   {
				// This duplicates the asset handle under the hood
				AssetHandle<Cubemap> nhandle = cubemap.get_asset_handle();
				return new Skybox(std::move(nhandle));
		   });

	table.new_usertype<EnvMap>("envmap", sol::base_classes, sol::bases<Light>(),
	        "new", []()
		   {
				return new EnvMap();
		   });
}
