#include "PlanetEditor.h"
#include <fstream>

void PlanetEditor::update(float dt, ImFont* code_font)
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New"))
			{

			}
			
			if (ImGui::MenuItem("Save"))
			{

			}

			if (ImGui::MenuItem("Save As"))
			{

			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Script Editor"))
			{
				show_script_editor = !show_script_editor;
			}

			if (ImGui::MenuItem("Config Editor (Text)"))
			{
				show_config_editor = !show_config_editor;
			}

			if (ImGui::MenuItem("QuadTree Planet"))
			{
				show_planet_info = !show_planet_info;
			}

			if (ImGui::MenuItem("Reset Camera"))
			{
				camera.reset(mesher_info.radius);
			}

			if (ImGui::MenuItem("Wireframe"))
			{
				wireframe = !wireframe;
			}

			ImGui::EndMenu();
		}

		std::string str = "Speed: " + std::to_string(camera.speed) + " m/s";
		ImGui::MenuItem(str.c_str());
		std::string alt = "Altitude: " + std::to_string(altitude) + " m";
		ImGui::MenuItem(alt.c_str());

		std::string salt = "Sea Altitude: " + std::to_string(glm::length(camera.pos) - mesher_info.radius) + " m";
		ImGui::MenuItem(salt.c_str());

		ImGui::EndMainMenuBar();
	}

	
	if (script_watch.has_changed())
	{
		on_script_file_change();
	}

	if (config_watch.has_changed())
	{
		on_config_file_change();
	}

	server->update(planet);
	planet.dirty = false;

	planet.update(*server);

	if (show_script_editor)
	{
		do_editor_window(code_font, &script_editor, "Script Editor", &show_script_editor);
	}

	if (show_config_editor)
	{
		do_editor_window(code_font, &toml_editor, "Config Editor", &show_config_editor);
	}

	if (show_planet_info)
	{
		do_planet_window();
	}

	

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		if (!last_frame_save)
		{
			// Save to file
			std::ofstream to_file = std::ofstream(path);
			to_file << script_editor.GetText();

			std::ofstream to_config_file = std::ofstream(config_path);
			to_config_file << toml_editor.GetText();


			last_frame_save = true;
		}
	}
	else
	{
		last_frame_save = false;
	}


	if (!ImGui::IsAnyItemFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsAnyWindowFocused())
	{
		bool moved = false;

		// Motion
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			camera.forwards(dt);
			moved = true;
		}

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			camera.backwards(dt);
			moved = true;
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			camera.leftwards(dt);
			moved = true;
		}

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			camera.rightwards(dt);
			moved = true;
		}

		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		{
			camera.upwards(dt);
			moved = true;
		}

		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		{
			camera.downwards(dt);
			moved = true;
		}

		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			camera.tilt(dt, -1.0f);
		}

		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		{
			camera.tilt(dt, 1.0f);
		}

		if (moved)
		{
			on_move();
		}

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2))
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			if (input->mouse_delta != glm::dvec2(0.0, 0.0))
			{
				camera.mouse(input->mouse_delta, dt);
			}

			if (input->mouse_scroll_delta != 0)
			{
				camera.speed += camera.speed * input->mouse_scroll_delta * 0.05;
			}
		}
		else
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

	}
	else
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	
}

void PlanetEditor::render(int width, int height)
{
	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// ~1 light year
	float far_plane = 10e16f;

	glm::mat4 proj = glm::perspective(glm::radians(80.0f), (float)width / (float)height, 0.1f, far_plane);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), (glm::vec3)camera.forward, (glm::vec3)camera.up);

	// We apply the view position to the world so floating point errors happen
	// far away, and not near the camera
	glm::dmat4 model = glm::translate(glm::dmat4(1.0f), -camera.pos);
	model = glm::scale(model, glm::dvec3(mesher_info.radius, mesher_info.radius, mesher_info.radius));

	glm::dmat4 proj_view = (glm::dmat4)proj * (glm::dmat4)view;

	glm::vec3 cam_pos_relative = (glm::vec3)(camera.pos / mesher_info.atmo_radius);

	glm::dmat4 amodel = glm::translate(glm::dmat4(1.0f), -camera.pos);
	amodel = glm::scale(amodel, glm::dvec3(mesher_info.atmo_radius, mesher_info.atmo_radius, mesher_info.atmo_radius));
	float rel_radius = (float)(mesher_info.radius / mesher_info.atmo_radius);

	if (mesher_info.atmo_radius > 0 )//&& glm::length(cam_pos_relative) <= 1.0f)
	{
		atmo_renderer.do_pass(proj_view, amodel, far_plane, rel_radius, cam_pos_relative);
	}

	renderer.render(*server, planet, proj_view, model, far_plane, camera.pos, mesher_info.has_water, mesher_info.radius,
		mesher_info.atmo_radius, glfwGetTime());

	/*if (mesher_info.atmo_radius > 0 && glm::length(cam_pos_relative) > 1.0f)
	{
		atmo_renderer.do_pass(proj_view, amodel, far_plane, rel_radius, cam_pos_relative);
	}*/


}

