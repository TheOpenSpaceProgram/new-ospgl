#pragma once
#include "BaseInterface.h"
#include <assets/BitmapFont.h>
#include <gui/GUIWindow.h>

class EditorVehicleInterface;

class WireInterface : public BaseInterface
{
private:
	EditorVehicle* edveh;
	EditorScene* scene;
	EditorVehicleInterface* edveh_int;

	std::vector<std::pair<Part*, std::vector<Machine*>>> visible_machines;
	// We story a copy for the GUI drawing
	CameraUniforms cu;

	AssetHandle<BitmapFont> tiny_font;

	Machine* hovered;
	Machine* selected;
	std::vector<Machine*> selected_wired;

	void see_part(Part* p);

	template <typename F>
	// Type of lambda F:
	// bool (was visible) F(Machine* m, glm::dvec2 final_pos, glm::dvec3 pos, bool in_front)
	// Extracted from the code to make it usable from other interfaces, but it's
	// a bit of code smell!
	static void draw_machine_polygon(NVGcontext* vg, Part* part, std::vector<Machine*> machines,
							  int* rnd_idx, glm::dvec4 vport, const CameraUniforms& cu, F&& lambda);


public:

	bool show_hidden;

	virtual void update(double dt) override;

	virtual void leave() override;

	virtual bool can_leave() override;
	
	virtual bool do_interface(const CameraUniforms& cu, glm::dvec3 ray_start, glm::dvec3 ray_end,
			glm::dvec4 viewport, NVGcontext* vg, GUIInput* gui_input, GUISkin* gui_skin) override;
	
	WireInterface(EditorVehicleInterface* edveh_int);

	bool do_machine(NVGcontext *vg, GUIInput *gui_input, Machine *m, glm::vec2 final_pos, glm::dvec3 pos,
				 glm::ivec4 vport, bool in_front, std::vector<glm::vec2> &seen_positions, int rnd_idx, Machine *&new_wire,
				 std::unordered_map<Machine *, std::pair<glm::vec2, bool>>& machine_to_pos,
				 bool mouse_blocked);
};
