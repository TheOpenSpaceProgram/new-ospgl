#pragma once
#include <nanovg/nanovg.h>
#include <gui/GUIInput.h>

#include <assets/PartPrototype.h>
#include <assets/Image.h>
#include <assets/AssetManager.h>
#include <gui/skins/SimpleSkin.h>
#include <gui/widgets/GUIImageButton.h>
#include <gui/GUIWindowManager.h>

#include "../EditorVehicleInterface.h"

#include "EditorPartList.h"
#include "EditorTrashcan.h"
#include "PlumbingPanel.h"


class EditorScene;

// The side-pane can display a lot of stuff, but usually it displays
// the C++ implemented part list and editor controls. Parts can draw a
// custom interface there, use their context menu, or pop up a full
// blown, lua controlled, gui
class EditorGUI
{
private:

	int prev_width, prev_height;

	EditorVehicleInterface* edveh_int;
	void create_toolset();
	void create_file();

public:

	friend class EditorTrashcan;
	friend class EditorPartList;
	friend class PlumbingPanel;

	enum EditorMode
	{
		ATTACHING,
		TRANSFORMING,
		WIRING,
		PLUMBING,
		ELECTRIC_WIRING
	};

	enum ShowPanel
	{
		PART_LIST,
		PLUMBING_PANEL,
		TRASHCAN
	};

	ShowPanel show_panel;
	EditorMode editor_mode;
	GUIImageButton* current_editor_mode_button;

	SimpleSkin skin;

	NVGcontext* vg;

	EditorPartList part_list;
	EditorTrashcan trashcan;
	PlumbingPanel plumbing;

	GUICanvas toolset_canvas;
	GUICanvas file_canvas;
	GUIWindowManager window_manager;

	void prepare_gui(int width, int height, GUIInput* gui_input);
	void prepare_toolset(int width, int height, float swidth, GUIInput* gui_input);
	void prepare_file(int width, int height, GUIInput *gui_input);
	void do_gui(int width, int height);
	void do_toolset(int width, int height, float swidth);
	void do_file(int width, int height);

	void init(EditorScene* scene);

	void set_editor_mode(EditorMode mode);


	int get_panel_width();

};
