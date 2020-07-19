#include "WireInterface.h"

void WireInterface::update(double dt) 
{
	
}

void WireInterface::leave() 
{
	
}

bool WireInterface::can_leave() 
{
	return true;
}

WireInterface::WireInterface(EditorVehicleInterface* edveh_int) 
{
	
}

bool WireInterface::handle_input(const CameraUniforms& cu, glm::dvec3 ray_start, 
		glm::dvec3 ray_end, GUIInput* gui_input)
{
	return false;
}
