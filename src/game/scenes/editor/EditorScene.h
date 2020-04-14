#pragma once
#include "../Scene.h"
#include <nanovg/nanovg.h>
#include "EditorCamera.h"
#include "EditorGUI.h"

class EditorScene : public Scene
{
private:

	EditorCamera cam;
	EditorGUI gui;

	void do_gui();

public:

	virtual void load() override;
	virtual void update() override;
	virtual void render() override;
	virtual void unload() override;
};
