#include "LuaUniverse.h"

#include <utility>
#include <universe/entity/Entity.h>
#include <game/scenes/flight/InputContext.h>
#include "LuaEvents.h"

void LuaUniverse::load_to(sol::table& table)
{

	table.new_usertype<Universe>("universe",
		EVENT_EMITTER_SIGN_UP(Universe),
		EVENT_EMITTER_EMIT(Universe),
		"update", &Universe::update,
		"bt_world", &Universe::bt_world,
		"save_db", &Universe::save_db,
		"system", &Universe::system_ptr,
		"get_entity", &Universe::get_entity,
		"entities", sol::property([](Universe* uv)
		  {
			return sol::as_table(uv->entities);
		  }),
	    "create_entity", [](Universe* uv, const std::string& script_path, sol::this_environment te, sol::variadic_args args)
		 {
			sol::environment& env = te;
			 // Not sure if this is neccesary, maybe just pass args?
			 std::vector<sol::object> args_v;
			 for(auto v : args)
			 {
				 args_v.push_back(v);
			 }
			auto* ent = uv->create_entity<Entity>(script_path,
				   env["__pkg"].get_or<std::string>("core"), nullptr, args_v, true);

			return ent;
		 }
	);

	table.new_usertype<PlanetarySystem>("planetary_system", sol::base_classes, sol::bases<Drawable>(),
	        "get_element_position", &PlanetarySystem::get_element_position,
			"get_element_velocity", &PlanetarySystem::get_element_velocity,
			"get_element", &PlanetarySystem::get_element,
			"elements", &PlanetarySystem::elements
	);

	table.new_usertype<SystemElement>("system_element",
			  "index", sol::readonly(&SystemElement::index),
			  "name", sol::readonly(&SystemElement::name),
			  "nbody", sol::readonly(&SystemElement::nbody),
			  "config", sol::readonly(&SystemElement::config),
			  "dot_factor", sol::readonly(&SystemElement::dot_factor),
			  "render_enabled", &SystemElement::render_enabled
			  );

	table.new_usertype<ElementConfig>("element_confg",
			  "mass", sol::readonly(&ElementConfig::mass),
			  "radius", sol::readonly(&ElementConfig::radius)
	);

	table.new_usertype<Entity>("entity", sol::no_constructor, sol::base_classes, sol::bases<Drawable>(),
	        "enable_bullet", &Entity::enable_bullet,
	        "disable_bullet", &Entity::disable_bullet,
	        "set_bullet_enabled", &Entity::enable_bullet_wrapper,
	        "get_position", &Entity::get_position,
			"get_velocity", &Entity::get_velocity,
	        "get_orientation", &Entity::get_orientation,
			"get_angular_velocity", &Entity::get_angular_velocity,
	        "get_physics_radius", &Entity::get_physics_radius,
	        "is_physics_loader", &Entity::is_physics_loader,
			"get_input_ctx", &Entity::get_input_ctx,
	        "timewarp_safe", &Entity::timewarp_safe,
	        "uid", sol::property(&Entity::get_uid),
	        "get_type", &Entity::get_type,
			"init_toml", &Entity::init_toml,
	        "save", &Entity::save,
			"lua", &Entity::env,
			//"__index", [](Entity* ent, const std::string& idx){ return ent->env[idx]; },
	        "drawable_uid", sol::readonly(&Entity::drawable_uid));

	table.new_usertype<SaveDatabase>("save_database", sol::no_constructor,
		"get_toml", sol::overload(&SaveDatabase::get_toml,
	  [](SaveDatabase* db, const std::string path, sol::this_environment tenv)
	  {
			sol::environment env = tenv;
			return db->get_toml(env["__pkg"].get_or<std::string>("core"), path);
	  }));

	table.new_usertype<WorldState>("world_state", sol::no_constructor,
	   "pos", &WorldState::pos,
	   "vel", &WorldState::vel,
	   "rot", &WorldState::rot,
	   "ang_vel", &WorldState::ang_vel,
	   "get_tform", [](WorldState* self)
	   {
			glm::dmat4 mat = glm::dmat4();
			mat = glm::translate(mat, self->pos) * glm::toMat4(self->rot);
			return mat;
	   });

}
