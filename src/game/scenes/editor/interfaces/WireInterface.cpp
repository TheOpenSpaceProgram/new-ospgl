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

void WireInterface::do_gui(NVGcontext* vg, GUISkin* gui_skin, glm::vec4 vport) 
{
	for(auto& pair : visible_machines)
	{
		Part* p = pair.first;
		glm::dvec3 pos = to_dvec3(p->get_piece("p_root")->packed_tform.getOrigin());
		auto[clip, in_front] = MathUtil::world_to_clip(cu.tform, pos);
		if(in_front)
		{
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
			if(machines.size() > 1)
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

				// Machine
				AssetHandle<Image> img = m->get_icon();
				int image = nvglCreateImageFromHandleGL3(vg, img->id, img->get_width(), img->get_height(), 0);
				NVGpaint paint = nvgImagePattern(vg, tex_pos.x, tex_pos.y, icon_size, icon_size, 0.0f, image, 1.0f);
				nvgBeginPath(vg);
				nvgRect(vg, rect_pos.x, rect_pos.y, icon_size, icon_size);
				nvgFillPaint(vg, paint);
				nvgFill(vg);

				i++;

			}
	}

	}
}

WireInterface::WireInterface(EditorVehicleInterface* edveh_int) 
{
	this->edveh_int = edveh_int;
	this->edveh = edveh_int->edveh;
	this->scene = edveh_int->scene;
	
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
	if(res.has_hit)
	{
		// TODO: It could be wise to add a check for p->part == nullptr
		see_part(res.p->part);
		edveh->piece_meta[res.p].highlight = glm::vec3(1.0f, 1.0f, 1.0f);
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
