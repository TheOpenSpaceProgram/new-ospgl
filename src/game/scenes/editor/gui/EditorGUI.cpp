#include "EditorGUI.h"
#include <glm/glm.hpp>

#include <gui/layouts/GUIVerticalLayout.h>
#include <gui/layouts/GUIListLayout.h>
#include <gui/widgets/GUIImageButton.h>
#include <gui/widgets/GUITextField.h>

#include "../EditorScene.h"
#include <OSP.h>


int EditorGUI::get_panel_width()
{
	return part_list.get_panel_width();
}

	void EditorGUI::init(EditorScene* sc)
	{
		this->scene = sc;
		prev_width = 0;
		prev_height = 0;
		// We hold a pointer to interact with the 3D editor
		this->edveh_int = &sc->vehicle_int;

		part_list.init(sc, vg);
		trashcan.init(sc, vg);
		plumbing.init(sc, vg);

		create_toolset();
		create_file();
	}

	void EditorGUI::set_editor_mode(EditorMode mode)
	{
		editor_mode = mode;

		edveh_int->current_interface->leave();
		switch(mode)
		{
			case ATTACHING:
				edveh_int->current_interface = &edveh_int->attach_interface;
				break;
			case WIRING:
				edveh_int->current_interface = &edveh_int->wire_interface;
				break;
			case PLUMBING:
				edveh_int->current_interface = &edveh_int->plumbing_interface;
				break;
		}
	}

	void EditorGUI::create_toolset()
	{
		// Prepare the toolset_canvas
		auto tlayout = std::make_shared<GUIListLayout>();
		tlayout->vscrollbar.draw = false;
		toolset_canvas.layout = tlayout;

		auto create_button = [tlayout, this](const std::string& name, EditorMode mode)
		{
			auto button = std::make_shared<GUIImageButton>();
			button->set_image(vg, AssetHandle<Image>("core", name));
			button->default_size = glm::ivec2(24, 24);
			tlayout->add_widget(button);

			button->on_clicked.add_handler([this, button, mode](int btn)
			{
				if(this->edveh_int->can_change_editor_mode())
				{
					this->current_editor_mode_button->toggled = false;
					this->current_editor_mode_button = button.get();
					button->toggled = true;
					this->set_editor_mode(mode);
				}
			});

			return button;
		};

		editor_mode = ATTACHING;
		current_editor_mode_button = create_button("editor/attach.png", ATTACHING).get();
		current_editor_mode_button->toggled = true;
		edveh_int->current_interface = &edveh_int->attach_interface;

		create_button("editor/transform.png", TRANSFORMING);
		create_button("editor/wire.png", WIRING);
		create_button("editor/duct.png", PLUMBING);
		create_button("editor/electric.png", ELECTRIC_WIRING);
	}

	void EditorGUI::create_file()
	{
		// It doesn't matter as we later on set the fixed pixel ammount
		auto pair = file_canvas.divide_h(0.5f);
		auto left = pair.first;
		auto right = pair.second;

		left->layout = std::make_shared<GUISingleLayout>();

		auto list = std::make_shared<GUIListLayout>(0, 2);
		list->vscrollbar.draw = false;
		list->margins.x = 1; list->margins.y = 1;
		right->layout = list;

		// Left contains just the text field
		auto text_field = std::make_shared<GUITextField>();
		left->layout->add_widget(text_field);

		// Right contains the small buttons

		auto create_button = [right, this](const std::string& name, std::function<void(int)> on_click)
		{
			auto button = std::make_shared<GUIImageButton>();
			button->set_image(vg, AssetHandle<Image>("core", name));
			button->on_clicked.add_handler(on_click);
			button->default_size = glm::ivec2(22, 22);
			button->img_mode = GUIImageButton::CENTER;
			right->layout->add_widget(button);
		};

		create_button("editor/new.png", [](int but)
		{

		});
		create_button("editor/load.png", [](int but)
		{

		});
		create_button("editor/save.png", [](int but)
		{

		});
		create_button("editor/launch.png", [](int but)
		{

		});
		create_button("editor/quit.png", [](int but)
		{

		});
	}

	void EditorGUI::add_gui(int width, int height)
{

}

void EditorGUI::add_toolset(int width, int height, float swidth)
{
	float twidth = 152.0f;
	float real_theight = 30.0f;
	glm::ivec4 screen = glm::ivec4(0, 0, width, height);
	scene->gui_screen.add_canvas(&toolset_canvas, glm::ivec2(swidth + 4, 0), glm::ivec2(twidth, real_theight));
}

