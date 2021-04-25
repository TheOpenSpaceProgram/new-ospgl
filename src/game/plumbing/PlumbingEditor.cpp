#include "PlumbingEditor.h"

void PlumbingEditor::show_editor(NVGcontext* vg, GUIInput* gui_input, glm::vec4 span)
{
	if(veh == nullptr)
	{
		logger->warn("Plumbing editor shown without assigned vehicle");
		return;
	}

	nvgSave(vg);
	nvgScissor(vg, span.x, span.y, span.z, span.w);

	nvgStrokeColor(vg, nvgRGB(200, 200, 200));
	draw_grid(vg, span);

	// Click and drag
	if(gui_input->mouse_inside(span) &&
		!gui_input->mouse_blocked && !gui_input->ext_mouse_blocked || in_drag)
	{
		if (gui_input->mouse_down(1))
		{
			last_click = input->mouse_pos;
			last_center = cam_center;
			in_drag = true;
		}

		if(gui_input->mouse_up(1))
		{
			in_drag = false;
		}

		if(gui_input->mouse_pressed(1))
		{
			glm::vec2 delta = input->mouse_pos - last_click;
			cam_center = last_center - delta / (float)zoom;
			gui_input->ext_mouse_blocked = true;
			gui_input->mouse_blocked = true;
		}

		double delta = gui_input->mouse_scroll_delta();
		if(delta != 0)
		{
			gui_input->scroll_blocked = true;
			zoom += zoom * delta * 0.1;
			zoom = glm::clamp(zoom, 16, 128);
		}
	}


	nvgRestore(vg);
}

PlumbingEditor::PlumbingEditor()
{
	cam_center = glm::vec2(0.0f, 0.0f);
	zoom = 32;
	veh = nullptr;
	in_drag = false;
}

void PlumbingEditor::draw_grid(NVGcontext* vg, glm::vec4 span)
{
	int min_x = (int)(span.x - fmod(cam_center.x, 1.0f) * (float)zoom);
	int min_y = (int)(span.y - fmod(cam_center.y, 1.0f) * (float)zoom);

	nvgBeginPath(vg);
	// Vertical lines
	for(int x = min_x; x < (int)(span.x + span.z); x+=zoom)
	{
		nvgMoveTo(vg, (float)x, span.y);
		nvgLineTo(vg, (float)x, span.y + span.w);
	}

	for(int y = min_y; y < (int)(span.y + span.w); y+=zoom)
	{
		nvgMoveTo(vg, span.x, (float)y);
		nvgLineTo(vg, span.x + span.z, (float)y);
	}
	nvgStroke(vg);
}
