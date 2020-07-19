#pragma once
#include "BaseInterface.h"

class EditorVehicleInterface;

class WireInterface : public BaseInterface
{
private:


public:

	virtual bool handle_input(const CameraUniforms& cu, glm::dvec3 ray_start, 
		glm::dvec3 ray_end, GUIInput* gui_input) override;

	virtual void update(double dt) override;

	virtual void leave() override;

	virtual bool can_leave() override;
	
	WireInterface(EditorVehicleInterface* edveh_int);

};