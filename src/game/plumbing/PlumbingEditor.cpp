#include "PlumbingEditor.h"
#include <lua/libs/LuaNanoVG.h>

void PlumbingEditor::show_editor(NVGcontext* vg, GUIInput* gui_input, glm::vec4 span)
{
	if(veh == nullptr)
	{
		logger->warn("Plumbing editor shown without assigned vehicle");
		return;
	}

	bool block = false;
	block |= update_mouse(gui_input, span);
	block |= update_selection(gui_input, span);

	if(block)
	{
		gui_input->ext_mouse_blocked = true;
		gui_input->ext_scroll_blocked = true;
		gui_input->mouse_blocked = true;
		gui_input->scroll_blocked = true;
	}

	nvgSave(vg);
	nvgScissor(vg, span.x, span.y, span.z, span.w);

	nvgStrokeColor(vg, nvgRGB(230, 230, 230));
	draw_grid(vg, span);
	draw_machines(vg, span);
	draw_selection(vg, span);


	nvgRestore(vg);
}

PlumbingEditor::PlumbingEditor()
{
	cam_center = glm::vec2(0.0f, 0.0f);
	zoom = 32;
	veh = nullptr;
	in_drag = false;
	in_selection = false;
}

void PlumbingEditor::draw_grid(NVGcontext* vg, glm::vec4 span) const
{

	nvgBeginPath(vg);

	int max_halfx = (int)((span.z / 2.0f) / (float)zoom);
	int max_halfy = (int)((span.w / 2.0f) / (float)zoom);

	nvgResetTransform(vg);
	glm::vec2 center = glm::vec2(span.x + span.z * 0.5f, span.y + span.w * 0.5f);
	nvgTranslate(vg, center.x-fmod(cam_center.x, 1.0f) *zoom, center.y-fmod(cam_center.y, 1.0f) *zoom);
	nvgScale(vg, (float)zoom, (float)zoom);
	nvgStrokeWidth(vg, 1.0f / (float)zoom);
	// Vertical lines
	for(int x = -max_halfx - 1; x < max_halfx + 1; x++)
	{
		nvgMoveTo(vg, (float)x, -(float)max_halfy - 1);
		nvgLineTo(vg, (float)x, (float)max_halfy + 1);
	}

	for(int y = -max_halfy - 1; y < max_halfy + 1; y++)
	{
		nvgMoveTo(vg, -(float)max_halfx - 1, (float)y);
		nvgLineTo(vg, (float)max_halfx + 1, (float)y);
	}
	nvgStroke(vg);
}


void PlumbingEditor::draw_machines(NVGcontext* vg, glm::vec4 span)
{
	nvgResetTransform(vg);

	glm::vec2 center = glm::vec2(span.x + span.z * 0.5f, span.y + span.w * 0.5f);

	int i = 0;

	for(const Part* p : veh->parts)
	{
		for(const auto& pair : p->machines)
		{
			if(pair.second->plumbing.has_lua_plumbing())
			{
				bool is_selected = false;
				bool is_conflict = false;
				for(Machine* m : selected)
				{
					if(m == pair.second)
					{
						is_selected = true;
						break;
					}
				}

				for(Machine* m : drag_conflicts)
				{
					if(m == pair.second)
					{
						is_conflict = true;
						break;
					}
				}

				nvgStrokeWidth(vg, 2.0f / (float)zoom);
				nvgStrokeColor(vg, nvgRGB(0, 0, 0));
				nvgFillColor(vg, nvgRGB(255, 255, 255));

				if(pair.second == hovered || is_selected)
				{
					nvgStrokeWidth(vg, 4.0f / (float)zoom);
				}
				if(is_selected)
				{
					nvgFillColor(vg, nvgRGB(128, 128, 255));
				}

				if(is_conflict)
				{
					nvgStrokeColor(vg, nvgRGB(255, 0, 0));
				}

				nvgResetTransform(vg);
				glm::vec2 ppos = pair.second->plumbing.editor_position;

				// Preview drag
				if(in_machine_drag && is_selected)
				{
					glm::vec2 offset = mouse_current - mouse_start;
					ppos += glm::round(offset);
				}

				nvgTranslate(vg, center.x + (ppos.x - cam_center.x) * (float)zoom,
				 	center.y + (ppos.y - cam_center.y) * (float)zoom);
				nvgScale(vg, (float)zoom, (float)zoom);

				pair.second->plumbing.draw_diagram((void*)vg);

				// Draw the ports
				for(const FluidPort& port : pair.second->plumbing.fluid_ports)
				{
					nvgFillColor(vg, nvgRGB(0, 0, 0));
					nvgBeginPath(vg);
					nvgCircle(vg, port.pos.x, port.pos.y, 0.15f);
					nvgFill(vg);
				}
			}
		}
		i += 3;
	}

}

bool PlumbingEditor::update_mouse(GUIInput* gui_input, glm::vec4 span)
{
	bool block = false;
	bool inside_and_not_blocked = is_inside_and_not_blocked(gui_input, span);

	// Click and drag
	if(inside_and_not_blocked || in_drag)
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
			block = true;
		}

	}

	// Zooming in and out
	if(inside_and_not_blocked)
	{
		block = true;
		double delta = input->mouse_scroll_delta;
		zoom += zoom * delta * 0.1;
		zoom = glm::clamp(zoom, 16, 128);
	}

	return block;
}

