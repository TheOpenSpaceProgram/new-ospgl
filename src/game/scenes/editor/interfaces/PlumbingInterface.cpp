#include "PlumbingInterface.h"
#include "../EditorVehicleInterface.h"
#include <nanovg/nanovg_gl.h>
#include <algorithm>
#include <glm/common.hpp>

bool PlumbingInterface::can_leave()
{
	return true;
}

void PlumbingInterface::leave()
{
}

void PlumbingInterface::update(double dt)
{

}

bool PlumbingInterface::do_interface(const CameraUniforms& cu, glm::dvec3 ray_start, glm::dvec3 ray_end,
								   glm::dvec4 viewport, NVGcontext* vg, GUIInput* gui_input, GUISkin* gui_skin)
{
	glm::vec4 span;
	span.x = (float)(viewport.x + viewport.z * view_size);
	span.y = (float)viewport.y;
	span.z = (float)(viewport.z * (1.0f - view_size));
	span.w = (float)viewport.w;

	glm::vec4 rviewport;
	rviewport.x = (float)viewport.x;
	rviewport.y = (float)viewport.y;
	rviewport.z = (float)(viewport.z * view_size);
	rviewport.w = (float)viewport.w;

	// A portion of the screen is dedicated to viewing the vehicle, the other to plumbing edition
	// We draw a background for the plumbing
	nvgBeginPath(vg);
	nvgRect(vg, span.x, span.y, span.z, span.w);
	nvgFillColor(vg, gui_skin->get_background_color());
	nvgFill(vg);

	pb_editor.prepare(gui_input, span);
	pb_editor.do_editor(vg, span, gui_skin);

	edveh->clear_meta();

	do_highlight();

	// Draw machine icons of hovered / selected parts
	std::set<Part*> all_parts;
	const PlumbingMachine* hovered = pb_editor.get_hovered();
	const std::vector<PlumbingMachine*> selected = pb_editor.get_selected();
	if(hovered)
	{
		all_parts.insert(hovered->in_machine->in_part);
	}
	for(PlumbingMachine* m : selected)
	{
		all_parts.insert(m->in_machine->in_part);
	}

	for(Part* p : all_parts)
	{
		draw_icons(vg, p, hovered, selected, cu, rviewport);
	}

	if(!gui_input->mouse_blocked)
	{
		do_3d_to_2d(gui_input, ray_start, ray_end);
	}

	return false;
}

glm::dvec4 PlumbingInterface::get_vehicle_viewport()
{
	return glm::dvec4(0.0, 0.0, (double)view_size, 1.0);
}

PlumbingInterface::PlumbingInterface(EditorVehicleInterface *edveh_int)
{
	pb_editor.veh = edveh_int->edveh->veh;
	pb_editor.allow_editing = true;
	pb_editor.allow_dragging = true;
	pb_editor.show_flow_direction = true;
	pb_editor.allow_tooltip = true;

	view_size = 0.35f;

	last_focused = nullptr;
	last_focused_i = 0;

	this->edveh = edveh_int->edveh;
	this->edveh_int = edveh_int;

	pb_editor.on_middle_click.add_handler([this](Machine* m){this->do_2d_to_3d(m);});

	tiny_font = AssetHandle<BitmapFont>("core:fonts/ProggyTiny.fnt");
}

void PlumbingInterface::do_highlight()
{
	// Highlight the pieces from the editor, selected in blue, hovered in white
	const PlumbingMachine* hovered = pb_editor.get_hovered();
	const std::vector<PlumbingMachine*> selected = pb_editor.get_selected();

	for(PlumbingMachine* elem : selected)
	{
		if(!elem->in_machine)
		{
			continue;
		}

		for (const auto &pair : elem->in_machine->in_part->pieces)
		{
			edveh->piece_meta[pair.second].highlight = glm::vec3(0.2f, 0.2f, 1.0f);
		}
	}

	if(hovered)
	{
		for (const auto &pair : hovered->in_machine->in_part->pieces)
		{
			edveh->piece_meta[pair.second].highlight = glm::vec3(1.0f, 1.0f, 1.0f);
		}
	}
}

void PlumbingInterface::do_2d_to_3d(Machine* m)
{
	glm::dvec3 pos = to_dvec3(m->in_part->get_piece("p_root")->packed_tform.getOrigin());
	edveh_int->camera->center = pos;
}

