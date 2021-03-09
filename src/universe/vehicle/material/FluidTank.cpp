#include "FluidTank.h"
#include <util/Logger.h>
#include <cmath>
#include <imgui/imgui.h>
#include <OSP.h>

// m^3 Pa K^-1 mol^-1
constexpr float R = 8.314462618153f;


static float get_partial_pressure(float temp, float volume, const std::pair<const AssetHandle<PhysicalMaterial>, StoredFluid>& pair)
{
	return pair.first->get_moles(pair.second.gas_mass) * R * temp / volume;
}


float FluidTank::get_pressure() const
{
	float P = 0.0f;
	for(const auto& content : contents)
	{
		P += get_partial_pressure(temperature, get_ullage_volume() + extra_volume, content);
	}
	return P;
}

// This implements an extremely simple simulation of a
// vapour-liquid system in which the contents are non-reactive
// We assume all substances evaporate at the same rate given the same
// vapor pressure, without considering the exposed surface.
void FluidTank::update(float dt, float acceleration)
{
	last_acceleration = glm::max(acceleration, 0.0f);

	// Ullage distribution simulation:
	float d_ullage = -ullage_a_factor * glm::pow(acceleration, 3.0f) + ullage_b_factor;
	ullage_distribution += d_ullage * dt;
	ullage_distribution = glm::clamp(ullage_distribution, 0.0f, 1.0f);

	constexpr float evp_factor = 1.0f;
	// Every fluid will try to make its partial pressure equal to
	// its vapor pressure via evaporation

	for(auto& content : contents)
	{
		float cp = get_partial_pressure(temperature, get_ullage_volume() + extra_volume, content);
		float vp = content.first->get_vapor_pressure(temperature);
		float dp = vp - cp; // + = evaporation, - = condensation
		float moles_needed = (dp * (get_ullage_volume() + extra_volume)) / (R * temperature);
		float dm = content.first->get_mass(moles_needed) * evp_factor * dt;
		if(dm > 0)
		{
			dm = fminf(content.second.liquid_mass, dm);
		}
		else
		{
			// We prevent overfilling the tank via condensation (very rare in real use!)
			float allowed_mass = get_ullage_volume() * content.first->liquid_density;
			// ! Keep in mind dm is negative !
			dm = -fminf(content.second.gas_mass, -dm);
			dm = -fminf(allowed_mass, -dm);
		}

		// We exchange energy for the process, if the energy is not available
		// it won't happen, dU = dH as V is constant
		exchange_heat(-content.first->dH_vaporization * dm);

		content.second.liquid_mass -= dm;
		content.second.gas_mass += dm;
	}
}

float FluidTank::get_ullage_volume() const
{
	return volume - get_fluid_volume();
}

float FluidTank::get_fluid_volume() const
{
	float V = 0.0f;
	for(const auto& c : contents)
	{
		V += c.second.liquid_mass / c.first->liquid_density;
	}

	return V;
}



