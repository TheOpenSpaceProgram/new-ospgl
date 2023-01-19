#pragma once
#include "Drawable.h"
#include <sol/sol.hpp>

// Allows any table as a drawable, it will check if pass functions are present
// and call them when neccesary. Can be used with great freedom, as it may implement
// for example all drawing on a scene, instead of just "an object".
// Make sure your table has the required functions at time of creation of the drawable!
// Once removed from the renderer, the lua object will not be set to nil or similar, it
// simply holds a reference.
// Furthermore, it will set the following inside the passed table:
// - drawable_uid: number, set during creation
// Finally, if you want to be included in env_map passes, include a "drawable_env_map_enable = true" member in the table!
class LuaDrawable : public Drawable
{
private:
	sol::table table;
	bool needs_deferred;
	bool needs_forward;
	bool needs_gui;
	bool needs_shadow;
	bool needs_far_shadow;
	bool needs_env_map;

public:

	void deferred_pass(CameraUniforms& cu, bool is_env_map = false) override;
	void forward_pass(CameraUniforms& cu, bool is_env_map = false) override;
	void gui_pass(CameraUniforms& cu) override;
	void shadow_pass(ShadowCamera& cu) override;
	void far_shadow_pass(ShadowCamera& cu) override;

	bool needs_deferred_pass() override { return needs_deferred; }
	bool needs_forward_pass() override { return needs_forward; }
	bool needs_gui_pass() override { return needs_gui; }
	bool needs_shadow_pass() override { return needs_shadow; }
	bool needs_far_shadow_pass() override { return needs_far_shadow; }
	bool needs_env_map_pass() override { return needs_env_map; }


	// Objects with higher priority get drawn first
	int get_forward_priority() override;

	explicit LuaDrawable(sol::table from_table);

};
