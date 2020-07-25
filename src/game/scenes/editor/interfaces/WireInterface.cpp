#include "WireInterface.h"
#include "../EditorVehicleInterface.h"
#include <nanovg/nanovg_gl.h>

void WireInterface::update(double dt) 
{
	edveh->clear_meta();
}

void WireInterface::leave() 
{
	
}

bool WireInterface::can_leave() 
{
	return true;
}

void WireInterface::do_gui(NVGcontext* vg, GUISkin* gui_skin, GUIInput* gui_input, glm::vec4 vport) 
{
	hovered = nullptr;
	std::unordered_map<Machine*, glm::vec2> machine_to_pos;
	Machine* new_wire = nullptr;

	for(auto& pair : visible_machines)
	{
		Part* p = pair.first;
		glm::dvec3 pos = to_dvec3(p->get_piece("p_root")->packed_tform.getOrigin());
		auto[clip, in_front] = MathUtil::world_to_clip(cu.tform, pos);
		glm::vec2 screen_pos = glm::round(MathUtil::clip_to_screen(clip, vport));
		// Draw the polygon with the machines on its edges
		// and a central "indicator"
		std::vector<Machine*> machines = pair.second;
		float dangle = glm::two_pi<float>() / machines.size();
		float scale = glm::clamp(200.0f / ((float)glm::distance2(pos, cu.cam_pos)), 0.5f, 1.0f);
		float icon_size = 22.0f * scale;
		// This formula gives appropiate icon distancing
		float radius = glm::max((float)log(machines.size()) * icon_size * scale * 0.6f, icon_size * 0.7f);
		if(machines.size() == 1)
		{
			radius = 0.0f;
		}

		// Indicator
		if(machines.size() > 1 && in_front)
		{
			nvgBeginPath(vg);
			nvgCircle(vg, screen_pos.x, screen_pos.y, 4.0f);
			nvgFillColor(vg, nvgRGB(0, 0, 0));
			nvgFill(vg);
		}

		int i = 0;
		for(float angle = 0.0f; angle < glm::two_pi<float>(); angle += dangle)
		{
			Machine* m = machines[i];

			glm::vec2 offset = glm::vec2(sin(angle), cos(angle));
			glm::vec2 final_pos = screen_pos + offset * radius;
			glm::vec2 rect_pos = glm::round(final_pos - icon_size * 0.5f);
			glm::vec2 tex_pos = glm::round(final_pos + icon_size * 0.5f);
			machine_to_pos[m] = rect_pos + glm::vec2(icon_size * 0.5f);

			if(in_front)
			{
				// Indicator
				if(machines.size() > 1)
				{
					nvgBeginPath(vg);
					nvgMoveTo(vg, screen_pos.x, screen_pos.y);
					nvgLineTo(vg, round(screen_pos.x + offset.x * 4.0f), round(screen_pos.y + offset.y * 4.0f));
					nvgStrokeColor(vg, nvgRGB(255, 255, 255));
					nvgStrokeWidth(vg, 2.0f);
					nvgStroke(vg);
				}

				// Hover test
				bool contained_in_wired = false;
				for(Machine *sm : selected_wired)
				{
					if(sm == m)
					{
						contained_in_wired = true;
						break;
					}
				}

				bool is_hovered = gui_input->mouse_inside(rect_pos, glm::ivec2(icon_size));

				if(is_hovered || selected == m || contained_in_wired)
				{
					hovered = m;	
					// Hover / Selected indicator
					nvgBeginPath(vg);
					//nvgRect(vg, rect_pos.x - 0.5f, rect_pos.y - 0.5f, icon_size + 1.0f, icon_size + 1.0f);
					nvgCircle(vg, rect_pos.x + icon_size * 0.5f, rect_pos.y + icon_size * 0.5f, icon_size * 0.71f);
					if((gui_input->mouse_down(GUI_LEFT_BUTTON) && is_hovered) || selected == m)
					{
						selected = m;
						selected_wired = m->get_all_wired_machines(false);
						nvgFillColor(vg, nvgRGB(255, 255, 255));
					}
					else
					{
						nvgFillColor(vg, nvgRGBA(0, 0, 0, 128));
					}
					nvgFill(vg);
					if(is_hovered && selected != nullptr && selected != m)
					{
						if(contained_in_wired)
						{
							// Remove (RED)
							nvgStrokeColor(vg, nvgRGB(255, 0, 0));
							if(gui_input->mouse_down(GUI_RIGHT_BUTTON))
							{
								// We have to do two iterations
								auto sel_to_m = edveh->veh->wires.equal_range(selected);
								auto m_to_sel = edveh->veh->wires.equal_range(m);
								for(auto i = sel_to_m.first; i != sel_to_m.second; i++)
								{
									if(i->second == m)
									{
										edveh->veh->wires.erase(i);
										break;
									}
								}
								for(auto i = m_to_sel.first; i != m_to_sel.second; i++)
								{
									if(i->second == selected)
									{
										edveh->veh->wires.erase(i);
										break;
									}
								}

							}
						}
						else
						{
							// Create (GREEN)
							nvgStrokeColor(vg, nvgRGB(0, 255, 0));
							if(gui_input->mouse_down(GUI_RIGHT_BUTTON))
							{
								edveh->veh->wires.insert(std::make_pair(selected, m));
								edveh->veh->wires.insert(std::make_pair(m, selected));	
							}
							else
							{
								new_wire = m;
							}
						}
						nvgStrokeWidth(vg, 2.0f);
					}
					else
					{
						nvgStrokeColor(vg, nvgRGB(255, 255, 255));
					}
					nvgStroke(vg);
					nvgStrokeWidth(vg, 1.0f);

				}

				// Machine
				AssetHandle<Image> img = m->get_icon();
				int image = nvglCreateImageFromHandleGL3(vg, img->id, img->get_width(), img->get_height(), 0);
				NVGpaint paint = nvgImagePattern(vg, tex_pos.x, tex_pos.y, icon_size, icon_size, 0.0f, image, 1.0f);
				nvgBeginPath(vg);
				nvgRect(vg, rect_pos.x, rect_pos.y, icon_size, icon_size);
				nvgFillPaint(vg, paint);
				nvgFill(vg);
			}


			i++;

		}

	}

	if(selected != nullptr)
	{
		glm::vec2 start = machine_to_pos[selected] + 0.5f;
		// Draw wiring lines. As rockets are usually a tall stack of pieces,
		// we do a "bracket" style drawing, we move horizontally, then
		// vertically, and then horizontally again
		// TODO: We could also have "random" curved paths determined via a hash?
		// TODO: Maybe 3D lines would be more convenient and better looking, think of it
		std::vector<Machine*> to_draw_wires = selected_wired;
		if(new_wire)
		{
			to_draw_wires.push_back(new_wire);
		}

		for(Machine* it : to_draw_wires)
		{
			glm::vec2 end = machine_to_pos[it] + 0.5f;
			float h_offset = 40.0f;
			nvgBeginPath(vg);
			nvgMoveTo(vg, start.x, start.y);
			if(end.y > start.y)
			{
				nvgLineTo(vg, start.x + h_offset, start.y);
				nvgLineTo(vg, start.x + h_offset, end.y);
			}
			else
			{
				nvgLineTo(vg, start.x - h_offset, start.y);
				nvgLineTo(vg, start.x - h_offset, end.y);
			}
			nvgLineTo(vg, end.x, end.y);
			if(it == new_wire)
			{
				nvgStrokeColor(vg, nvgRGB(0, 255, 0));
			}
			else
			{
				nvgStrokeColor(vg, nvgRGB(255, 255, 255));
			}
			nvgStroke(vg); 
		}

	}
}

