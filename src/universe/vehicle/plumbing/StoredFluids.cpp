#include "StoredFluids.h"
#include <game/database/GameDatabase.h>

StoredFluids StoredFluids::modify(const StoredFluids &b)
{
	StoredFluids out;
	float final_heat = get_total_heat_capacity() * temperature + b.get_total_heat_capacity() * b.temperature;

	for(const auto& pair : b.contents)
	{
		auto it = contents.find(pair.first);
		if(it == contents.end())
		{
			// We create a new AssetHandle here to store it as we are creating the key
			for(auto& ast : b.mat_ptrs)
			{
				if(ast.data == pair.first)
				{
					mat_ptrs.insert(ast.duplicate());
					name_to_ptr[ast->get_asset_id()] = ast.data;
				}
			}

			contents[pair.first] = pair.second;
			if(pair.second.gas_mass < 0.0f)
			{
				contents.at(pair.first).gas_mass = 0.0f;
			}
			if(pair.second.liquid_mass < 0.0f)
			{
				contents.at(pair.first).liquid_mass = 0.0f;
			}
		}
		else
		{
			out.contents[pair.first] = StoredFluid();
			// We create a new AssetHandle in out to store it as we are creating the key
			for(auto& ast : b.mat_ptrs)
			{
				if(ast.data == pair.first)
				{
					out.mat_ptrs.insert(ast.duplicate());
					out.name_to_ptr[ast->get_asset_id()] = ast.data;
				}
			}

			if(pair.second.gas_mass < 0.0f)
			{
				float original = it->second.gas_mass;
				it->second.gas_mass -= pair.second.gas_mass;
				if(it->second.gas_mass < 0.0f)
				{
					out.contents.at(it->first).gas_mass = original;
				}
				else
				{
					out.contents.at(it->first).gas_mass = pair.second.gas_mass;
				}
			}
			else
			{
				it->second.gas_mass += pair.second.gas_mass;
			}

			if(pair.second.liquid_mass < 0.0f)
			{
				float original = it->second.liquid_mass;
				it->second.liquid_mass -= pair.second.liquid_mass;
				if(it->second.liquid_mass < 0.0f)
				{
					out.contents.at(it->first).liquid_mass = original;
				}
				else
				{
					out.contents.at(it->first).liquid_mass = pair.second.liquid_mass;
				}
			}
			else
			{
				it->second.liquid_mass += pair.second.liquid_mass;
			}
		}
	}

	// Now distribute heat again
	temperature = final_heat / get_total_heat_capacity();

	// Out is at the same temperature as ourselves
	// TODO: This is technically wrong if we both accepted and drained, but it's ok
	// as it's not common and the error is tiny
	out.temperature = temperature;


	return out;
}

StoredFluids StoredFluids::multiply(float value)
{
	StoredFluids out;

	for(const auto& pair : contents)
	{
		out.contents[pair.first] = pair.second;
		// we create a new assethandle in out to store it as we are creating the key
		for(auto& ast : mat_ptrs)
		{
			if(ast.data == pair.first)
			{
				out.mat_ptrs.insert(ast.duplicate());
			}
		}
		out.contents.at(pair.first).gas_mass *= value;
		out.contents.at(pair.first).liquid_mass *= value;
	}

	return out;
}

void StoredFluids::remove_empty_fluids()
{

}

void StoredFluids::add_fluid(const AssetHandle<PhysicalMaterial>& mat, float liquid_mass, float gas_mass, float temp)
{
	StoredFluids tmp;
	if(mat_ptrs.count(mat) == 0)
	{
		// We must introduce a new asset handle
		mat_ptrs.insert(mat.duplicate());
		name_to_ptr[mat->get_asset_id()] = mat.data;
	}

	tmp.contents[mat.data] = StoredFluid(liquid_mass, gas_mass);
	if(temp < 0.0f)
	{
		temp = temperature;
	}
	tmp.temperature = temp;
	modify(tmp);
}