void PlumbingInterface::do_3d_to_2d(GUIInput* gui_input, glm::dvec3 ray_start, glm::dvec3 ray_end)
{
	EditorVehicleInterface::RaycastResult rresult = edveh_int->raycast(ray_start, ray_end, false);

	if(rresult.has_hit)
	{
		edveh->piece_meta[rresult.p].highlight = glm::vec3(1.0, 1.0, 1.0);

		if(gui_input->mouse_down(2) && rresult.p->part != nullptr)
		{
			if(last_focused != rresult.p->part)
			{
				last_focused_i = 0;
			}
			last_focused = rresult.p->part;

			// Focus succesive machines with plumbing on the plumbing editor
			// TODO: C++ doesn't guarantee that this iteration will always be in the same
			// order, so it could break on some weird cases or weird C++ implementations
			size_t j = 0;
			bool found_any = false;
			Machine* first_m = nullptr;
			for(const auto& pair : rresult.p->part->machines)
			{
				if(pair.second->plumbing.has_lua_plumbing())
				{
					if(j == 0)
					{
						first_m = pair.second;
					}

					if(j == last_focused_i)
					{
						focus_pb_editor(pair.second);
						found_any = true;
						last_focused_i++;
						continue;
					}

					j++;
				}
			}

			// Focus the first one if we have finished looping around parts
			if(found_any == false && first_m)
			{
				focus_pb_editor(first_m);
				last_focused_i = 1;
			}
		}
	}

}

void PlumbingInterface::focus_pb_editor(Machine *m)
{
	glm::vec2 topleft = m->plumbing.editor_position;
	glm::vec2 size = m->plumbing.get_size(false, true);
	pb_editor.cam_center = topleft + size * 0.5f;
}

void PlumbingInterface::draw_icons(NVGcontext* vg, const Part *p, const PlumbingMachine *hovered,
								   const std::vector<PlumbingMachine *> selected, const CameraUniforms &cu,
								   glm::dvec4 vport)
{
	int rnd_idx = 0;

	std::vector<Machine*> machines;
	for(auto pair : p->get_all_machines())
	{
		machines.push_back(pair.second);
	}


	// TODO: This is mostly duplicate code from do_machine, abstract it away into a separate class!
	auto lambda = [this, vg, vport, cu, &hovered, &selected](
			Machine* m, glm::dvec2 final_pos, glm::dvec3 pos, bool in_front)
	{
		double scale = glm::clamp(200.0f / ((float)glm::distance2(pos, cu.cam_pos)), 0.5f, 1.0f);
		double real_icon_size = 22.0f;
		double icon_size = real_icon_size * scale;

		if(!in_front)
		{
			glm::dvec3 rel = cu.cam_pos - pos;
			final_pos.y = rel.z * 1e10f + vport.y + vport.w * 0.5f;
			final_pos.x = 2.0f * vport.x + vport.z - final_pos.x;
		}

		final_pos = glm::clamp(final_pos, glm::dvec2(vport.x, vport.y) + glm::dvec2(real_icon_size * 0.5),
							   glm::dvec2(vport.x + vport.z, vport.y + vport.w) - glm::dvec2(real_icon_size * 0.5));
		glm::vec2 tex_pos = glm::round(final_pos + icon_size * 0.5f);
		glm::vec2 rect_pos = glm::round(final_pos - icon_size * 0.5f);

		bool is_attached = vector_contains(m->in_part->attached_machines, m);


		nvgBeginPath(vg);
		nvgCircle(vg, rect_pos.x + icon_size * 0.5f, rect_pos.y + icon_size * 0.5f, icon_size * 0.71f);
		nvgStrokeWidth(vg, 2.0f);
		if(&m->plumbing == hovered)
		{
			nvgFillColor(vg, nvgRGB(255, 255, 255));
			nvgStrokeColor(vg, nvgRGB(255, 255, 255));
		}
		else if(vector_contains(selected, &m->plumbing))
		{
			nvgFillColor(vg, nvgRGB(100, 100, 255));
			nvgStrokeColor(vg, nvgRGB(50, 50, 200));
		}
		else
		{
			nvgFillColor(vg, nvgRGBA(0, 0, 0, 0));
			nvgStrokeColor(vg, nvgRGB(0, 0, 0));
		}
		nvgFill(vg);
		nvgStroke(vg);


		// Machine icon
		AssetHandle<Image> img = m->get_icon();
		int image = nvglCreateImageFromHandleGL3(vg, img->id, img->get_width(), img->get_height(), 0);
		NVGpaint paint = nvgImagePattern(vg, tex_pos.x, tex_pos.y, icon_size, icon_size, 0.0f, image, 1.0f);
		nvgBeginPath(vg);
		nvgRect(vg, rect_pos.x, rect_pos.y, icon_size, icon_size);
		nvgFillPaint(vg, paint);
		nvgFill(vg);

		if(is_attached)
		{
			nvgFillColor(vg, nvgRGB(0, 0, 0));
			nvgBeginPath(vg);
			nvgCircle(vg, rect_pos.x + icon_size, rect_pos.y + 5, 7.0f);
			nvgFill(vg);
			nvgFillColor(vg, nvgRGB(255, 255, 255));
			// "@ = at = attachment"
			nvgBitmapText(vg, tiny_font.duplicate(), NVG_ALIGN_LEFT,
						  rect_pos.x + icon_size, rect_pos.y, "@");
		}
		return true;
	};

	draw_machine_polygon(vg, p, machines, &rnd_idx, vport, cu, lambda);
}



