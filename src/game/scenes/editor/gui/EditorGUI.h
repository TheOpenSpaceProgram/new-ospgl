#pragma once
#include <nanovg/nanovg.h>
#include <gui/GUIInput.h>

#include <assets/PartPrototype.h>
#include <assets/Image.h>
#include <assets/AssetManager.h>
#include <gui/skins/SimpleSkin.h>
#include "../EditorVehicleInterface.h"

#include "EditorPartList.h"
#include "EditorTrashcan.h"


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

	void prepare_toolset();
	void prepare_file();

public:

	friend class EditorTrashcan;
	friend class EditorPartList;

	SimpleSkin skin;

	NVGcontext* vg;

	EditorPartList part_list;
	EditorTrashcan trashcan;

	GUICanvas toolset_canvas;
	GUICanvas file_canvas;

	void do_gui(int width, int height, GUIInput* gui_input);	
	void do_toolset(int width, int height, float swidth, GUIInput* gui_input);
	void do_file(int width, int height, GUIInput* gui_input);

	void init(EditorScene* scene);


	int get_panel_width();

};