void StoredFluids::drain_to(StoredFluids* target, PhysicalMaterial* mat, float liquid_mass, float gas_mass, bool do_flow)
{
	logger->check(target, "Trying to drain into nullptr");
	logger->check(mat, "Trying to drain nullptr material");

	auto it = target->contents.find(mat);
	StoredFluid* target_fluids;
	if(it == target->contents.end())
	{
		// we create a new assethandle in out to store it as we are creating the key
		for(auto& ast : mat_ptrs)
		{
			if(ast.data == mat)
			{
				target->mat_ptrs.insert(ast.duplicate());
			}
		}

		target->contents[mat] = StoredFluid();
		target_fluids = &target->contents[mat];
	}
	else
	{
		target_fluids = &it->second;
	}

	// Drain from our tank
	auto self_it = contents.find(mat);
	logger->check(self_it != contents.end(), "Could not find material in drain_to, this is not allowed!");

	float tmp_liquid = self_it->second.liquid_mass;
	float tmp_gas = self_it->second.gas_mass;

	self_it->second.gas_mass -= gas_mass;
	self_it->second.liquid_mass -= liquid_mass;
	float tfer_gas = gas_mass;
	float tfer_liq = liquid_mass;
	if(self_it->second.liquid_mass < 0.0f)
	{
		tfer_liq = -self_it->second.liquid_mass;
	}
	if(self_it->second.gas_mass < 0.0f)
	{
		tfer_gas = -self_it->second.gas_mass;
	}

	self_it->second.gas_mass = glm::max(self_it->second.gas_mass, 0.0f);
	self_it->second.liquid_mass = glm::max(self_it->second.liquid_mass, 0.0f);

	float target_heat = target->get_total_heat_capacity() * target->temperature;

	target_fluids->gas_mass += tfer_gas;
	target_fluids->liquid_mass += tfer_liq;

	float tfer_heat = mat->heat_capacity_gas * tfer_gas + mat->heat_capacity_liquid * tfer_liq;
	// This prevents NaN when transferring no heat to a 0 heat capacity target
	if(tfer_heat != 0)
	{
		// Temperature transfer, we remain the same but target will be modified
		target->temperature = (target_heat + tfer_heat * temperature) / target->get_total_heat_capacity();
	}

	// Restore the original fluids
	if(!do_flow)
	{
		self_it->second.gas_mass = tmp_gas;
		self_it->second.liquid_mass = tmp_liquid;
	}

	remove_empty_fluids();

}

float StoredFluids::get_total_liquid_mass() const
{
	float total = 0.0f;
	for(const auto& pair : contents)
	{
		total += pair.second.liquid_mass;
	}
	return total;
}

float StoredFluids::get_total_liquid_volume() const
{
	float total = 0.0f;
	for(const auto& pair : contents)
	{
		total += pair.second.liquid_mass / pair.first->liquid_density;
	}
	return total;
}

float StoredFluids::get_average_liquid_density() const
{
	return get_total_liquid_mass() / get_total_liquid_volume();
}

float StoredFluids::get_total_gas_mass() const
{
	float total = 0.0f;
	for(const auto& pair : contents)
	{
		total += pair.second.gas_mass;
	}
	return total;
}