WireInterface::WireInterface(EditorVehicleInterface* edveh_int) 
{
	this->edveh_int = edveh_int;
	this->edveh = edveh_int->edveh;
	this->scene = edveh_int->scene;
	selected = nullptr;
	hovered = nullptr;
	
}

bool WireInterface::handle_input(const CameraUniforms& cu, glm::dvec3 ray_start, 
		glm::dvec3 ray_end, GUIInput* gui_input)
{
	this->cu = cu;	

	visible_machines.clear();

	EditorVehicleInterface::RaycastResult res = 
		edveh_int->raycast(ray_start, ray_end, false);

	bool see_all = gui_input->mouse_pressed(GUI_LEFT_BUTTON);


	// Highlighting always shows the machines of said piece
	if(res.has_hit && hovered == nullptr)
	{
		// TODO: It could be wise to add a check for p->part == nullptr
		see_part(res.p->part);
		edveh->piece_meta[res.p].highlight = glm::vec3(1.0f, 1.0f, 1.0f);
	}

	if(selected != nullptr)
	{
		// Also see all wired parts (including ourselves)
		see_part(selected->in_part);
		for(Machine* it : selected_wired)
		{
			see_part(it->in_part);
		}
	}
	
	for(Part* p : edveh->veh->parts)
	{
		if(p->machines.empty())
		{
			continue;
		}
		// We raycast p_root's center because that's where 
		// the icons are, but ignore all pieces
		bool visible = false;
		if(!see_all)
		{
			std::vector<Piece*> all_pieces;
			for(auto pair : p->pieces)
			{
				all_pieces.push_back(pair.second);
			}

			for(Piece* p : all_pieces)
			{
				EditorVehicleInterface::RaycastResult res = 
					edveh_int->raycast(cu.cam_pos, 
					to_dvec3(p->packed_tform.getOrigin()), false, all_pieces);

				if(!res.has_hit)
				{
					visible = true;
				}
			}
		}

		if(visible || see_all)
		{
			see_part(p);
		}
	}

	return false;
}

void WireInterface::see_part(Part* p) 
{
	// Check if it's already visible
	for(const auto& pair : visible_machines)
	{
		if(pair.first == p)
		{
			return;
		}
	}

	std::vector<Machine*> mch;
	for(const auto& pair : p->machines)
	{
		mch.push_back(pair.second);
	}
	if(!mch.empty())
	{
		visible_machines.push_back(std::make_pair(p, mch));
	}
}