void FluidTank::draw_imgui_debug()
{
	if(!ImGui::Begin("Fluid Tank", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}

	ImGui::Text("Temperature: %fC", temperature - 273.15f);
	ImGui::Text("Volume: %fL", volume * 1e3f);
	ImGui::Text("Ullage Distribution: %f%%", ullage_distribution * 100.0f);
	ImGui::Text("Ullage Volume: %fL (%f%%)", get_ullage_volume() * 1e3f, get_ullage_volume() / volume * 100.0f);
	ImGui::Text("Fluid Volume: %fL (%f%%)", get_fluid_volume() * 1e3f, get_fluid_volume() / volume * 100.0f);
	ImGui::Text("Pressure: %fkPa", get_pressure() * 1e-3);

	ImGui::BeginTable("content_table", 5, ImGuiTableFlags_Borders);

	ImGui::TableNextColumn();
	ImGui::Text("Material");
	ImGui::TableNextColumn();
	ImGui::Text("Liquid Mass");
	ImGui::TableNextColumn();
	ImGui::Text("Liquid Volume");
	ImGui::TableNextColumn();
	ImGui::Text("Gas Mass");
	ImGui::TableNextColumn();
	ImGui::Text("Gas Pressure");

	for(const auto& content : contents)
	{
		ImGui::TableNextColumn();
		ImGui::Text("%s", content.first->name.c_str());
		ImGui::TableNextColumn();
		ImGui::Text("%f", content.second.liquid_mass);
		ImGui::TableNextColumn();
		float v = content.second.liquid_mass / content.first->liquid_density * 1e3f;
		ImGui::Text("%f", v);
		ImGui::TableNextColumn();
		ImGui::Text("%f", content.second.gas_mass);
		ImGui::TableNextColumn();
		ImGui::Text("%f", get_partial_pressure(temperature, get_ullage_volume() + extra_volume, content) * 1e-3);
	}

	ImGui::EndTable();


	ImGui::End();

}

/*std::vector<StoredFluid> FluidTank::vent(float speed, float dt, bool only_gas)
{
	std::vector<StoredFluid> out;
	float U = ullage_distribution;
	if(only_gas)
	{
		U = 0.0f;
	}

	// At 100% ullage distribution, a quarter of vented material will be liquid
	float liquid_factor = U / 4.0f;

	for(auto& content : contents)
	{
		float P = get_partial_pressure(temperature, get_ullage_volume(), content);
		float vented_mass = glm::min(speed * dt * P, content.second.gas_mass);
	}

	return out;
}

std::vector<StoredFluid> FluidTank::pump(float speed, float dt, bool only_liquid)
{
	std::vector<StoredFluid> out;
	float U = ullage_distribution;
	if(only_liquid)
	{
		U = 0.0f;
	}


	return out;
}*/

void FluidTank::exchange_heat(float d)
{
	float cl_sum = 0.0f;
	float cg_sum = 0.0f;
	for(auto& content : contents)
	{
		float cl = content.first->heat_capacity_liquid;
		float cg = content.first->heat_capacity_gas;
		cl_sum += cl * content.second.liquid_mass;
		cg_sum += cg * content.second.gas_mass;
	}

	// J / K
	float c_sum = cl_sum + cg_sum;

	temperature += d / (c_sum);
}

void FluidTank::go_to_equilibrium(float max_dP)
{
	// TODO: Use a mathematical method to solve the differential, there are plenty of them!
	// I simply fine-tuned the values to reduce the iterations on reasonable test cases
	float ullage_0 = ullage_distribution;
	float P0 = get_pressure();
	float T0 = temperature;
	float softening = 1.0f;
	int it = 0;

	while(true)
	{

		update(12.0f * softening, 0.0f);
		temperature = T0;

		float P = get_pressure();
		float dP = P - P0;
		if(glm::abs(dP) < max_dP)
		{
			break;
		}

		if(it >= 50)
		{
			logger->warn("Too many iterations (50) to solve tank, breaking!");
			break;
		}

		P0 = P;
		it++;
		softening *= 0.96f;
	}


	ullage_distribution = ullage_0;
}

TankContents FluidTank::request_liquid(float speed, float dt, const PartialPressures& outside, bool only_liquid)
{
	TankContents out;

	float gas_factor = ullage_distribution / 4.0f;
	float liquid_factor = 1.0f - gas_factor;

	float rspeed = speed * liquid_factor;
	float gspeed = speed * gas_factor;

	if(!only_liquid)
	{
		out = request_gas(gspeed, dt, outside, true);
	}

	float out_P = 0.0f;
	for(const auto& pressure : outside)
	{
		out_P += pressure.second;
	}

	float Pdiff = out_P - get_pressure();


	return out;
}

TankContents FluidTank::request_gas(float speed, float dt, const PartialPressures &outside, bool only_gas)
{
	return TankContents();
}

