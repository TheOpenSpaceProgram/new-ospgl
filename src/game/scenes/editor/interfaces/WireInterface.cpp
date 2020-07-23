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
			glm::vec2 screen_pos = MathUtil::clip_to_screen(clip, vport);
			// Draw the polygon with the machines on its edges
			// and a central "indicator"
			std::vector<Machine*> machines = pair.second;
			float dangle = glm::two_pi<float>() / machines.size();
			float scale = glm::clamp(200.0f / ((float)glm::distance2(pos, cu.cam_pos)), 0.5f, 1.0f);
			float radius = (float)(machines.size() - 1) * 10.0f * scale;
			float icon_size = 22.0f * scale;

			int i = 0;
			for(float angle = 0.0f; angle < glm::two_pi<float>(); angle += dangle)
			{
				Machine* m = machines[i];

				glm::vec2 offset = glm::vec2(sin(angle), cos(angle));
				glm::vec2 final_pos = screen_pos + offset * radius;

				// Indicator
				nvgBeginPath(vg);
				nvgMoveTo(vg, screen_pos.x, screen_pos.y);
				nvgLineTo(vg, screen_pos.x + offset.x * 4.0f, screen_pos.y + offset.y * 4.0f);
				nvgStrokeColor(vg, nvgRGB(255, 0, 255));
				nvgStroke(vg);

				// Machine
				AssetHandle<Image> img = m->get_icon();
				int image = nvglCreateImageFromHandleGL3(vg, img->id, img->get_width(), img->get_height(), 0);
				NVGpaint paint = nvgImagePattern(vg, final_pos.x + icon_size * 0.5f, final_pos.y + icon_size * 0.5f, icon_size, icon_size, 0.0f, image, 1.0f);
				nvgBeginPath(vg);
				nvgCircle(vg, final_pos.x, final_pos.y, icon_size * 0.5f);
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
