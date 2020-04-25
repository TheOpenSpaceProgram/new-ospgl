#pragma once
#include <universe/vehicle/Vehicle.h>
#include <renderer/Drawable.h>
#include <universe/vehicle/VehicleLoader.h>

class EditorVehicle : public Drawable
{
public:

	Vehicle* veh;
	bool draw_attachments;

	virtual void deferred_pass(CameraUniforms& cu);
	virtual void forward_pass(CameraUniforms& cu);
	virtual void shadow_pass(ShadowCamera& cu);

	virtual bool needs_deferred_pass() { return true; }
	virtual bool needs_forward_pass() { return true; }
	virtual bool needs_shadow_pass() { return true; }

	EditorVehicle() : Drawable()
	{
		auto vehicle_toml = SerializeUtil::load_file("udata/vehicles/Test Vehicle.toml");
		veh = VehicleLoader::load_vehicle(*vehicle_toml);
	}

};