bool PlumbingEditor::update_dragging(GUIInput *gui_input, glm::vec2 mpos)
{
	if(in_machine_drag)
	{
		mouse_current = mpos;
		glm::vec2 offset = glm::round(mouse_current - mouse_start);

		drag_conflicts.clear();
		for(Machine* m : selected)
		{
			glm::ivec2 end_pos = m->plumbing.editor_position + glm::ivec2(offset);
			if(!veh->plumbing.grid_aabb_check(end_pos, end_pos + m->plumbing.get_editor_size(), selected).empty())
			{
				drag_conflicts.push_back(m);
			}
		}


		if(input->mouse_up(0))
		{
			if(drag_conflicts.empty())
			{
				for (Machine *m : selected)
				{
					m->plumbing.editor_position += glm::ivec2(offset);
				}
			}

			in_machine_drag = false;
			drag_conflicts.clear();
		}
		return true;
	}
	else
	{
		bool hovered_selected = false;
		for (Machine *m : selected)
		{
			if (hovered == m)
			{
				hovered_selected = true;
				break;
			}
		}

		if (gui_input->mouse_down(0) && hovered_selected)
		{
			in_machine_drag = true;
			mouse_start = mpos;
			mouse_current = mpos;
			return true;
		}

		return false;
	}
}

bool PlumbingEditor::update_selection(GUIInput *gui_input, glm::vec4 span)
{
	bool block = false;
	bool dragged = false;
	bool inside_and_not_blocked = is_inside_and_not_blocked(gui_input, span);

	glm::vec2 mpos = get_mouse_pos(span);

	// Hovering
	hovered = nullptr;

	auto hover_vec = veh->plumbing.grid_aabb_check(mpos, mpos);
	if(hover_vec.size() >= 1)
	{
		hovered = hover_vec[0];
	}

	if(!in_selection && inside_and_not_blocked && !in_drag || in_machine_drag)
	{
		dragged = update_dragging(gui_input, mpos);
		block |= dragged;
	}

	// Left click + drag does box selection and unselects everything else
	// Shift + Left click adds to selection with box

	if(!in_drag && !in_selection && inside_and_not_blocked && !dragged)
	{
		if(gui_input->mouse_down(0))
		{
			in_selection = true;
			mouse_start = mpos;
		}
	}

	if(in_selection)
	{
		mouse_current = mpos;
		if(gui_input->mouse_up(0))
		{
			if(!input->key_pressed(GLFW_KEY_LEFT_SHIFT) && !input->key_pressed(GLFW_KEY_RIGHT_SHIFT))
			{
				selected.clear();
			}

			// AABB check
			if(mouse_start.x > mouse_current.x)
			{
				std::swap(mouse_start.x, mouse_current.x);
			}
			if(mouse_start.y > mouse_current.y)
			{
				std::swap(mouse_start.y, mouse_current.y);
			}
			auto new_sel = veh->plumbing.grid_aabb_check(mouse_start, mouse_current, selected);
			selected.insert(selected.end() , new_sel.begin(), new_sel.end());

			in_selection = false;
		}
	}

	return block;
}

bool PlumbingEditor::is_inside_and_not_blocked(GUIInput *gui_input, glm::vec4 span)
{
	return gui_input->mouse_inside(span) &&
		!gui_input->mouse_blocked && !gui_input->ext_mouse_blocked &&
		!gui_input->scroll_blocked && !gui_input->ext_scroll_blocked;
}


glm::vec2 PlumbingEditor::get_mouse_pos(glm::vec4 span) const
{
	glm::vec2 window_relative = input->mouse_pos;
	glm::vec2 span_relative;
	span_relative.x = (window_relative.x - span.x) / span.z;
	span_relative.y = (window_relative.y - span.y) / span.w;
	glm::vec2 center_relative = span_relative - glm::vec2(0.5f, 0.5f);
	center_relative.x *= span.z;
	center_relative.y *= span.w;

	return cam_center + center_relative / (float)zoom;

}

void PlumbingEditor::draw_selection(NVGcontext *vg, glm::vec4 span) const
{
	if(in_selection)
	{
		nvgResetTransform(vg);
		glm::vec2 center = glm::vec2(span.x + span.z * 0.5f, span.y + span.w * 0.5f);
		nvgTranslate(vg, center.x - cam_center.x * (float) zoom, center.y - cam_center.y * (float) zoom);
		nvgScale(vg, (float) zoom, (float) zoom);

		nvgStrokeColor(vg, nvgRGB(0, 0, 255));
		nvgFillColor(vg, nvgRGBA(0, 0, 255, 100));
		nvgStrokeWidth(vg, 1.0f / (float)zoom);

		nvgBeginPath(vg);
		nvgRect(vg, mouse_start.x, mouse_start.y,
				mouse_current.x - mouse_start.x, mouse_current.y - mouse_start.y);
		nvgStroke(vg);
		nvgFill(vg);
	}
}

