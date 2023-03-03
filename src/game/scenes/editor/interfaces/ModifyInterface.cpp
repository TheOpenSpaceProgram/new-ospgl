#include "ModifyInterface.h"
#include "../EditorVehicleInterface.h"

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

	// Highlighting
	for(auto& mp : edveh->piece_meta)
	{
		if(mp.first == hovered)
		{
			mp.second.highlight = glm::vec3(1.0f);
		}
	}

	if(!gui_input->mouse_blocked)
	{
		if(input->mouse_down(GLFW_MOUSE_BUTTON_LEFT))
		{
			int id = hovered ? hovered->id : -1;
			edveh_int->emit_event("on_piece_click", id);
		}
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
}
