#pragma once
#include <gui/GUIWindow.h>

class EditorPartList;
class EditorScene;

class GroupManager
{
private:
	EditorScene* scene;
	EditorPartList* pl;
	std::weak_ptr<GUIWindow> win;
	int selected_group;
	void create_right_panel();

public:

	void try_show();

	void init(EditorScene* nsc, EditorPartList* pl);

};
