#pragma once
#include <universe/vehicle/Vehicle.h>
#include <renderer/Drawable.h>

class EditorVehicle : public Drawable
{
public:

	Vehicle* veh;

	virtual void deferred_pass(CameraUniforms& cu);
	virtual void forward_pass(CameraUniforms& cu);
	virtual void shadow_pass(ShadowCamera& cu);

	virtual bool needs_deferred_pass() { return true; }
	virtual bool needs_forward_pass() { return true; }
	virtual bool needs_shadow_pass() { return true; }

	EditorVehicle() : Drawable()
	{

	}

};
