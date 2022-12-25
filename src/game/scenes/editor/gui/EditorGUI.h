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
#include "WiringPanel.h"


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
		WIRING_PANEL,
		TRASHCAN
	};

	EditorScene* scene;
	ShowPanel show_panel;
	EditorMode editor_mode;
	GUIImageButton* current_editor_mode_button;

	NVGcontext* vg;

	EditorPartList part_list;
	EditorTrashcan trashcan;
	WiringPanel wiring;
	PlumbingPanel plumbing;

	GUICanvas toolset_canvas;
	GUICanvas file_canvas;

	// To be called everyframe so canvas are drawn
	void add_canvas(int width, int height);

	void add_gui(int width, int height);
	void add_toolset(int width, int height, float swidth);
	void add_file(int width, int height);

	// Call this before any canvas are drawn
	void do_backgrounds(int width, int height);

	void init(EditorScene* scene);

	void set_editor_mode(EditorMode mode);


	int get_panel_width();

};
