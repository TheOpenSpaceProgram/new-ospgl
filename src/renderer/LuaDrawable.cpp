#include "LuaDrawable.h"
#include <util/LuaUtil.h>

#include <utility>

LuaDrawable::LuaDrawable(sol::table from_table)
{
	this->table = std::move(from_table);
	// Find all of required values in lua
	this->needs_deferred = table["deferred_pass"].valid() && table["deferred_pass"].get_type() == sol::type::function;
	this->needs_forward = table["forward_pass"].valid() && table["forward_pass"].get_type() == sol::type::function;
	this->needs_gui = table["gui_pass"].valid() && table["gui_pass"].get_type() == sol::type::function;
	this->needs_shadow = table["shadow_pass"].valid() && table["shadow_pass"].get_type() == sol::type::function;
	this->needs_far_shadow = table["far_shadow_pass"].valid() && table["far_shadow_pass"].get_type() == sol::type::function;
	this->needs_env_map = table["drawable_env_map_enable"].valid() && table["drawable_env_map_enable"].get_or(false);
}

int LuaDrawable::get_forward_priority()
{
	return LuaUtil::call_function_if_present_returns<int>(table["get_forward_priority"]).value_or(0);
}

void LuaDrawable::deferred_pass(CameraUniforms &cu, bool is_env_map)
{
	LuaUtil::call_function_if_present(table["deferred_pass"], cu, is_env_map);
}

void LuaDrawable::forward_pass(CameraUniforms &cu, bool is_env_map)
{
	LuaUtil::call_function_if_present(table["forward_pass"], cu, is_env_map);
}

void LuaDrawable::gui_pass(CameraUniforms &cu)
{
	LuaUtil::call_function_if_present(table["gui_pass"], cu);
}

void LuaDrawable::shadow_pass(ShadowCamera &cu)
{
	LuaUtil::call_function_if_present(table["shadow_pass"], cu);
}

void LuaDrawable::far_shadow_pass(ShadowCamera &cu)
{
	LuaUtil::call_function_if_present(table["far_shadow_pass"], cu);
}
