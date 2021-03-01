#include "FluidTank.h"
#include <util/Logger.h>
#include <cmath>
#include <imgui/imgui.h>
#include <OSP.h>

constexpr float R = 0.082057366f;

static const PhysicalMaterial& get_material(const std::string& name)
{
	auto it = osp->game_database.materials.find(name);
	logger->check(it != osp->game_database.materials.end(), "Could not find given material '{}'!", name);
	return it->second;
}

static float get_partial_pressure(float temp, float volume, const std::pair<const std::string, StoredFluid>& pair)
{
	return get_material(pair.first).get_moles(pair.second.gas_mass) * R * temp / volume;
}


float FluidTank::get_pressure() const
{
	float P = 0.0f;
	for(const auto& content : contents)
	{
		P += get_partial_pressure(temperature, get_ullage_volume(), content);
	}
	return P;
}

// This implements an extremely simple simulation of a
// vapour-liquid system in which the contents are non-reactive
// We assume all substances evaporate at the same rate given the same
// vapor pressure, without considering the exposed surface.
void FluidTank::update(float dt, float acceleration)
{
	// Ullage distribution simulation:
	float d_ullage = -ullage_a_factor * glm::pow(acceleration, 3.0f) + ullage_b_factor;
	ullage_distribution += d_ullage * dt;
	ullage_distribution = glm::clamp(ullage_distribution, 0.0f, 1.0f);

	constexpr float evp_factor = 0.1f;
	// Every fluid will try to make its partial pressure equal to
	// its vapor pressure via evaporation

	for(auto& content : contents)
	{
		float cp = get_partial_pressure(temperature, get_ullage_volume(), content);
		float vp = get_material(content.first).get_vapor_pressure(temperature);
		float dp = vp - cp; // + = evaporation, - = condensation
		float moles_needed = (dp * get_ullage_volume()) / (R * temperature);
		float dm = get_material(content.first).get_mass(moles_needed) * evp_factor * dt;
		if(dm > 0)
		{
			dm = fminf(content.second.liquid_mass, dm);
		}
		else
		{
			// We prevent overfilling the tank via condensation (very rare!)
			float allowed_mass = get_ullage_volume() * get_material(content.first).liquid_density;
			dm = fminf(content.second.gas_mass, dm);
			dm = fminf(allowed_mass, dm);
		}

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
		V += c.second.liquid_mass / get_material(c.first).liquid_density;
	}

	if(V > volume)
	{
		logger->warn("Volume in tank too high!");
	}

	return V;
}



void FluidTank::draw_imgui_debug()
{
	ImGui::Begin("Fluid Tank", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("Temperature: %fC", temperature - 273.15f);
	ImGui::Text("Volume: %fL", volume);
	ImGui::Text("Ullage Distribution: %f%%", ullage_distribution * 100.0f);
	ImGui::Text("Ullage Volume: %fL (%f%%)", get_ullage_volume(), get_ullage_volume() / volume * 100.0f);
	ImGui::Text("Fluid Volume: %fL (%f%%)", get_fluid_volume(), get_fluid_volume() / volume * 100.0f);
	ImGui::Text("Pressure: %fatm", get_pressure());

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
		ImGui::Text("%s", get_material(content.first).name.c_str());
		ImGui::TableNextColumn();
		ImGui::Text("%f", content.second.liquid_mass);
		ImGui::TableNextColumn();
		float v = content.second.liquid_mass / get_material(content.first).liquid_density;
		ImGui::Text("%f", v);
		ImGui::TableNextColumn();
		ImGui::Text("%f", content.second.gas_mass);
		ImGui::TableNextColumn();
		ImGui::Text("%f", get_partial_pressure(temperature, get_ullage_volume(), content));
	}

	ImGui::EndTable();


	ImGui::End();

}

std::vector<StoredFluid> FluidTank::vent(float speed, float dt, bool only_gas)
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
}
