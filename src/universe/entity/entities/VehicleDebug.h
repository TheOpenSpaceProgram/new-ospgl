#pragma once
#include <vector>
#include <cstdint>

class VehicleEntity;
class Machine;

class VehicleDebug
{
private:
	VehicleEntity* ent;
	void parts_tab();

	std::vector<Machine*> shown_machines;

public:
	void show_imgui();


	explicit VehicleDebug(VehicleEntity* ent){
		this->ent = ent;
	}
};
