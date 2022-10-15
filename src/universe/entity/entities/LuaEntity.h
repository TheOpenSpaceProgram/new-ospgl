#pragma once
#include "../Entity.h"
#include <cpptoml.h>

// Globals:
// entity (itself), universe (not present until init is called)
// And the virtual functions that are seen in C++ may be implemented in lua (except get_type)
// TODO: Think how inter-entity communication should work: Overseer system in the scene / message passing / both?
class LuaEntity : public Entity
{
private:
	sol::environment env;
	sol::state* lua_state;

	std::string type_str;
	std::shared_ptr<cpptoml::table> init_toml;

public:

	// You should start simulating bullet physics here
	void enable_bullet(btDynamicsWorld* world) override;
	// You must stop simulating bullet physics here
	void disable_bullet(btDynamicsWorld* world) override;

	// Return our position to be used by physics loading
	glm::dvec3 get_physics_origin() override;

	// Return position to be used as a sort of visual origin (for camera centering, etc...)
	glm::dvec3 get_visual_origin() override;

	// An approximation of our size, try to go higher than the real number
	// Values of 0.0 means that we don't have a limit for physics loading
	double get_physics_radius() override;

	// Return true if physics are required around this entity
	// (This entity will also be loaded)
	bool is_physics_loader() override;

	// Visual update, always realtime
	void update(double dt) override;

	// Ticks alongside bullet (bullet tick callback)
	// Note: Ticks before bullet update! (pretick)
	void physics_update(double pdt) override;

	// Called when the entity is added into the universe
	// Universe is already initialized
	void init() override;

	// Return true if the physics have stabilized enough for timewarp
	// Vehicles should return false when they are close enough to surfaces
	// or in atmospheric flight
	bool timewarp_safe() override;

	std::string get_type() override;

	explicit LuaEntity(std::string script_path, std::string in_pkg, std::shared_ptr<cpptoml::table> init_toml);

};