void EditorGUI::add_file(int width, int height)
{
	float fwidth = 380.0f;
	float fheight = 30.0f;
	glm::ivec4 screen = glm::ivec4(0, 0, width, height);
	scene->gui_screen.add_canvas(&file_canvas, glm::ivec2(width - fwidth, 0), glm::ivec2(fwidth, fheight));
}

void EditorGUI::do_backgrounds(int width, int height)
{
	float swidth = (float)get_panel_width();

	nvgResetScissor(vg);

	nvgFillColor(vg, scene->gui_screen.skin->get_background_color());

	// Panel background
	nvgBeginPath(vg);
	nvgRect(vg, 0.0f, 0.0f, swidth, 1.0f * height);
	nvgFill(vg);

	// File canvas background
	float fwidth = 380.0f;
	float fheight = 30.0f;
	nvgBeginPath(vg);
	nvgMoveTo(vg, width, 0.0f);
	nvgLineTo(vg, width - fwidth - fheight, 0.0f);
	nvgLineTo(vg, width - fwidth, fheight);
	nvgLineTo(vg, width, fheight);
	nvgLineTo(vg, width, 0.0f);
	nvgFillColor(vg, scene->gui_screen.skin->get_background_color());
	nvgFill(vg);

	file_canvas.child_pixels = fwidth - 26.0f * 5.0f;
	file_canvas.pixels_for_child_1 = false;

	float twidth = 152.0f;
	float theight = 23.0f;

	nvgBeginPath(vg);
	nvgMoveTo(vg, swidth, 0.0f);
	nvgLineTo(vg, swidth + twidth + theight, 0.0f);
	nvgLineTo(vg, swidth + twidth, theight);
	nvgLineTo(vg, swidth, theight);
	nvgLineTo(vg, swidth, 0.0f);
	nvgFillColor(vg, scene->gui_screen.skin->get_background_color());
	nvgFill(vg);

	// Wiring mode indicator
	if(editor_mode == EditorMode::ATTACHING || editor_mode == EditorMode::PLUMBING)
	{
		nvgFontFace(vg, "regular");
		nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);
		// Calculate bounds for black background
		float xpos = swidth + 8.0f;
		float ypos = theight + 20.0f;
		std::string str = osp->game_database->get_string("core:editor_auto_wire") + ": ";
		float bounds[4];
		nvgTextBounds(vg, xpos, ypos, str.c_str(), nullptr, bounds);
		// Black background
		nvgFillColor(vg, nvgRGBA(0, 0, 0, 140));
		nvgBeginPath(vg);
		nvgRect(vg, bounds[0] - 5.0f, bounds[1] - 5.0f, bounds[2] - bounds[0] + 10.0f, bounds[3] - bounds[1] + 10.0f);
		nvgFill(vg);
		nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
		nvgText(vg, xpos, ypos, str.c_str(), nullptr);
	}
}

void EditorGUI::add_canvas(int width, int height)
{
	float swidth = (float)get_panel_width();

	if(editor_mode == ATTACHING)
	{
		if(edveh_int->attach_interface.selected == nullptr)
		{
			show_panel = PART_LIST;
		}
		else
		{
			show_panel = TRASHCAN;
			trashcan.add_gui(width, get_panel_width(), height, &scene->gui_screen);
		}
	}
	else if(editor_mode == PLUMBING)
	{
		show_panel = PLUMBING_PANEL;
	}
	else if(editor_mode == WIRING)
	{
		show_panel = WIRING_PANEL;
	}

	if(show_panel == PART_LIST)
	{
		part_list.add_gui(width, get_panel_width(), height, &scene->gui_screen);
	}
	else if(show_panel == TRASHCAN)
	{
		trashcan.add_gui(width, get_panel_width(), height, &scene->gui_screen);
	}
	else if(show_panel == PLUMBING_PANEL)
	{
		plumbing.add_gui(width, get_panel_width(), height, &scene->gui_screen);
	}
	else if(show_panel == WIRING_PANEL)
	{
		wiring.add_gui(width, get_panel_width(), height, &scene->gui_screen);
	}


	add_gui(width, height);
	add_toolset(width, height, swidth);
	add_file(width, height);
}


