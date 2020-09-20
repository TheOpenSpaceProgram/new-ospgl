#include "WireInterface.h"
#include "../EditorVehicleInterface.h"
#include <nanovg/nanovg_gl.h>
#include "../EditorScene.h"
#include <util/InputUtil.h>

void WireInterface::update(double dt) 
{
	edveh->clear_meta();

	if(input->key_down(GLFW_KEY_LEFT_ALT))
	{
		show_hidden = !show_hidden;
	}
}

void WireInterface::leave() 
{
	
}

bool WireInterface::can_leave() 
{
	return true;
}

bool WireInterface::do_interface(const CameraUniforms& cu, glm::dvec3 ray_start, glm::dvec3 ray_end,
	glm::dvec4 vport, NVGcontext* vg, GUIInput* gui_input, GUISkin* gui_skin)
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
			for(const auto& pair : p->pieces)
			{
				all_pieces.push_back(pair.second);
			}

			for(Piece* sp : all_pieces)
			{
				EditorVehicleInterface::RaycastResult sres =
					edveh_int->raycast(cu.cam_pos,
									   to_dvec3(sp->packed_tform.getOrigin()), false, all_pieces);

				if(!sres.has_hit)
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

	hovered = nullptr;
	std::unordered_map<Machine*, std::pair<glm::vec2, bool>> machine_to_pos;
	Machine* new_wire = nullptr;

	std::vector<glm::vec2> seen_positions;

	int rnd_idx = 0;
	for(auto& pair : visible_machines)
	{
		std::vector<Machine*> machines_with_marker;

		for(auto it = pair.second.begin(); it != pair.second.end(); )
		{
			Machine* m = *it;
			if(m->editor_location_marker.empty())
			{
				machines_with_marker.push_back(m);
				it = pair.second.erase(it);
			}
			else
			{
				it++;
			}
		}

		Part* p = pair.first;
		glm::dvec3 pos = to_dvec3(p->get_piece("p_root")->packed_tform.getOrigin());
		// Draw the polygon with the machines on its edges
		// and a central "indicator"
		int polygon_machines = machines_with_marker.size();

		float dangle = glm::two_pi<float>() / polygon_machines;

		float scale = glm::clamp(200.0f / ((float)glm::distance2(pos, cu.cam_pos)), 0.5f, 1.0f);
		float real_icon_size = 22.0f;
		float icon_size = real_icon_size * scale;

		// This formula gives appropiate icon distancing
		float radius = glm::max((float)log(polygon_machines) * icon_size * scale * 0.6f, icon_size);
		if(polygon_machines == 1)
		{
			radius = 0.0f;
		}

		auto[clip, in_front] = MathUtil::world_to_clip(cu.tform, pos);
		glm::vec2 screen_pos = glm::round(MathUtil::clip_to_screen(clip, vport));

		// Indicator
		if(polygon_machines > 1 && in_front)
		{
			nvgBeginPath(vg);
			nvgCircle(vg, screen_pos.x, screen_pos.y, 4.0f);
			nvgFillColor(vg, nvgRGB(0, 0, 0));
			nvgFill(vg);
		}

		float angle = 0.0f;
		for(int i = 0; i < polygon_machines; i++)
		{
			Machine* m = pair.second[i];
			
			float adj_angle = angle + glm::half_pi<float>();
			glm::vec2 offset = glm::vec2(sin(adj_angle), cos(adj_angle));
			glm::vec2 final_pos = glm::round(screen_pos + offset * radius);

			bool was_visible = do_machine(vg, gui_input, m, final_pos, pos, vport, in_front, seen_positions, rnd_idx,
								 polygon_machines, new_wire, machine_to_pos);

			// Indicator
			if(was_visible && polygon_machines != 1)
			{
				nvgBeginPath(vg);
				nvgMoveTo(vg, screen_pos.x, screen_pos.y);
				nvgLineTo(vg, round(screen_pos.x + offset.x * 4.0f), round(screen_pos.y + offset.y * 4.0f));
				nvgStrokeColor(vg, nvgRGB(255, 255, 255));
				nvgStrokeWidth(vg, 2.0f);
				nvgStroke(vg);
			}

			angle += dangle;

		}

		// Draw the machines at a marker location
		for(Machine* m : pair.second)
		{

			glm::dvec3 mpos = p->get_piece("p_root")->get_marker_position(m->editor_location_marker);
			mpos = to_dmat4(p->get_piece("p_root")->get_global_transform()) * glm::dvec4(mpos, 1.0);
			auto[clip, in_front] = MathUtil::world_to_clip(cu.tform, mpos);
			glm::vec2 mscreen_pos = glm::round(MathUtil::clip_to_screen(clip, vport));

			do_machine(vg, gui_input, m, mscreen_pos, pos, vport, in_front, seen_positions, rnd_idx,
			  			polygon_machines, new_wire, machine_to_pos);
		}

		rnd_idx++;

	}

	if(selected != nullptr)
	{
		glm::vec2 start = machine_to_pos[selected].first + 0.5f;
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
		

		int off_screen = 0;
		for(Machine* it : to_draw_wires)
		{
			glm::vec2 end = machine_to_pos[it].first + 0.5f;

			nvgBeginPath(vg);
			nvgMoveTo(vg, start.x, start.y);
			if(it == new_wire)
			{
				nvgStrokeColor(vg, nvgRGB(0, 255, 0));
			}
			else
			{
				nvgStrokeColor(vg, nvgRGB(255, 255, 255));
			}

			if(it->in_part == selected->in_part)
			{
				// We wire using a straight line
				nvgLineTo(vg, end.x, end.y);
			}
			else
			{
				float h_offset = 40.0f;
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
			}
			nvgStroke(vg); 
		}
		
	}

	return false;

}

