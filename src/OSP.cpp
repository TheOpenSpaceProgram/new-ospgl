#include "OSP.h"
#include "rang.hpp"
#include <renderer/util/TextDrawer.h>
#include <util/Profiler.h>

InputUtil* input;

#ifdef DETAILED_TIMING
extern Timer timing;
#endif

static void menu_item(const std::string& name, const std::string& value, const std::string& def, const std::string& help)
{
	std::cout << rang::fg::reset;
	std::cout << "-";
	std::cout << rang::fg::yellow;
	std::cout << name;
	std::cout << rang::fg::reset;
	std::cout << "=";
	std::cout << rang::fgB::blue;
	std::cout << value;
	std::cout << rang::fg::reset;
	std::cout << " (Default: ";
	std::cout << rang::fgB::blue;
	std::cout << def;
	std::cout << rang::fg::reset;
	std::cout << ")\n";
	std::cout << rang::fgB::gray;
	std::cout << " " << help << std::endl << std::endl;	
}

static void help_menu()
{

	std::cout << std::endl;
	std::cout << "OSPGL " << OSP::OSP_VERSION << " help:" << std::endl;
	menu_item("settings", "path/to/settings.toml", "settings.toml", "What file to load as the configuration file, relative to the set udata folder");	
	menu_item("res_path", "path/to/res/folder/", "./res/", "Path to the resource folder you want to use. End it with a \"/\"");
	menu_item("udata_path", "path/to/udata/", "./udata/", "Path to the user data folder, ended with a \"/\"");
	std::cout << rang::fgB::gray << "You can override any of the settings in the loaded settings file using this syntax: " << std::endl;
	std::cout << rang::fgB::gray << "-" << rang::fgB::blue << "toml.path" << rang::fg::reset <<
		   	"=" << rang::fgB::blue << "toml-value" << rang::fg::reset << std::endl;
	std::cout << "For example, to change the resolution you could write: " << std::endl;
	std::cout << "-" << rang::fgB::blue << "renderer.width" << rang::fg::reset << "=768 -" << 
			rang::fgB::blue << "renderer.height" << rang::fg::reset << "=768" << std::endl;
	std::cout << std::endl << std::endl;
	std::cout << "Press any key to close the program" << std::endl;
	std::cin.get();
}

bool is_number(const std::string& s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && (std::isdigit(*it) || *it == '.' || *it == '-'))
	{	
		++it;
	}

	return !s.empty() && it == s.end();
}

void OSP::init(int argc, char** argv)
{
	argh::parser args(argc, argv);

	if(args[{"-h", "--help"}])
	{
		help_menu();
	}
	else
	{
		std::string settings_path = "settings.toml";
		std::string res_path = "./res/";
		std::string udata_path = "./udata/";

		std::vector<std::pair<std::string, std::string>> toml_pairs;
		
		for(auto& param : args.params())
		{
			if(param.first == "settings")
			{
				settings_path = param.second;
			}
			else if(param.first == "res_path")
			{
				res_path = param.second;
			}
			else if(param.first == "udata_path")
			{
				udata_path = param.second;
			}
			else
			{
				// Add TOML entry
				std::string real_second = param.second;
				if(param.second != "true" && param.second != "false" && !is_number(param.second))
				{
					real_second.insert(real_second.begin(), '"');
					real_second += '"';
				}	
								
				toml_pairs.push_back(std::make_pair(param.first, real_second));
				
			}
		}

		// Initialize subsystems
		create_global_logger();

		logger->info("Starting OSP with settings = \"{}\", resource path = \"{}\", user data path = \"{}\"", settings_path, res_path, udata_path);
		
		// Load settings
		std::string settings_file = AssetManager::load_string_raw(udata_path + settings_path);
		std::shared_ptr<cpptoml::table> config = SerializeUtil::load_string(settings_file);
		if(!config)
		{
			logger->fatal("Could not load settings file!");
		}

		// Override with commandline arguments
		for(auto[key, value] : toml_pairs)
		{
			// Parse the value (this method is a bit hacky, but works fine)
			const std::string fix_key = "_IKEY";
			auto n_parse = SerializeUtil::load_string(fix_key + "=" + value);
			auto value_toml = n_parse->get_qualified(fix_key);

			// Go through every space in key
			auto sub_paths = split(key.c_str(), '.');
			auto ptr = config;
			for(size_t i = 0; i < sub_paths.size(); i++)
			{
				std::string& sub_path = sub_paths[i];

				if(i == sub_paths.size() - 1)
				{
					// Insert value			
					ptr->insert(sub_path, value_toml);
				}
				else
				{
					if(ptr->get_table(sub_path))
					{
						ptr = ptr->get_table(sub_path);	
					}
					else
					{
						auto n_ptr = cpptoml::make_table();
						ptr->insert(sub_path, n_ptr);
						ptr = n_ptr;	
					}	
				}
			}	
		}

		std::cout << *config << std::endl;

		create_global_asset_manager(res_path, udata_path);
		renderer = new Renderer(*config);
		create_global_debug_drawer();
		create_global_texture_drawer();
		create_global_text_drawer();
		create_global_lua_core();
		create_global_profiler();

		// Load packages now so they register all scripts...
		assets->load_packages(lua_core, &game_database);

		input = new InputUtil();	
		input->setup(renderer->window);

		dt = 0.0;
		Timer dtt = Timer();
	}	
}

void OSP::finish()
{
	logger->info("Closing OSP");
	delete input;
	destroy_global_lua_core();
	destroy_global_text_drawer();
	destroy_global_texture_drawer();
	destroy_global_debug_drawer();
	delete renderer;
	destroy_global_asset_manager();	
	destroy_global_logger();
}

bool OSP::should_loop()
{
	return !glfwWindowShouldClose(renderer->window);
}

void OSP::start_frame()
{
	if(renderer != nullptr)
	{
		input->update(renderer->window);
		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float w = (float)renderer->get_width();
		float h = (float)renderer->get_height();
		nvgBeginFrame(renderer->vg, w, h, w / h);
	}
}

void OSP::update()
{
	game_state.update();
}

void OSP::render()
{
	if(renderer != nullptr)
	{
		game_state.render();
		// It's the responsability of the Scene to call renderer render
		renderer->do_imgui();
		renderer->finish();
	}
}

void OSP::finish_frame()
{
	double max_dt = game_state.universe.MAX_PHYSICS_STEPS * game_state.universe.PHYSICS_STEPSIZE;
	dt = dtt.restart();
	game_dt = dt;

	if(dt > max_dt)
	{
		logger->warn("Delta-time too high ({})/({}), slowing down", dt, max_dt);
		dt = max_dt;
	}
}

OSP::OSP() : game_state(this)
{

}
