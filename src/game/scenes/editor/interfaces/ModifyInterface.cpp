#include "ModifyInterface.h"
#include "../EditorVehicleInterface.h"
#include "../EditorScene.h"

void ModifyInterface::update(double dt)
{
	edveh->clear_meta();
}

bool
ModifyInterface::do_interface(const CameraUniforms &cu, glm::dvec3 ray_start, glm::dvec3 ray_end, glm::dvec4 viewport,
							  NVGcontext *vg, GUIInput *gui_input, GUISkin *gui_skin)
{
	EditorVehicleInterface::RaycastResult rresult = edveh_int->raycast(ray_start, ray_end, false);

	Piece* hovered = nullptr;
	if(rresult.has_hit)
	{
		hovered = rresult.p;
	}

	if(cur_state == IDLE)
	{
		return do_interface_idle(hovered, gui_input);
	}

	return false;
}

void ModifyInterface::leave()
{
	edveh_int->emit_event("close_context_menus");

}

bool ModifyInterface::can_leave()
{
	return true;
}

ModifyInterface::ModifyInterface(EditorVehicleInterface* eint)
{
	this->edveh_int = eint;
	this->edveh = eint->edveh;
	cur_state = IDLE;
}

void ModifyInterface::change_state(ModifyInterface::State st)
{
	if(st == IDLE)
	{
	}
	else if(st == CREATING_SYMMETRY)
	{

	}
	else if(st == RE_ROOTING)
	{

	}
	else if(st == SELECTING_PIECE)
	{

	}

	edveh_int->scene->gui.modify_tools.change_state(st);

	cur_state = st;
}

bool ModifyInterface::do_interface_idle(Piece* hovered, GUIInput* ipt)
{
	// Highlighting affects only hovered piece
	for(auto& mp : edveh->piece_meta)
	{
		if(mp.first == hovered)
		{
			mp.second.highlight = glm::vec3(1.0f);
		}
	}

	if(!ipt->mouse_blocked)
	{
		if(input->mouse_down(GLFW_MOUSE_BUTTON_LEFT))
		{
			int id = hovered ? hovered->id : -1;
			edveh_int->emit_event("on_piece_click", id);
		}
	}

	return false;

}
