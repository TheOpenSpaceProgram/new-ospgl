#pragma once
#include "../../assets/AssetManager.h"
#include "../../renderer/PlanetaryBodyRenderer.h"

#include "../../universe/body/config/PlanetConfig.h"
#include <imgui/imgui.h>
#include <TextEditor/TextEditor.h>
#include "../../util/FileWatcher.h"
#include <GLFW/glfw3.h>
#include "EditorCamera.h"
#include "../../util/InputUtil.h"


// Allows editing of planet, featuring hot-reloading
// of lua code, and many cool features
class PlanetEditor
{
private:
	GLFWwindow* window;

	size_t depth = 0;
	float x = 0.2f;

	std::string path;
	std::string config_path;

	PlanetConfig config;
	PlanetaryBodyRenderer renderer;

	FileWatcher script_watch;
	FileWatcher config_watch;

	bool show_script_editor, show_config_editor, show_planet_info;
	bool wireframe;

	void do_editor_window(ImFont* code_font, TextEditor* editor, const std::string& name, bool* close);
	void do_planet_window();

	void on_move();

	bool last_frame_save;

	EditorCamera camera;

	double altitude;

public:

	void update(float dt, ImFont* code_font);
	void render(int width, int height);

	// Called by the file watcher
	void on_script_file_change();
	void on_config_file_change();

	PlanetEditor(GLFWwindow* window, const std::string& planet_name);
	~PlanetEditor();
};

