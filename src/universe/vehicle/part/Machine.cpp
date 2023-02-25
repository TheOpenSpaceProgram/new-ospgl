#include "Machine.h"
#include <util/Logger.h>
#include <util/SerializeUtil.h>
#include <assets/AssetManager.h>
#include <lua/libs/LuaAssets.h>
#include <game/database/GameDatabase.h>

#include "../Vehicle.h"
#include "Part.h"
#include "sol/sol.hpp"
#include <imgui/imgui.h>


Machine::Machine(std::shared_ptr<cpptoml::table> init_toml, std::string cur_pkg) : plumbing(this)
{
	paused = false;
	step = false;
	this->init_toml = init_toml;
	this->in_pkg = cur_pkg;
	this->editor_location_marker = init_toml->get_as<std::string>("__editor_marker").value_or("");
	this->editor_hidden = init_toml->get_as<bool>("__editor_hidden").value_or(false);

	default_icon = AssetHandle<Image>("core:machines/icons/default_icon.png");

	piece_missing = false;
	runtime_uid = osp->get_runtime_uid();
}

void Machine::load_interface(const std::string& name, sol::table n_table) 
{
	interfaces.insert(std::make_pair(name, n_table));
}

void Machine::pre_update(double dt)
{
	if(!paused || step)
	{
		LuaUtil::call_function_if_present(env["pre_update"], dt);
	}
}

void Machine::update(double dt)
{
	if(!paused || step)
	{
		LuaUtil::call_function_if_present(env["update"], dt);
		step = false;
	}
}

void Machine::physics_update(double dt)
{
	if(!paused || step)
	{
		LuaUtil::call_function_if_present(env["physics_update"], dt);
		// TODO: Handle step properly?
	}
}

void Machine::editor_update(double dt)
{
	if(!paused || step)
	{
		// Called regardless of enabled status
		LuaUtil::call_function_if_present(env["editor_update"], dt);
		step = false;
	}
}

void Machine::init(sol::state* lua_state, Part* in_part)
{
	logger->check(init_toml != nullptr, "Malformed init_toml");
	cpptoml::table& init_toml_p = *init_toml;
	this->lua_state = lua_state;

	std::string script_path;
	SAFE_TOML_GET_FROM(init_toml_p, script_path, "script", std::string);

	// Extract package from script path, as in_pkg is the PART's package
	auto[pkg, name] = osp->assets->get_package_and_name(script_path, in_pkg);
	this->in_pkg = pkg;
	this->name = name;

	// We create a new environment for our script
	env = sol::environment(*lua_state, sol::create, lua_state->globals());
	// We need to load LuaCore to it
	lua_core->load((sol::table&)env, pkg);
	env["machine"] = this;
	env["osp"] = osp;

	this->in_part = in_part;

	std::string old = osp->assets->get_current_package();
	std::string full_path = osp->assets->res_path + pkg + "/" + name;
	auto result = (*lua_state).safe_script_file(full_path, env);
	if(!result.valid())
	{
		sol::error err = result;
		logger->fatal("Lua Error loading machine:\n{}", err.what());
	}

	// Then we simply move over the environment to an entry in the global lua_state
	// TODO: Is this even neccesary?
	//(*lua_state)[this] = sol::table(env);

	plumbing.init(*init_toml);
}

std::vector<Machine*> Machine::get_all_wired_machines(bool include_this)
{
	// TODO: We could cache this? Could be a small perfomance gain
	return get_connected_if([](Machine* m){ return true; }, include_this);
}

std::vector<Machine*> Machine::get_wired_machines_with(const std::vector<std::string>& interfaces, bool include_this)
{
	return get_connected_if([interfaces](Machine* m)
	{
		for(const std::string& a : interfaces)
		{
			if(m->interfaces.find(a) != m->interfaces.end())
			{
				return true;
			}
		}

		return false;
	}, include_this);
}

std::vector<sol::table> Machine::get_wired_interfaces(const std::string& type, bool include_this) 
{
	std::vector<Machine*> machines = get_wired_machines_with({type}, include_this);
	std::vector<sol::table> out;
	out.reserve(machines.size());

	for(Machine* m : machines)
	{
		out.push_back(m->interfaces[type]);
	}

	return out;
}

sol::table Machine::get_interface(const std::string& name) 
{
	auto it = interfaces.find(name);
	if(it == interfaces.end())
	{
		return sol::nil;
	}
	else
	{
		return it->second;
	}
}

AssetHandle<Image> Machine::get_icon() 
{
	auto result = LuaUtil::call_function_if_present(env["get_icon"]);
	if(result.has_value())
	{
		return std::move(result->get<LuaAssetHandle<Image>>().get_asset_handle());
	}
	else
	{
		return default_icon.duplicate();
	}
}

std::shared_ptr<InputContext> Machine::get_input_context()
{
	return LuaUtil::call_function_if_present_returns<std::shared_ptr<InputContext>>(
			env["get_input_context"]
			).value_or(nullptr);
}

Machine::~Machine()
{
	logger->info("Ending machine");
	//env.clear();
}

std::vector<Machine*> Machine::get_connected_if(std::function<bool(Machine*)> fnc, bool include_this) 
{
	std::vector<Machine*> out;

	auto range = in_part->vehicle->wires.equal_range(this);
	for(auto it = range.first; it != range.second; it++)
	{
		if(fnc(it->second))
		{
			out.push_back(it->second);
		}
	}

	if(include_this && fnc(this))
	{
		out.push_back(this);
	}

	return out;
}

bool Machine::is_enabled()
{
	return !piece_missing;
}

std::string Machine::get_pkg()
{
	return in_pkg;
}

std::string Machine::get_name()
{
	return name;
}

std::string Machine::get_id()
{
	std::string ret = in_pkg;
	ret += ":";
	ret += name;
	return ret;
}

bool Machine::draw_imgui()
{
	bool tval = true;
	std::string title = "M(" + in_pkg + ":" + name + "), ruid = " + std::to_string(runtime_uid);
	ImGui::SetNextWindowSize(ImVec2(0, 0));
	ImGui::Begin(title.c_str(), &tval, ImGuiWindowFlags_MenuBar);
	if(ImGui::BeginMenuBar())
	{
		ImGui::Button("Focus");
		if(paused)
		{
			if(ImGui::Button("Unpause"))
			{
				paused = false;
			}
			if(ImGui::Button("Step"))
			{
				step = true;
			}
		}
		else
		{
			if (ImGui::Button("Pause"))
			{
				paused = true;
			}
		}
		ImGui::EndMenuBar();
	}
	LuaUtil::call_function_if_present(env["draw_imgui"]);
	ImGui::End();

	return tval;
}

std::string Machine::get_display_name()
{
	return LuaUtil::call_function_if_present_returns<std::string>(env["get_display_name"]).value_or(get_id());
}