WireInterface::WireInterface(EditorVehicleInterface* edveh_int) 
{
	show_hidden = false;

	this->edveh_int = edveh_int;
	this->edveh = edveh_int->edveh;
	this->scene = edveh_int->scene;
	selected = nullptr;
	hovered = nullptr;

	tiny_font = AssetHandle<BitmapFont>("core:fonts/ProggyTiny.fnt");
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
		visible_machines.emplace_back(p, mch);
	}
}

bool WireInterface::do_machine(NVGcontext* vg, GUIInput* gui_input, Machine* m, glm::vec2 final_pos, glm::dvec3 pos,
							   glm::ivec4 vport, bool in_front, std::vector<glm::vec2>& seen_positions,
							   int rnd_idx, int polygon_machines, Machine*& new_wire,
							   std::unordered_map<Machine*, std::pair<glm::vec2, bool>>& machine_to_pos)
{
	float scale = glm::clamp(200.0f / ((float)glm::distance2(pos, cu.cam_pos)), 0.5f, 1.0f);
	float real_icon_size = 22.0f;
	float icon_size = real_icon_size * scale;

	if(!in_front)
	{
		// Calculate final_pos from relative position to the camera of
		// the machine in 3D space instead of screen space
		glm::dvec3 rel = cu.cam_pos - pos;
		final_pos.y = rel.z * 1e10f + vport.y + vport.w * 0.5f;
		final_pos.x = 2.0f * vport.x + vport.z - final_pos.x;
	}

	bool contained_in_wired = false;
	for(Machine *sm : selected_wired)
	{
		if(sm == m)
		{
			contained_in_wired = true;
			break;
		}
	}

	if(m == selected)
	{
		contained_in_wired = true;
	}

	// Clamp to screen edges
	glm::vec2 old_final_pos = final_pos;
	final_pos = glm::clamp(final_pos, glm::vec2(vport.x, vport.y) + real_icon_size * 0.5f,
						   glm::vec2(vport.x + vport.z, vport.y + vport.w) - real_icon_size * 0.5f);

	bool clamped = final_pos != old_final_pos;

	bool done = false;
	int its = 0;
	while(!done && its < 20 && clamped)
	{

		bool any = false;
		// Make sure there are no overlaps
		// It also gives special priority to top-bottom overlaps,
		// as side overlaps will be rarer due to the camera movement
		// TODO: Write a better method, this gets a bit jittery
		for(glm::vec2 other_pos : seen_positions)
		{
			float dist = glm::distance(other_pos, final_pos);
			if(dist < real_icon_size)
			{
				glm::vec2 rand_offset;
				if(rnd_idx % 2 == 0)
				{
					rand_offset = glm::vec2(1.0f, 0.0f);
				}
				else
				{
					rand_offset = glm::vec2(-1.0f, 0.0f);
				}
				final_pos += rand_offset * real_icon_size * 2.0f;


				any = true;
				break;
			}
		}

		done = !any;
		its++;

	}

	final_pos = glm::clamp(final_pos, glm::vec2(vport.x, vport.y) + real_icon_size * 0.5f,
						   glm::vec2(vport.x + vport.z, vport.y + vport.w) - real_icon_size * 0.5f);

	bool visible = contained_in_wired || (!clamped && in_front) &&
			(!m->editor_hidden || contained_in_wired || show_hidden);


	if(visible)
	{
		glm::vec2 rect_pos = glm::round(final_pos - icon_size * 0.5f);
		glm::vec2 tex_pos = glm::round(final_pos + icon_size * 0.5f);

		glm::vec2 fpos = rect_pos + glm::vec2(icon_size * 0.5f);
		machine_to_pos[m] = std::make_pair(fpos, in_front);

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

		seen_positions.push_back(final_pos);
	}

	return visible;

}