void PlanetEditor::on_script_file_change()
{
	delete server;
	bool exists = false;

	{
		std::ifstream checker = std::ifstream(path);
		exists = checker.good();
	}

	if (!exists)
	{
		logger->warn("Script file did not exist, copying default");

		std::filesystem::path script_path = path;
		script_path.remove_filename();

		std::filesystem::create_directories(script_path);

		std::ifstream src("./res/default_planet.lua", std::ios::binary);
		std::ofstream dst(path, std::ios::binary);

		dst << src.rdbuf();

	}
	script_loaded = assets->loadString(path);
	server = new PlanetTileServer(script_loaded, &mesher_info, mesher_info.seed, mesher_info.interp, 
		mesher_info.has_water);


	planet.dirty = true;

	script_editor.SetText(script_loaded);
}

void PlanetEditor::on_config_file_change()
{

	SerializeUtil::read_file_to(config_path, mesher_info, "mesher_info");

	if (mesher_info.script_path != path)
	{
		path = mesher_info.script_path;
	}

	config_loaded = assets->loadString(config_path);
	toml_editor.SetText(config_loaded);

	on_script_file_change();
}

PlanetEditor::PlanetEditor(GLFWwindow* window, const std::string& planet_name)
{

	this->window = window;
	last_frame_save = false;


	config_path = "./res/planets/" + planet_name + "/config.toml";

	// Copy default config if file does not exists
	std::ifstream f2(config_path);
	if (!f2.good())
	{
		std::filesystem::path cpath = config_path;
		cpath.remove_filename();

		std::filesystem::create_directories(cpath);

		std::ifstream src("./res/default_planet.toml", std::ios::binary);
		std::ofstream dst(config_path, std::ios::binary);

		dst << src.rdbuf();
	}


	SerializeUtil::read_file_to(config_path, mesher_info, "mesher_info");

	if (mesher_info.script_path == "$$NEW_SCRIPT$$")
	{
		path = "./res/planets/" + planet_name + "/surface.lua";
		mesher_info.script_path = path;
		SerializeUtil::write_to(config_path, mesher_info, "mesher_info");
	}
	else
	{
		path = mesher_info.script_path;
	}

	config_loaded = assets->loadString(config_path);

	on_script_file_change();



	script_loaded = assets->loadString(path);
	server = new PlanetTileServer(script_loaded, &mesher_info, 0, 0, mesher_info.has_water);

	

	// Load custom theme
	const static TextEditor::Palette pallete = 
	{ {
			0xffcbcbcb,	// Default
			0xff756cd4,	// Keyword	
			0xffdd78c6,	// Number
			0xff56b4e5,	// String
			0xff56b4e5, // Char literal
			0xffcbcbcb, // Punctuation
			0xff408080,	// Preprocessor
			0xff79c398, // Identifier
			0xff79c398, // Known identifier
			0xff79c398, // Preproc identifier
			0xff79685f, // Comment (single line)
			0xff79685f, // Comment (multi line)
			0xff342c28, // Background
			0xffe0e0e0, // Cursor
			0x80a06020, // Selection
			0x800020ff, // ErrorMarker
			0x40f08000, // Breakpoint
			0xff79685f, // Line number
			0x10ffffff, // Current line fill
			0x10808080, // Current line fill (inactive)
			0x10a0a0a0, // Current line edge
	} };

	// Setup editors
	auto script_lang = TextEditor::LanguageDefinition::Lua();
	script_editor.SetLanguageDefinition(script_lang);

	script_editor.SetText(script_loaded);
	script_editor.SetShowWhitespaces(false);
	script_editor.SetPalette(pallete); 

	auto config_lang = TextEditor::LanguageDefinition::TOML();
	toml_editor.SetLanguageDefinition(config_lang);

	toml_editor.SetText(config_loaded);
	toml_editor.SetShowWhitespaces(false);
	toml_editor.SetPalette(pallete);

	script_watch = FileWatcher(path);
	config_watch = FileWatcher(config_path);

	camera.reset(mesher_info.radius);

	on_move();

}


PlanetEditor::~PlanetEditor()
{
	delete server;
}

void PlanetEditor::do_editor_window(ImFont* code_font, TextEditor* editor, const std::string& name, bool* open)
{
	ImGui::Begin(name.c_str(), open);

	ImGui::PushFont(code_font);

	editor->Render((name + "EDIT").c_str());
	
	ImGui::PopFont();

	ImGui::End();
}


void PlanetEditor::do_planet_window()
{
	if (ImGui::Begin("Planet", &show_planet_info))
	{



		if (ImGui::CollapsingHeader("Quadtree", ImGuiTreeNodeFlags_DefaultOpen))
		{
			planet.do_imgui(*server);
		}

		if (ImGui::CollapsingHeader("Tile Server"))
		{
			server->do_imgui();
		}

	}
	ImGui::End();
}

void PlanetEditor::on_move()
{
	glm::vec3 pos_nrm = (glm::vec3)glm::normalize(camera.pos);
	PlanetSide side = planet.get_planet_side(pos_nrm);
	glm::dvec2 offset = planet.get_planet_side_offset(pos_nrm, side);

	double height = std::max(glm::length(camera.pos) - mesher_info.radius - server->get_height(pos_nrm, 1), 1.0);
	altitude = height;
	height /= mesher_info.radius;
	double depthf = (mesher_info.coef_a - (mesher_info.coef_a * glm::log(height) 
		/ ((glm::pow(height, 0.15) * mesher_info.coef_b))) - 0.3 * height) * 0.4;

	depth = (size_t)round(std::max(std::min(depthf, (double)mesher_info.max_depth - 1.0), -1.0) + 1.0);

	planet.set_wanted_subdivide(offset, side, depth);

}
