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
	AssetHandle<Image> clip_front;
	AssetHandle<Image> clip_back;



	Machine* hovered;
	Machine* selected;
	std::vector<Machine*> selected_wired;

	void see_part(Part* p);



public:

	LogicalGroup* editing;

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

// Type of lambda F:
// bool (was visible) F(Machine* m, glm::dvec2 final_pos, glm::dvec3 pos, bool in_front)
// Extracted from the code to make it usable from other interfaces, but it's
// a bit of code smell!
// TODO: Clean this mess up a bit (create an external class with all this functionality)
template <typename F>
static void draw_machine_polygon(NVGcontext* vg, const Part* p, std::vector<Machine*> machines,
								 int* rnd_idx, glm::dvec4 vport, const CameraUniforms& cu, F&& lambda)
{

	std::vector<Machine *> machines_with_marker;

	// Remove machines with marker and add them to the other array
	for (auto it = machines.begin(); it != machines.end();)
	{
		Machine *m = *it;
		if (m->editor_location_marker.empty())
		{
			it++;
		}
		else
		{
			machines_with_marker.push_back(m);
			it = machines.erase(it);
		}
	}

	glm::dvec3 pos = to_dvec3(p->get_piece("p_root")->packed_tform.getOrigin());
	// Draw the polygon with the machines on its edges
	// and a central "indicator"
	int polygon_machines = machines.size();

	float dangle = glm::two_pi<float>() / polygon_machines;

	float scale = glm::clamp(200.0f / ((float) glm::distance2(pos, cu.cam_pos)), 0.5f, 1.0f);
	float real_icon_size = 22.0f;
	float icon_size = real_icon_size * scale;

	// This formula gives appropiate icon distancing
	float radius = glm::max((float) log(polygon_machines) * icon_size * scale * 0.6f, icon_size);
	if (polygon_machines == 1)
	{
		radius = 0.0f;
	}

	auto [clip, in_front] = MathUtil::world_to_clip(cu.tform, pos);
	glm::vec2 screen_pos = glm::round(MathUtil::clip_to_screen(clip, vport));

	// Indicator
	if (polygon_machines > 1 && in_front)
	{
		nvgBeginPath(vg);
		nvgCircle(vg, screen_pos.x, screen_pos.y, 4.0f);
		nvgFillColor(vg, nvgRGB(0, 0, 0));
		nvgFill(vg);
	}

	float angle = 0.0f;
	int i = 0;
	for (Machine *m: machines)
	{
		float adj_angle = angle + glm::half_pi<float>();
		glm::vec2 offset = glm::vec2(sin(adj_angle), cos(adj_angle));
		glm::vec2 final_pos = glm::round(screen_pos + offset * radius);

		bool was_visible = lambda(m, final_pos, pos, in_front);

		// Indicator
		if (was_visible && polygon_machines != 1)
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, screen_pos.x, screen_pos.y);
			nvgLineTo(vg, round(screen_pos.x + offset.x * 4.0f), round(screen_pos.y + offset.y * 4.0f));
			nvgStrokeColor(vg, nvgRGB(255, 255, 255));
			nvgStrokeWidth(vg, 2.0f);
			nvgStroke(vg);
		}

		angle += dangle;
		i++;
	}

	// Draw the machines at a marker location
	for (Machine *m: machines_with_marker)
	{

		glm::dvec3 mpos = p->get_piece("p_root")->get_marker_position(m->editor_location_marker);
		mpos = p->get_piece("p_root")->get_graphics_matrix() * glm::dvec4(mpos, 1.0);
		auto [clip, in_front] = MathUtil::world_to_clip(cu.tform, mpos);
		glm::vec2 mscreen_pos = glm::round(MathUtil::clip_to_screen(clip, vport));

		lambda(m, mscreen_pos, pos, in_front);
	}

	rnd_idx++;
}