float StoredFluids::react(float T, float react_V, float liq_fac, float dt)
{
	// TODO: Optimize the way we obtain the possible reactions. Maybe cache?
	// Usually there will be 2 or 3 reactants and maybe 4 reactions per reactant so not too bad
	std::vector<ChemicalReaction> reactions;
	for(auto pair : contents)
	{
		auto lb = osp->game_database->material_to_reactions.lower_bound(pair.first->get_asset_id());
		auto ub = osp->game_database->material_to_reactions.upper_bound(pair.first->get_asset_id());
		for(auto it = lb; it != ub; it++)
		{
			// Check that it's not already added
			bool found = false;
			for (size_t i = 0; i < reactions.size(); i++)
			{
				if (reactions[i] == it->second)
				{
					found = true;
					break;
				}
			}

			if(found)
				break;


			// TODO: Check that either all products or all reactants are present!
			reactions.push_back(it->second);

		}
	}

	for(const ChemicalReaction& reaction : reactions)
	{
		for(auto& reactant : reaction.reactants)
		{
			auto it = name_to_ptr.find(reactant.reactant);
			if (it == name_to_ptr.end())
			{
				// Add the material as it may be a product that's not yet there
				add_fluid(AssetHandle<PhysicalMaterial>(reactant.reactant), 0.0f, 0.0f);
			}
		}
	}
	// Now we move towards the equilibrium in substeps to achieve great precision
	// when multiple reactions are competing. This is not really needed to be extremely
	// precise as we are satisfied with approximate solutions to the equilibria
	constexpr float sub_step = 0.01f;
	float total_h = 0.0f;
	for(float s = 0.0f; s < dt; s += sub_step)
	{
		for(auto& reaction : reactions)
		{
			// Obtain current reaction quotient
			float Q = reaction.get_Q(this, react_V);
			float K = reaction.get_K(T);
			float QmK = Q - K;
			constexpr float MAX_RATE = 50000.0f;
			// Clamp reaction rates
			QmK = glm::max(QmK, -MAX_RATE);
			QmK = glm::min(QmK, MAX_RATE);
			// K must move towards Q. Reacting DECREASES K so we must do the opposite:
			float diff = -QmK * reaction.get_rate(T) * react_V * sub_step;
			float total_react = reaction.react(this, diff, diff * liq_fac);
			total_h += reaction.dH * total_react;
		}
	}

	// Heat released, so the opposite of enthalpy change
	return -total_h;
}

StoredFluid::StoredFluid()
{
	liquid_mass = 0.0f;
	gas_mass = 0.0f;
}

float StoredFluid::get_total_mass()
{
	return liquid_mass + gas_mass;
}

float StoredFluids::get_total_heat_capacity() const
{
	float total = 0.0f;
	for(auto pair : contents)
	{
		total += pair.first->heat_capacity_gas * pair.second.gas_mass + pair.first->heat_capacity_liquid * pair.second.liquid_mass;
	}
	return total;
}

float StoredFluids::get_total_gas_moles() const
{
	float total = 0.0f;
	for(const auto& pair : contents)
	{
		total += pair.first->get_moles(pair.second.gas_mass);
	}
	return total;
}

void StoredFluids::add_heat(float v)
{
	float heat_cap = get_total_heat_capacity();
	if(heat_cap != 0)
	{
		temperature += v / get_total_heat_capacity();
	}
}

void StoredFluids::set_vapor_fraction(const AssetHandle<PhysicalMaterial> &mat, float factor)
{
	auto it = contents.find(mat.get());
	// TODO: Don't include this check and simply ignore the command silently?
	logger->check(it != contents.end(), "Cannot modify liquid fraction of not-present fluid");
	logger->check(factor <= 1.0f && factor >= 0.0f, "Vapor fraction must be within 0 and 1 and its value is {}", factor);

	float total_mass = it->second.gas_mass + it->second.liquid_mass;
	it->second.gas_mass = total_mass * factor;
	it->second.liquid_mass = total_mass * (1.0f - factor);
}


float StoredFluids::get_specific_gas_constant() const
{
	constexpr float R = 8.314462618f;

	float total_mass = get_total_gas_mass();
	float mass_sum = 0.0f;

	for(const auto& pair : contents)
	{
		mass_sum += pair.second.gas_mass * pair.first->molar_mass;
	}

	mass_sum /= total_mass;

	return R / mass_sum;
}

StoredFluid::StoredFluid(float liquid, float gas)
{
	liquid_mass = liquid;
	gas_mass = gas;
}

